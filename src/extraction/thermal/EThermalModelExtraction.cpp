#include "EThermalModelExtraction.h"

#include "model/thermal/utils/EStackupPrismThermalModelBuilder.h"
#include "model/geometry/utils/ELayerCutModelQuery.h"
#include "model/thermal/EStackupPrismThermalModel.h"
#include "model/thermal/io/EPrismThermalModelIO.h"
#include "model/thermal/EPrismThermalModel.h"
#include "model/thermal/EGridThermalModel.h"
#include "model/geometry/ELayerCutModel.h"
#include "utility/EMetalFractionMapping.h"
#include "utility/ELayoutRetriever.h"
#include "generic/tools/FileSystem.hpp"
#include "generic/geometry/Mesh2D.hpp"
#include "interface/Interface.h"

namespace ecad::extraction {

using namespace ecad::model;
using namespace ecad::utils;
using namespace generic::geometry;

ECAD_INLINE UPtr<IModel> EThermalModelExtraction::GenerateThermalModel(Ptr<ILayoutView> layout, const EThermalModelExtractionSettings & settings)
{
    if (auto gridSettings = dynamic_cast<CPtr<EGridThermalModelExtractionSettings>>(&settings); gridSettings)
        return GenerateGridThermalModel(layout, *gridSettings);
    else if (auto prismSettings = dynamic_cast<CPtr<EPrismThermalModelExtractionSettings>>(&settings); prismSettings) {
        if (prismSettings->meshSettings.genMeshByLayer)
            return GenerateStackupPrismThermalModel(layout, *prismSettings);
        else return GeneratePrismThermalModel(layout, *prismSettings);
    }
    return nullptr;
}

ECAD_INLINE UPtr<IModel> EThermalModelExtraction::GenerateGridThermalModel(Ptr<ILayoutView> layout, const EGridThermalModelExtractionSettings & settings)
{
    ECAD_EFFICIENCY_TRACK("generate grid thermal model")

    ELayoutMetalFractionMapper mapper(settings.metalFractionMappingSettings);
    if (not mapper.GenerateMetalFractionMapping(layout)) return nullptr;

    auto mf = mapper.GetLayoutMetalFraction();
    auto mfInfo = mapper.GetMetalFractionInfo();
    if (nullptr == mf || nullptr == mfInfo) return nullptr;

    if (not settings.workDir.empty() && settings.dumpDensityFile) {
        auto densityFile = settings.workDir + ECAD_SEPS + "density.txt";
        WriteThermalProfile(*mfInfo, *mf, densityFile);
    }

    const auto & coordUnits = layout->GetCoordUnits();

    auto [nx, ny] = mfInfo->grid;
    auto model = new EGridThermalModel(settings, ESize2D(nx, ny));

    auto rx = coordUnits.toUnit(mfInfo->stride[0], ECoordUnits::Unit::Meter);
    auto ry = coordUnits.toUnit(mfInfo->stride[1], ECoordUnits::Unit::Meter);
    model->SetResolution(rx, ry);

    std::vector<Ptr<IStackupLayer> > layers;
    layout->GetStackupLayers(layers);
    ECAD_ASSERT(layers.size() == mf->size())

    std::unordered_map<ELayerId, size_t> lyrMap;
    for(size_t i = 0; i < layers.size(); ++i) {
        auto stackupLayer = layers.at(i);
        auto name = stackupLayer->GetName();
        auto thickness = coordUnits.toCoordF(stackupLayer->GetThickness());
        thickness = coordUnits.toUnit(thickness, ECoordUnits::Unit::Meter);
        auto layerMetalFraction = mf->at(i);
        EGridThermalLayer layer(name, layerMetalFraction);
        layer.SetThickness(thickness);
        auto index = model->AppendLayer(std::move(layer));
        ECAD_ASSERT(index != invalidIndex)
        lyrMap.emplace(layers.at(i)->GetLayerId(), index);
    }
    
    //bondwire
    auto primIter = layout->GetPrimitiveIter();
    while (auto * prim = primIter->Next()) {
        if (auto * bw = prim->GetBondwireFromPrimitive(); bw) {
            const auto & start = bw->GetStartPt();
            const auto & end  = bw->GetEndPt();
            auto l = coordUnits.toUnit(Distance(start, end), ECoordUnits::Unit::Meter);
            auto r = coordUnits.toCoordF(bw->GetRadius());
            r = coordUnits.toUnit(r, ECoordUnits::Unit::Meter);
            auto alpha = generic::math::pi * r * r / l;
            auto index1 = mfInfo->GetIndex(start);
            auto index2 = mfInfo->GetIndex(end);
            model->AppendJumpConnection(ESize3D(index1, 0), ESize3D(index2, 0), alpha);
        }
    }

    auto compIter = layout->GetComponentIter();
    std::unordered_map<size_t, std::vector<EGridData> > gridMap;
    std::vector<EFloat> temp{25, 50, 75, 100, 125};
    while (auto * component = compIter->Next()) {
        if (component->hasLossPower()) {
            auto layer = component->GetPlacementLayer();
            if (lyrMap.find(layer) == lyrMap.cend()) {
                ECAD_ASSERT(false)
                continue;
            }
            auto lyrId = lyrMap.at(layer);
            auto bbox = component->GetBoundingBox();
            auto ll = mfInfo->GetIndex(bbox[0]);
            auto ur = mfInfo->GetIndex(bbox[1]);
            if (not ll.isValid() || not ur.isValid()) {
                ECAD_ASSERT(false)
                continue;
            }
            
            auto iter = gridMap.find(lyrId);
            if (iter == gridMap.cend())
                iter = gridMap.emplace(lyrId, std::vector<EGridData>(temp.size(), EGridData(nx, ny, 0))).first;
            auto & gridData = iter->second;
            auto totalTiles = (ur[1] - ll[1] + 1) * (ur[0] - ll[0] + 1);
            for (size_t t = 0; t < temp.size(); ++t) {
                auto power = component->GetLossPower(temp.at(t)) / totalTiles;
                for (size_t i = ll[0]; i <= ur[0]; ++i)
                    for (size_t j = ll[1]; j <= ur[1]; ++j)
                        gridData[t](i, j) += power;
            }
        }
    }

    for (auto & [lyrId, gridData] : gridMap) {
        auto powerModel = new EGridPowerModel(ESize2D(nx, ny));
        for (size_t i = 0; i < gridData.size(); ++i)
            powerModel->GetTable().AddSample(ETemperature::Celsius2Kelvins(temp.at(i)), std::move(gridData[i]));
        model->AddPowerModel(lyrId, std::shared_ptr<EThermalPowerModel>(powerModel));
    }
    
    //bc
    if (settings.topUniformBC.isValid())
        model->SetUniformBC(EOrientation::Top, settings.topUniformBC);
    if (settings.botUniformBC.isValid())
        model->SetUniformBC(EOrientation::Bot, settings.botUniformBC);
    for (const auto & block : settings.topBlockBC) {
        if (not block.second.isValid()) continue;
        const auto & bbox = coordUnits.toCoord(block.first);
        auto ll = mfInfo->GetIndex(bbox[0]);
        auto ur = mfInfo->GetIndex(bbox[1]);
        model->AddBlockBC(EOrientation::Top, std::move(ll), std::move(ur), block.second);
    }
    for (const auto & block : settings.botBlockBC) {
        if (not block.second.isValid()) continue;
        const auto & bbox = coordUnits.toCoord(block.first);
        auto ll = mfInfo->GetIndex(bbox[0]);
        auto ur = mfInfo->GetIndex(bbox[1]);
        model->AddBlockBC(EOrientation::Bot, std::move(ll), std::move(ur), block.second);
    }        
    return std::unique_ptr<IModel>(model);
}

ECAD_INLINE bool GenerateMesh(const std::vector<EPolygonData> & polygons, const std::vector<EPoint2D> & steinerPoints, const ECoordUnits & coordUnits, const EPrismMeshSettings & meshSettings, 
                                tri::Triangulation<EPoint2D> & triangulation, std::string meshFile)
{
    ECAD_TRACE("refine mesh, minAlpha: %1%, minLen: %2%, maxLen: %3%, tolerance: %4%, ite: %5%", 
                meshSettings.minAlpha, meshSettings.minLen, meshSettings.maxLen, meshSettings.tolerance, meshSettings.iteration);
    auto minAlpha = math::Rad(meshSettings.minAlpha);
    auto minLen = coordUnits.toCoord(meshSettings.minLen);
    auto maxLen = coordUnits.toCoord(meshSettings.maxLen);
    auto tolerance = coordUnits.toCoord(meshSettings.tolerance);
    mesh2d::IndexEdgeList edges;
    mesh2d::Point2DContainer points;
    mesh2d::Segment2DContainer segments;
    mesh2d::ExtractIntersections(polygons, segments);
    mesh2d::ExtractTopology(segments, points, edges);
    points.insert(points.end(), steinerPoints.begin(), steinerPoints.end());
    mesh2d::MergeClosePointsAndRemapEdge(points, edges, tolerance);
    mesh2d::TriangulatePointsAndEdges(points, edges, triangulation);
    mesh2d::TriangulationRefinement(triangulation, minAlpha, minLen, maxLen, meshSettings.iteration);
    if (not meshFile.empty()) GeometryIO::WritePNG(meshFile, triangulation, 4096);
    return true;
}

ECAD_INLINE UPtr<IModel> EThermalModelExtraction::GeneratePrismThermalModel(Ptr<ILayoutView> layout, const EPrismThermalModelExtractionSettings & settings)
{
    ECAD_EFFICIENCY_TRACK("generate prism thermal model")
    auto model = new EPrismThermalModel(layout, settings);
    auto lcModel = layout->ExtractLayerCutModel(settings.layerCutSettings);
    auto compact = dynamic_cast<CPtr<ELayerCutModel>>(lcModel);
    ECAD_ASSERT(compact)

    if (not settings.workDir.empty() && settings.layerCutSettings.dumpSketchImg) {
        std::string filename = settings.workDir + ECAD_SEPS + "LayerCut.png";
        if (fs::FileExists(filename)) {
            size_t index = 0;
            do {  
                filename = settings.workDir + ECAD_SEPS + "LayerCut" + std::to_string(++index) + ".png";
            } while (fs::FileExists(filename));
        }
        compact->WriteImgView(filename, 3000);
    }

    auto triangulation = std::make_shared<EPrismThermalModel::PrismTemplate>();
    const auto & coordUnits = layout->GetDatabase()->GetCoordUnits();
    std::string meshFile = (settings.meshSettings.dumpMeshFile && not settings.workDir.empty()) ?
        settings.workDir + ECAD_SEPS + "mesh.png" : std::string{};
    GenerateMesh(compact->GetAllPolygonData(), compact->GetSteinerPoints(), coordUnits, settings.meshSettings, *triangulation, meshFile);
    ECAD_TRACE("total mesh elements: %1%", triangulation->triangles.size());

    ecad::utils::ELayoutRetriever retriever(layout);
    for (size_t layer = 0; layer < compact->TotalLayers(); ++layer) {
        model->SetLayerPrismTemplate(layer, triangulation);
        PrismLayer prismLayer(layer);
        [[maybe_unused]] auto check = compact->GetLayerHeightThickness(layer, prismLayer.elevation, prismLayer.thickness); { ECAD_ASSERT(check) }
        model->AppendLayer(std::move(prismLayer));
    }

    std::unordered_set<EMaterialId> fluidMaterials;
    auto matIter = layout->GetDatabase()->GetMaterialDefIter();
    while (auto * material = matIter->Next()) {
        if (EMaterialType::Fluid == material->GetMaterialType())
            fluidMaterials.emplace(material->GetMaterialId());
    }

    model::utils::ELayerCutModelQuery query(compact);
    const auto & powerBlocks = compact->GetAllPowerBlocks();
    std::unordered_map<size_t, std::unordered_map<size_t, size_t> > templateIdMap;//[layer, [tempId, eleId]]

    auto buildOnePrismLayer = [&](size_t index) {
        auto & prismLayer = model->layers.at(index);
        auto & idMap = templateIdMap.emplace(prismLayer.id, std::unordered_map<size_t, size_t>{}).first->second;    
        auto triangulation = model->GetLayerPrismTemplate(index);
        for (size_t it = 0; it < triangulation->triangles.size(); ++it) {
            ECAD_ASSERT(compact->hasPolygon(prismLayer.id))
            auto ctPoint = tri::TriangulationUtility<EPoint2D>::GetCenter(*triangulation, it).Cast<ECoord>();
            auto pid = query.SearchPolygon(prismLayer.id, ctPoint);
            if (pid == invalidIndex) continue;;
            if (fluidMaterials.count(compact->GetMaterialId(pid))) continue;
            if (EMaterialId::noMaterial == compact->GetMaterialId(pid)) continue;

            auto & ele = prismLayer.AddElement(it);
            idMap.emplace(it, ele.id);
            ele.matId = compact->GetMaterialId(pid);
            ele.netId = compact->GetNetId(pid);
            auto iter = powerBlocks.find(pid);
            if (iter != powerBlocks.cend() &&
                prismLayer.id == compact->GetLayerIndexByHeight(iter->second.range.high)) {
                auto area = tri::TriangulationUtility<EPoint2D>::GetTriangleArea(*triangulation, it);
                ele.powerRatio = area / compact->GetAllPolygonData().at(pid).Area();
                ele.powerScenario = iter->second.scen;
                ele.powerLut = iter->second.power;
            }
        }
        ECAD_TRACE("layer %1%'s total elements: %2%", index, prismLayer.elements.size());
    };

    for (size_t index = 0; index < model->TotalLayers(); ++index)
        buildOnePrismLayer(index);
    
    //build connection
    for (size_t index = 0; index < model->TotalLayers(); ++index) {
        auto & layer = model->layers.at(index);
        auto & elements = layer.elements;
        const auto & currIdMap = templateIdMap.at(layer.id);
        for (auto & ele : elements) {
            //layer neighbors
            const auto & triangle = triangulation->triangles.at(ele.templateId);
            for (size_t nid = 0; nid < triangle.neighbors.size(); ++nid) {
                if (tri::noNeighbor == triangle.neighbors.at(nid)) continue;
                auto iter = currIdMap.find(triangle.neighbors.at(nid));
                if (iter != currIdMap.cend()) ele.neighbors[nid] = iter->second;
            }
        }
        if (not model->isBotLayer(index)) {
            auto & lowerLayer = model->layers.at(index + 1);
            auto & lowerEles = lowerLayer.elements;
            const auto & lowerIdMap = templateIdMap.at(lowerLayer.id);
            for (auto & ele : elements) {
                auto iter = lowerIdMap.find(ele.templateId);
                if (iter != lowerIdMap.cend()) {
                    auto & lowerEle = lowerEles.at(iter->second);
                    lowerEle.neighbors[PrismElement::TOP_NEIGHBOR_INDEX] = ele.id;
                    ele.neighbors[PrismElement::BOT_NEIGHBOR_INDEX] = lowerEle.id;
                }
            }
        }
    }
    auto scaleH2Unit = coordUnits.Scale2Unit();
    auto scale2Meter = coordUnits.toUnit(coordUnits.toCoord(1), ECoordUnits::Unit::Meter);
    model->BuildPrismModel(scaleH2Unit, scale2Meter);
    ECAD_TRACE("total prism elements: %1%", model->TotalPrismElements());

    model->AddBondWiresFromLayerCutModel(compact);
    ECAD_TRACE("total line elements: %1%", model->TotalLineElements());

    //bc
    if (settings.topUniformBC.isValid())
        model->SetUniformBC(EOrientation::Top, settings.topUniformBC);
    if (settings.botUniformBC.isValid())
        model->SetUniformBC(EOrientation::Bot, settings.botUniformBC);
        
    for (const auto & block : settings.topBlockBC) {
        if (not block.second.isValid()) continue;
        model->AddBlockBC(EOrientation::Top, coordUnits.toCoord(block.first), block.second);
    }
    for (const auto & block : settings.botBlockBC) {
        if (not block.second.isValid()) continue;
        model->AddBlockBC(EOrientation::Bot, coordUnits.toCoord(block.first), block.second);
    }
        
    if (not settings.workDir.empty() && settings.meshSettings.dumpMeshFile) { 
        auto meshFile = settings.workDir + ECAD_SEPS + "mesh.vtk";
        io::GenerateVTKFile(meshFile, *model);
    }

    return std::unique_ptr<IModel>(model);
}

ECAD_INLINE UPtr<IModel> EThermalModelExtraction::GenerateStackupPrismThermalModel(Ptr<ILayoutView> layout, const EPrismThermalModelExtractionSettings & settings)
{
    ECAD_EFFICIENCY_TRACK("generate stackup prism thermal model")
    auto model = new EStackupPrismThermalModel(layout, settings);
    auto lcModel = layout->ExtractLayerCutModel(settings.layerCutSettings);
    auto compact = dynamic_cast<CPtr<ELayerCutModel>>(lcModel);
    ECAD_ASSERT(compact)

    if (not settings.workDir.empty() && settings.layerCutSettings.dumpSketchImg) {
        std::string filename = settings.workDir + ECAD_SEPS + "LayerCut.png";
        if (fs::FileExists(filename)) {
            size_t index = 0;
            do {  
                filename = settings.workDir + ECAD_SEPS + "LayerCut" + std::to_string(++index) + ".png";
            } while (fs::FileExists(filename));
        }
        compact->WriteImgView(filename, 3000);
    }

    const auto & coordUnits = layout->GetDatabase()->GetCoordUnits();

    //mesh, todo steiner points
    using PrismTemplate = typename EPrismThermalModel::PrismTemplate;
    std::vector<std::vector<EPolygonData>> layerPolygons{compact->GetLayerPolygons(0)};
    std::vector<SPtr<PrismTemplate>> prismTemplates{std::make_shared<PrismTemplate>()};
    std::unordered_map<size_t, size_t> layer2Template{{0, 0}};
    for (size_t i = 1; i < compact->TotalLayers(); ++i) {
        auto indices = compact->GetLayerPolygonIndices(i);
        if (indices != compact->GetLayerPolygonIndices(i - 1)) {
            auto polygons = compact->GetLayerPolygons(i);
            if (settings.meshSettings.imprintUpperLayer) {
                const auto & upperLyr = layerPolygons.back();
                polygons.insert(polygons.end(), upperLyr.begin(), upperLyr.end());
            }
            layerPolygons.emplace_back(std::move(polygons));
            prismTemplates.emplace_back(new PrismTemplate);
        }
        layer2Template.emplace(i, prismTemplates.size() - 1);
    }
    
    std::vector<EPoint2D> steinerPoints;//todo, bwu
    ECAD_TRACE("generate mesh for %1% layers", prismTemplates.size());
    if (settings.threads > 1) {
        generic::thread::ThreadPool pool(settings.threads);
        for (size_t i = 0; i < prismTemplates.size(); ++i) {
            std::string meshFile = (settings.meshSettings.dumpMeshFile && not settings.workDir.empty()) ?
                settings.workDir + ECAD_SEPS + "mesh" + std::to_string(i + 1) + ".png" : std::string{};
            pool.Submit(std::bind(GenerateMesh, std::ref(layerPolygons.at(i)), std::ref(steinerPoints), std::ref(coordUnits), 
                        std::ref(settings.meshSettings), std::ref(*prismTemplates.at(i)), meshFile));
        }

    }
    else {
        for (size_t i = 0; i < prismTemplates.size(); ++i) {
            const auto & polygons = layerPolygons.at(i);
            auto & triangulation = *prismTemplates.at(i);
           std::string meshFile = (settings.meshSettings.dumpMeshFile && not settings.workDir.empty()) ?
                settings.workDir + ECAD_SEPS + "mesh" + std::to_string(i + 1) + ".png" : std::string{};
            GenerateMesh(polygons, {}, coordUnits, settings.meshSettings, triangulation, meshFile);
        }
    }

    for (size_t i = 0; i < compact->TotalLayers(); ++i) {
        model->SetLayerPrismTemplate(i, prismTemplates.at(layer2Template.at(i)));
        PrismLayer prismLayer(i);
        [[maybe_unused]] auto check = compact->GetLayerHeightThickness(i, prismLayer.elevation, prismLayer.thickness); { ECAD_ASSERT(check) }
        model->AppendLayer(std::move(prismLayer));
    }

    std::unordered_set<EMaterialId> fluidMaterials;
    auto matIter = layout->GetDatabase()->GetMaterialDefIter();
    while (auto * material = matIter->Next()) {
        if (EMaterialType::Fluid == material->GetMaterialType())
            fluidMaterials.emplace(material->GetMaterialId());
    }

    model::utils::ELayerCutModelQuery query(compact);
    const auto & powerBlocks = compact->GetAllPowerBlocks();
    std::unordered_map<size_t, std::unordered_map<size_t, size_t> > templateIdMap;//[layer, [tempId, eleId]]

    auto buildOnePrismLayer = [&](size_t index) {
        auto & prismLayer = model->layers.at(index);
        auto & idMap = templateIdMap.emplace(prismLayer.id, std::unordered_map<size_t, size_t>{}).first->second;    
        auto triangulation = model->GetLayerPrismTemplate(index);
        for (size_t it = 0; it < triangulation->triangles.size(); ++it) {
            ECAD_ASSERT(compact->hasPolygon(prismLayer.id))
            auto ctPoint = tri::TriangulationUtility<EPoint2D>::GetCenter(*triangulation, it).Cast<ECoord>();
            auto pid = query.SearchPolygon(prismLayer.id, ctPoint);
            if (pid == invalidIndex) continue;;
            if (fluidMaterials.count(compact->GetMaterialId(pid))) continue;
            if (EMaterialId::noMaterial == compact->GetMaterialId(pid)) continue;

            auto & ele = prismLayer.AddElement(it);
            idMap.emplace(it, ele.id);
            ele.matId = compact->GetMaterialId(pid);
            ele.netId = compact->GetNetId(pid);
            auto iter = powerBlocks.find(pid);
            if (iter != powerBlocks.cend() &&
                prismLayer.id == compact->GetLayerIndexByHeight(iter->second.range.high)) {
                auto area = tri::TriangulationUtility<EPoint2D>::GetTriangleArea(*triangulation, it);
                ele.powerRatio = area / compact->GetAllPolygonData().at(pid).Area();
                ele.powerScenario = iter->second.scen;
                ele.powerLut = iter->second.power;
            }
        }
        ECAD_TRACE("layer %1%'s total elements: %2%", index, prismLayer.elements.size());
    };

    for (size_t index = 0; index < model->TotalLayers(); ++index)
        buildOnePrismLayer(index);

    //build connection
    for (size_t index = 0; index < model->TotalLayers(); ++index) {
        auto & layer = model->layers.at(index);
        auto & elements = layer.elements;
        const auto & currIdMap = templateIdMap.at(layer.id);
        auto triangulation = model->GetLayerPrismTemplate(index);
        for (auto & ele : elements) {
            //layer neighbors
            const auto & triangle = triangulation->triangles.at(ele.templateId);
            for (size_t nid = 0; nid < triangle.neighbors.size(); ++nid) {
                if (tri::noNeighbor == triangle.neighbors.at(nid)) continue;
                auto iter = currIdMap.find(triangle.neighbors.at(nid));
                if (iter != currIdMap.cend()) ele.neighbors[nid] = iter->second;
            }
            ele.neighbors[PrismElement::TOP_NEIGHBOR_INDEX] = ele.id;//todo
            ele.neighbors[PrismElement::BOT_NEIGHBOR_INDEX] = ele.id;//todo
        }
    }

    auto scaleH2Unit = coordUnits.Scale2Unit();
    auto scale2Meter = coordUnits.toUnit(coordUnits.toCoord(1), ECoordUnits::Unit::Meter);

    model::utils::EStackupPrismThermalModelBuilder builder(model);
    builder.BuildPrismModel(scaleH2Unit, scale2Meter, settings.threads);
    ECAD_TRACE("total prism elements: %1%", model->TotalPrismElements());

    builder.AddBondWiresFromLayerCutModel(compact);
    ECAD_TRACE("total line elements: %1%", model->TotalLineElements());

    //bc
    if (settings.topUniformBC.isValid())
        model->SetUniformBC(EOrientation::Top, settings.topUniformBC);
    if (settings.botUniformBC.isValid())
        model->SetUniformBC(EOrientation::Bot, settings.botUniformBC);
        
    for (const auto & block : settings.topBlockBC) {
        if (not block.second.isValid()) continue;
        model->AddBlockBC(EOrientation::Top, coordUnits.toCoord(block.first), block.second);
    }
    for (const auto & block : settings.botBlockBC) {
        if (not block.second.isValid()) continue;
        model->AddBlockBC(EOrientation::Bot, coordUnits.toCoord(block.first), block.second);
    }
        
    if (not settings.workDir.empty() && settings.meshSettings.dumpMeshFile) { 
        auto meshFile = settings.workDir + ECAD_SEPS + "mesh.vtk";
        io::GenerateVTKFile(meshFile, *model);
    }
    return std::unique_ptr<IModel>(model);
}

}//namespace ecad::extraction