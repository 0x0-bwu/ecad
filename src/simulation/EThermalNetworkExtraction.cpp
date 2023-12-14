#include "EThermalNetworkExtraction.h"

#include "models/thermal/EPrismaThermalModel.h"
#include "models/thermal/EGridThermalModel.h"
#include "utilities/EMetalFractionMapping.h"
#include "solvers/EThermalNetworkSolver.h"
#include "generic/tools/FileSystem.hpp"

#include "Mesher2D.h"
#include "interfaces/ILayoutView.h"
#include "interfaces/IComponent.h"
#include "interfaces/IPrimitive.h"
#include "interfaces/ILayer.h"

namespace ecad {
namespace esim {

using namespace eutils;
using namespace esolver;
using namespace emodel::etherm;

ECAD_INLINE void EThermalNetworkExtraction::SetExtractionSettings(EThermalNetworkExtractionSettings settings)
{
    m_settings = std::move(settings);
}

ECAD_INLINE UPtr<EThermalModel> EThermalNetworkExtraction::GenerateGridThermalModel(Ptr<ILayoutView> layout)
{
    ECAD_EFFICIENCY_TRACK("generate grid thermal model")

    EMetalFractionMappingSettings settings;
    settings.grid = m_settings.grid;
    settings.regionExtTop = m_settings.regionExtTop;
    settings.regionExtBot = m_settings.regionExtBot;
    settings.regionExtLeft  = m_settings.regionExtLeft;
    settings.regionExtRight = m_settings.regionExtRight;
    settings.mergeGeomBeforeMapping = m_settings.mergeGeomBeforeMetalMapping;
    if (not m_settings.outDir.empty() && m_settings.dumpDensityFile)
        settings.outFile = m_settings.outDir + GENERIC_FOLDER_SEPS + "mf.txt";

    ELayoutMetalFractionMapper mapper(settings);
    if (not mapper.GenerateMetalFractionMapping(layout)) return nullptr;

    auto mf = mapper.GetLayoutMetalFraction();
    auto mfInfo = mapper.GetMetalFractionInfo();
    if (nullptr == mf || nullptr == mfInfo) return nullptr;

    if (not m_settings.outDir.empty() && m_settings.dumpDensityFile) {
        auto densityFile = m_settings.outDir + GENERIC_FOLDER_SEPS + "density.txt";
        WriteThermalProfile(*mfInfo, *mf, densityFile);
    }

    const auto & coordUnits = layout->GetCoordUnits();

    auto [nx, ny] = mfInfo->grid;
    EGridThermalModel model(ESize2D(nx, ny));

    auto rx = coordUnits.toUnit(mfInfo->stride[0], ECoordUnits::Unit::Meter);
    auto ry = coordUnits.toUnit(mfInfo->stride[1], ECoordUnits::Unit::Meter);
    model.SetResolution(rx, ry);

    std::vector<Ptr<ILayer> > layers;
    layout->GetStackupLayers(layers);
    ECAD_ASSERT(layers.size() == mf->size());

    std::unordered_map<ELayerId, size_t> lyrMap;
    for(size_t i = 0; i < layers.size(); ++i) {
        auto name = layers.at(i)->GetName();
        auto stackupLayer = layers.at(i)->GetStackupLayerFromLayer();
        auto thickness = coordUnits.toCoordF(stackupLayer->GetThickness());
        thickness = coordUnits.toUnit(thickness, ECoordUnits::Unit::Meter);
        auto layerMetalFraction = mf->at(i);
        EGridThermalLayer layer(name, layerMetalFraction);
        layer.SetThickness(thickness);
        auto index = model.AppendLayer(std::move(layer));
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
            model.AppendJumpConnection(ESize3D(index1, 0), ESize3D(index2, 0), alpha);
        }
    }

