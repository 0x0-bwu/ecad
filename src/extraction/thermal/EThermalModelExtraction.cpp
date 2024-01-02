#include "EThermalModelExtraction.h"

#include "models/thermal/io/EPrismaThermalModelIO.h"
#include "models/thermal/EPrismaThermalModel.h"
#include "models/thermal/EGridThermalModel.h"
#include "utilities/EMetalFractionMapping.h"
#include "utilities/ELayoutRetriever.h"
#include "generic/tools/FileSystem.hpp"

#include "Mesher2D.h"
#include "Interface.h"

namespace ecad::extraction {

using namespace ecad::model;
using namespace ecad::utils;

ECAD_INLINE UPtr<IModel> EThermalModelExtraction::GenerateThermalModel(Ptr<ILayoutView> layout, const EThermalModelExtractionSettings & settings)
{
    if (auto gridSettings = dynamic_cast<CPtr<EGridThermalModelExtractionSettings>>(&settings); gridSettings)
        return GenerateGridThermalModel(layout, *gridSettings);
    else if (auto prismaSettings = dynamic_cast<CPtr<EPrismaThermalModelExtractionSettings>>(&settings); prismaSettings)
        return GeneratePrismaThermalModel(layout, *prismaSettings);
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
    auto model = new EGridThermalModel(ESize2D(nx, ny));

    auto rx = coordUnits.toUnit(mfInfo->stride[0], ECoordUnits::Unit::Meter);
    auto ry = coordUnits.toUnit(mfInfo->stride[1], ECoordUnits::Unit::Meter);
    model->SetResolution(rx, ry);

    std::vector<Ptr<IStackupLayer> > layers;
    layout->GetStackupLayers(layers);
    ECAD_ASSERT(layers.size() == mf->size());

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
            auto l = coordUnits.toUnit(generic::geometry::Distance(start, end), ECoordUnits::Unit::Meter);
            auto r = coordUnits.toCoordF(bw->GetRadius());
            r = coordUnits.toUnit(r, ECoordUnits::Unit::Meter);
            auto alpha = generic::math::pi * r * r / l;
            auto index1 = mfInfo->GetIndex(start);
            auto index2 = mfInfo->GetIndex(end);
            model->AppendJumpConnection(ESize3D(index1, 0), ESize3D(index2, 0), alpha);
        }
    }

    EFloat iniT = 25;
    constexpr bool useGridPower = true;
    auto compIter = layout->GetComponentIter();
    std::unordered_map<size_t, EGridData> gridMap;
    while (auto * component = compIter->Next()) {
        if (auto power = component->GetLossPower(); power > 0) {
            auto layer = component->GetPlacementLayer();
            if (lyrMap.find(layer) == lyrMap.cend()) {
                GENERIC_ASSERT(false)
                continue;
            }
            auto lyrId = lyrMap.at(layer);
            auto bbox = component->GetBoundingBox();
            auto ll = mfInfo->GetIndex(bbox[0]);
            auto ur = mfInfo->GetIndex(bbox[1]);
            if (not ll.isValid() || not ur.isValid()) {
                GENERIC_ASSERT(false)
                continue;
            }
            if constexpr (useGridPower) {
                auto iter = gridMap.find(lyrId);
                if (iter == gridMap.cend())
                    iter = gridMap.emplace(lyrId, EGridData(nx, ny, 0)).first;
                auto & gridData = iter->second;
                auto totalTiles = (ur[1] - ll[1] + 1) * (ur[0] - ll[0] + 1);
                power /= totalTiles;
                for (size_t i = ll[0]; i <= ur[0]; ++i)
                    for (size_t j = ll[1]; j <= ur[1]; ++j)
                        gridData(i, j) += power;
            }
            else model->AddPowerModel(lyrMap.at(layer), std::shared_ptr<EThermalPowerModel>(new EBlockPowerModel(ll, ur, power)));
        }
    }

    for (auto & [lyrId, gridData] : gridMap) {
        auto powerModel = new EGridPowerModel(ESize2D(nx, ny));
        powerModel->GetTable().AddSample(iniT, std::move(gridData));
        model->AddPowerModel(lyrId, std::shared_ptr<EThermalPowerModel>(powerModel));
    }

    //htc
    model->SetTopBotBCType(EGridThermalModel::BCType::HTC, EGridThermalModel::BCType::HTC);
    model->SetUniformTopBotBCValue(0, 2750);

    return std::unique_ptr<IModel>(model);
}

