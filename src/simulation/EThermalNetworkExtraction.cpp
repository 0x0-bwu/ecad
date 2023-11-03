#include "EThermalNetworkExtraction.h"

#include "utilities/EMetalFractionMapping.h"
#include "solvers/EThermalNetworkSolver.h"
#include "generic/tools/FileSystem.hpp"

#include "interfaces/ILayoutView.h"
#include "interfaces/IComponent.h"
#include "interfaces/IPrimitive.h"
#include "interfaces/ILayer.h"

namespace ecad {
namespace esim {

using namespace eutils;
using namespace esolver;

ECAD_INLINE void EThermalNetworkExtraction::SetExtractionSettings(EThermalNetworkExtractionSettings settings)
{
    m_settings = std::move(settings);
}

ECAD_INLINE bool EThermalNetworkExtraction::GenerateThermalNetwork(Ptr<ILayoutView> layout)
{
    ECAD_EFFICIENCY_TRACK("generate thermal network")

    EMetalFractionMappingSettings settings;
    settings.grid = m_settings.grid;
    settings.regionExtTop = m_settings.regionExtTop;
    settings.regionExtBot = m_settings.regionExtBot;
    settings.regionExtLeft  = m_settings.regionExtLeft;
    settings.regionExtRight = m_settings.regionExtRight;
    settings.mergeGeomBeforeMapping = m_settings.mergeGeomBeforeMetalMapping;
    if(!m_settings.outDir.empty() && m_settings.dumpDensityFile)
        settings.outFile = m_settings.outDir + GENERIC_FOLDER_SEPS + "mf.txt";

    ELayoutMetalFractionMapper mapper(settings);
    if(!mapper.GenerateMetalFractionMapping(layout)) return false;

    auto mf = mapper.GetLayoutMetalFraction();
    auto mfInfo = mapper.GetMetalFractionInfo();
    if(nullptr == mf || nullptr == mfInfo) return false;

    if(!m_settings.outDir.empty() && m_settings.dumpDensityFile) {
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

    for(size_t i = 0; i < layers.size(); ++i) {
        auto name = layers.at(i)->GetName();
        auto stackupLayer = layers.at(i)->GetStackupLayerFromLayer();
        auto thickness = coordUnits.toCoordF(stackupLayer->GetThickness());
        thickness = coordUnits.toUnit(thickness, ECoordUnits::Unit::Meter);
        auto layerMetalFraction = mf->at(i);
        EGridThermalLayer layer(name, layerMetalFraction);
        layer.SetThickness(thickness);
        [[maybe_unused]] auto res = model.AppendLayer(std::move(layer));
        ECAD_ASSERT(res)
    }

    ESimVal iniT = 25;
    auto gridPower = EGridData(nx, ny, 0);
    auto compIter = layout->GetComponentIter();
    while (auto * component = compIter->Next()) {
        if (auto power = component->GetLossPower(); power > 0) {
            auto bbox = component->GetBoundingBox();
            auto ll = mfInfo->GetIndex(bbox[0]);
            auto ur = mfInfo->GetIndex(bbox[1]);
            if (ll.isValid() && ur.isValid()) {
                auto totalTiles = (ur[1] - ll[1] + 1) * (ur[0] - ll[0] + 1);
                power /= totalTiles;
                for (size_t i = ll[0]; i <= ur[0]; ++i)
                    for (size_t j = ll[1]; j <= ur[1]; ++j)
                        gridPower(i, j) = power;
            }
        }
    }
    auto powerModel = std::make_shared<EGridPowerModel>(ESize2D(nx, ny));
    powerModel->AddSample(iniT, std::move(gridPower));
    model.SetPowerModel(0, powerModel);

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

    //htc
    auto bcModel = std::make_shared<EGridBCModel>(ESize2D(nx, ny));
    bcModel->AddSample(iniT, EGridData(nx, ny, 2750));

    model.SetTopBotBCModel(bcModel, bcModel);
    model.SetTopBotBCType(EGridThermalModel::BCType::HTC, EGridThermalModel::BCType::HTC);

    std::vector<ESimVal> results;
    EGridThermalNetworkDirectSolver solver(model);
    if (not solver.Solve(iniT, results)) return false;

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

#ifdef BOOST_GIL_IO_PNG_SUPPORT

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
#endif//BOOST_GIL_IO_PNG_SUPPORT
    return true;
}

}//namespace esim
}//namespace ecad