    ESimVal iniT = 25;
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
            else model.AddPowerModel(lyrMap.at(layer), std::shared_ptr<EThermalPowerModel>(new EBlockPowerModel(ll, ur, power)));
        }
    }

    for (auto & [lyrId, gridData] : gridMap) {
        auto powerModel = new EGridPowerModel(ESize2D(nx, ny));
        powerModel->GetTable().AddSample(iniT, std::move(gridData));
        model.AddPowerModel(lyrId, std::shared_ptr<EThermalPowerModel>(powerModel));
    }

    //htc
    model.SetTopBotBCType(EGridThermalModel::BCType::HTC, EGridThermalModel::BCType::HTC);
    model.SetUniformTopBotBCValue(invalidSimVal, 2750);
    // auto bcModel = std::make_shared<EGridBCModel>(ESize2D(nx, ny));
    // bcModel->AddSample(iniT, EGridData(nx, ny, 2750));
    // model.SetTopBotBCModel(nullptr, bcModel);

    std::vector<ESimVal> results;
    EGridThermalNetworkStaticSolver solver(model);
    // EGridThermalNetworkTransientSolver solver(model);
    EThermalNetworkSolveSettings solverSettings;
    if (m_settings.dumpSpiceFile)
        solverSettings.spiceFile = m_settings.outDir + GENERIC_FOLDER_SEPS + "spice.sp";
    solver.SetSolveSettings(solverSettings);
    if (not solver.Solve(iniT, results)) return nullptr;

    auto modelSize = model.ModelSize();
    auto htMap = std::unique_ptr<ELayoutMetalFraction>(new ELayoutMetalFraction);
    for (size_t z = 0; z < modelSize.z; ++z)
        htMap->push_back(std::make_shared<ELayerMetalFraction>(modelSize.x, modelSize.y));

    for (size_t i = 0; i < results.size(); ++i){
        auto gridIndex = model.GetGridIndex(i);
        auto lyrHtMap = htMap->at(gridIndex.z);
        (*lyrHtMap)(gridIndex.x, gridIndex.y) = results[i];
    }

    auto htMapInfo = *mfInfo;
    if(!m_settings.outDir.empty() && m_settings.dumpTemperatureFile) {
        auto tFile = m_settings.outDir + GENERIC_FOLDER_SEPS + "temperature.txt";
        WriteThermalProfile(htMapInfo, *htMap, tFile);
    }

    using ValueType = typename ELayerMetalFraction::ResultType;
    if (not m_settings.outDir.empty() && m_settings.dumpHotmaps) {        
        for (size_t index = 0; index < mfInfo->layers.size(); ++index) {
            auto lyr = htMap->at(index);
            auto min = lyr->MaxOccupancy(std::less<ValueType>());
            auto max = lyr->MaxOccupancy(std::greater<ValueType>());
            auto range = max - min;
            auto rgbaFunc = [&min, &range](ValueType d) {
                int r, g, b, a = 255;
                generic::color::RGBFromScalar((d - min) / range, r, g, b);
                return std::make_tuple(r, g, b, a);
            };
            std::cout << "layer: " << index + 1 << ", min: " << min << ", max: " << max << std::endl;   
            std::string filepng = m_settings.outDir + GENERIC_FOLDER_SEPS + std::to_string(index) + ".png";
            lyr->WriteImgProfile(filepng, rgbaFunc);
        }
    }
    return std::make_unique<EThermalModel>(std::move(model));
}

