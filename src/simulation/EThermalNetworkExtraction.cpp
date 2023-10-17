#include "EThermalNetworkExtraction.h"

#include "utilities/EMetalFractionMapping.h"
#include "solvers/EThermalNetworkSolver.h"
#include "generic/tools/FileSystem.hpp"

#include "interfaces/ILayoutView.h"
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
    GENERIC_ASSERT(layers.size() == mf->size());

    for(size_t i = 0; i < layers.size(); ++i) {
        auto name = layers.at(i)->GetName();
        auto stackupLayer = layers.at(i)->GetStackupLayerFromLayer();
        auto thickness = coordUnits.toUnit(stackupLayer->GetThickness(), ECoordUnits::Unit::Meter);
        auto layerMetalFraction = mf->at(i);
        EGridThermalLayer layer(name, layerMetalFraction);
        layer.SetThickness(thickness);

        GENERIC_ASSERT(model.AppendLayer(std::move(layer)));
    }

    //wbtest
    ESimVal iniT = 25;
    //power
    size_t pwrTiles = 0;
    ESimVal totalP = 1;//W
    auto modelSize = model.ModelSize();
    const auto & topLayer = *(mf->front());
    for (size_t x = 0; x < modelSize.x; ++x)
        for (size_t y = 0; y < modelSize.y; ++y)
            if (topLayer(x, y) > 0.5) pwrTiles +=1;
    std::cout << "power tiles: " << EValue(pwrTiles) / modelSize.x / modelSize.x << std::endl;
    ESimVal aveP = totalP / EValue(pwrTiles);

    auto gridPower = EGridData(nx, ny, 0);
    for (size_t x = 0; x < modelSize.x; ++x)
        for (size_t y = 0; y < modelSize.y; ++y)
            if (topLayer(x, y) > 0.5) gridPower(x, y) = aveP;

    auto powerModel = std::make_shared<EGridPowerModel>(ESize2D(nx, ny));
    powerModel->AddSample(iniT, std::move(gridPower));

    model.SetPowerModel(0, powerModel);

    //htc
    auto bcModel = std::make_shared<EGridBCModel>(ESize2D(nx, ny));
    bcModel->AddSample(iniT, EGridData(nx, ny, 1000));

    model.SetTopBotBCModel(bcModel, bcModel);
    model.SetTopBotBCType(EGridThermalModel::BCType::HTC, EGridThermalModel::BCType::HTC);

    std::vector<ESimVal> results;
    EGridThermalNetworkDirectSolver solver(model);
    if(!solver.Solve(iniT, results)) return false;
    
    auto htMap = std::unique_ptr<ELayoutMetalFraction>(new ELayoutMetalFraction);
    for(size_t z = 0; z < modelSize.z; ++z)
        htMap->push_back(std::make_shared<ELayerMetalFraction>(modelSize.x, modelSize.y));

    for(size_t i = 0; i < results.size(); ++i){
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