ECAD_INLINE UPtr<IModel> EThermalModelExtraction::GeneratePrismaThermalModel(Ptr<ILayoutView> layout, const EPrismaThermalModelExtractionSettings & settings)
{
    ECAD_EFFICIENCY_TRACK("generate prisma thermal model")
    auto model = new EPrismaThermalModel(layout);
    auto compact = makeCompactLayout(layout);
    if (not settings.workDir.empty()) {
        auto compactModelFile = settings.workDir + ECAD_SEPS + "compact.png";
        compact->WriteImgView(compactModelFile, 1024);
    }

    const auto & coordUnits = layout->GetDatabase()->GetCoordUnits();

    //todo wrapper to mesh
    using namespace emesh;
    using namespace generic;
    using namespace generic::geometry;
    const auto & meshSettings = settings.meshSettings;
    std::list<tri::IndexEdge> edges;
    std::vector<Point2D<ECoord> > points;
    std::vector<Segment2D<ECoord> > segments;
    auto & triangulation = model->prismaTemplate;
    MeshFlow2D::ExtractIntersections(compact->polygons, segments);
    MeshFlow2D::ExtractTopology(segments, points, edges);
    points.insert(points.end(), compact->steinerPoints.begin(), compact->steinerPoints.end());
    MeshFlow2D::TriangulatePointsAndEdges(points, edges, triangulation);
    ECAD_TRACE("refine mesh, minAlpha: %1%, minLen: %2%, maxLen: %3%, ite: %4%", 
                meshSettings.minAlpha, meshSettings.minLen, meshSettings.maxLen, meshSettings.iteration)
    auto minAlpha = math::Rad(meshSettings.minAlpha);
    auto minLen = coordUnits.toCoord(meshSettings.minLen);
    auto maxLen = coordUnits.toCoord(meshSettings.maxLen);
    MeshFlow2D::TriangulationRefinement(triangulation, minAlpha, minLen, maxLen, meshSettings.iteration);
    if (not settings.workDir.empty()) {
        auto meshTemplateFile = settings.workDir + ECAD_SEPS + "mesh.png";
        GeometryIO::WritePNG(meshTemplateFile, triangulation, 4096);
    }
    ECAD_TRACE("total elements: %1%", triangulation.triangles.size())

    //todo wrapper to mesh

    ecad::utils::ELayoutRetriever retriever(layout);
    for (size_t layer = 0; layer < compact->TotalLayers(); ++layer) {
        EPrismaThermalModel::PrismaLayer prismaLayer;
        prismaLayer.id = layer;
        [[maybe_unused]] auto check = compact->GetLayerHeightThickness(layer, prismaLayer.elevation, prismaLayer.thickness); { ECAD_ASSERT(check) }
        model->AppendLayer(std::move(prismaLayer));
    }

    std::unordered_set<EMaterialId> fluidMaterials;
    auto matIter = layout->GetDatabase()->GetMaterialDefIter();
    while (auto * material = matIter->Next()) {
        if (EMaterialType::Fluid == material->GetMaterialType())
            fluidMaterials.emplace(material->GetMaterialId());
    }

    std::unordered_map<size_t, std::unordered_map<size_t, size_t> > templateIdMap;//[layer, [tempId, eleId]]

    auto buildOnePrismaLayer = [&](size_t index) {
        auto & prismaLayer = model->layers.at(index);   
        auto & idMap = templateIdMap.emplace(prismaLayer.id, std::unordered_map<size_t, size_t>{}).first->second;    
        for (size_t it = 0; it < triangulation.triangles.size(); ++it) {
            ECAD_ASSERT(compact->hasPolygon(prismaLayer.id))
            auto ctPoint = tri::TriangulationUtility<EPoint2D>::GetCenter(triangulation, it).Cast<ECoord>();
            auto pid = compact->SearchPolygon(prismaLayer.id, ctPoint);
            if (pid != invalidIndex) {
                if (not fluidMaterials.count(compact->materials.at(pid))) {
                    auto & ele = prismaLayer.AddElement(it);
                    idMap.emplace(it, ele.id);
                    ele.matId = compact->materials.at(pid);
                    ele.netId = compact->nets.at(pid);
                    auto iter = compact->powerBlocks.find(pid);
                    if (iter != compact->powerBlocks.cend()) {
                        auto area = tri::TriangulationUtility<EPoint2D>::GetTriangleArea(triangulation, it);
                        ele.avePower = area * iter->second.powerDensity;
                    }
                }
            }
        }
        ECAD_TRACE("layer %1%'s total elements: %2%", index, prismaLayer.elements.size())
    };

    for (size_t index = 0; index < model->TotalLayers(); ++index)
        buildOnePrismaLayer(index);
    
    //build connection
    for (size_t index = 0; index < model->TotalLayers(); ++index) {
        auto & layer = model->layers.at(index);
        auto & elements = layer.elements;
        const auto & currIdMap = templateIdMap.at(layer.id);
        for (auto & ele : elements) {
            //layer neighbors
            const auto & triangle = triangulation.triangles.at(ele.templateId);
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
                    lowerEle.neighbors[EPrismaThermalModel::PrismaElement::TOP_NEIGHBOR_INDEX] = ele.id;
                    ele.neighbors[EPrismaThermalModel::PrismaElement::BOT_NEIGHBOR_INDEX] = lowerEle.id;
                }
            }
        }
    }
    auto scaleH2Unit = coordUnits.Scale2Unit();
    auto scale2Meter = coordUnits.toUnit(coordUnits.toCoord(1), ECoordUnits::Unit::Meter);
    model->BuildPrismaModel(scaleH2Unit, scale2Meter);

    for (const auto & bondwire : compact->bondwires)
        model->AddBondWire(bondwire);
    ECAD_TRACE("total line elements: %1%", model->TotalLineElements())

    if (not settings.workDir.empty()) { 
        auto meshFile = settings.workDir + ECAD_SEPS + "mesh.vtk";
        io::GenerateVTKFile(meshFile, *model);
    }

    //htc
    model->SetTopBotBCType(EGridThermalModel::BCType::HTC, EGridThermalModel::BCType::HTC);
    model->SetUniformTopBotBCValue(0, 2750);
    model->uniformBcSide = 0;

    return std::unique_ptr<IModel>(model);
}

}//namespace ecad::extraction