ECAD_INLINE UPtr<EThermalModel> EThermalNetworkExtraction::GeneratePrismaThermalModel(Ptr<ILayoutView> layout)
{
    ECAD_EFFICIENCY_TRACK("generate prisma thermal model")
    auto compact = makeCompactLayout(layout);
    auto compactModelFile = m_settings.outDir + GENERIC_FOLDER_SEPS + "compact.png";
    compact->WriteImgView(compactModelFile, 1024);

    //todo wrapper to mesh
    using namespace emesh;
    using namespace generic::geometry;
    std::list<tri::IndexEdge> edges;
    std::vector<Point2D<ECoord> > points;
    std::vector<Segment2D<ECoord> > segments;
    tri::Triangulation<Point2D<ECoord> > triangulation;
    MeshFlow2D::ExtractIntersections(compact->polygons, segments);
    MeshFlow2D::ExtractTopology(segments, points, edges);
    MeshFlow2D::TriangulatePointsAndEdges(points, edges, triangulation);
    MeshFlow2D::TriangulationRefinement(triangulation, math::Rad(15), 100, 1e3, 1000);
    auto meshTemplateFile = m_settings.outDir + GENERIC_FOLDER_SEPS + "mesh.png";
    GeometryIO::WritePNG(meshTemplateFile, triangulation, 2048);
    std::cout << "total elements: " << triangulation.triangles.size() << std::endl;
    //todo wrapper to mesh

    EPrismaThermalModel model;
    std::vector<Ptr<ILayer> > layers;//todo, refinement
    std::vector<Ptr<IStackupLayer> > stackupLayers;
    layout->GetStackupLayers(layers);
    for (auto layer : layers) {
        EPrismaThermalModel::PrismaLayer prismaLayer;
        auto stackupLayer = layer->GetStackupLayerFromLayer();
        prismaLayer.layerId = layer->GetLayerId();
        prismaLayer.elevation = stackupLayer->GetElevation();
        prismaLayer.thickness = stackupLayer->GetThickness();
        //todo material
        model.AppendLayer(std::move(prismaLayer));
    }

    std::unordered_map<ELayerId, std::unordered_map<size_t, size_t> > templateIdMap;//[ELayerId, [tempId, eleId]]

    auto buildOnePrismaLayer = [&](size_t index) {
        auto & prismaLayer = model.layers.at(index);   
        auto idMap = templateIdMap.emplace(prismaLayer.layerId, std::unordered_map<size_t, size_t>{}).first->second;    
        for (size_t it = 0; it < triangulation.triangles.size(); ++it) {
            if (compact->hasPolygon(prismaLayer.layerId)) {
                auto ctPoint = tri::TriangulationUtility<EPoint2D>::GetCenter(triangulation, it).Cast<ECoord>();
                auto pid = compact->SearchPolygon(prismaLayer.layerId, ctPoint);
                if (pid != invalidIndex) {
                    auto & ele = prismaLayer.AddElement(it);
                    idMap.emplace(it, ele.id);
                    ele.matId = compact->materials.at(pid);
                    ele.netId = compact->nets.at(pid);
                }
                else if (prismaLayer.dielectricMatId != EMaterialId::noMaterial) {
                    auto & ele = prismaLayer.AddElement(it);
                    idMap.emplace(it, ele.id);
                    ele.matId = prismaLayer.dielectricMatId;
                    ele.netId = ENetId::noNet;    
                }
            }
            else if (prismaLayer.dielectricMatId != EMaterialId::noMaterial) {
                auto & ele = prismaLayer.AddElement(it);
                idMap.emplace(it, ele.id);
                ele.matId = prismaLayer.dielectricMatId;
                ele.netId = ENetId::noNet;    
            }  
        }
        std::cout << "layer " << index << " 's total elements: " << prismaLayer.elements.size() << std::endl; 
    };

    for (size_t index = 0; index < model.TotalLayers(); ++index)
        buildOnePrismaLayer(index);
    
    //build connection
    for (size_t index = 1; index < model.TotalLayers(); ++index) {
        auto & upperLayer = model.layers.at(index - 1);
        auto & lowerLayer = model.layers.at(index);
        auto & upperEles = upperLayer.elements;
        auto & lowerEles = lowerLayer.elements;
        const auto & upperIdMap = templateIdMap.at(upperLayer.layerId);
        const auto & lowerIdMap = templateIdMap.at(lowerLayer.layerId);
        for (auto & ele : upperEles) {
            //layer neighbors
            const auto & triangle = triangulation.triangles.at(ele.templateId);
            for (size_t nid = 0; nid < triangle.neighbors.size(); ++nid) {
                if (tri::noNeighbor == triangle.neighbors.at(nid)) continue;
                auto iter = upperIdMap.find(triangle.neighbors.at(nid));
                if (iter != upperIdMap.cend()) ele.neighbors[nid] = iter->second;
            }
            auto iter = lowerIdMap.find(ele.templateId);
            if (iter != lowerIdMap.cend()) {
                auto & lowerEle = lowerEles.at(iter->second);
                lowerEle.neighbors[EPrismaThermalModel::PrismaElement::TOP_NEIGHBOR_INDEX] = ele.id;
                ele.neighbors[EPrismaThermalModel::PrismaElement::BOT_NEIGHBOR_INDEX] = lowerEle.id;
            }
        }
    }

    return nullptr;
}

}//namespace esim
}//namespace ecad