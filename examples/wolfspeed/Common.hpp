#pragma once
#include "../test/TestData.hpp"
#include "EDataMgr.h"

using namespace ecad;
UPtr<EPrismThermalModelExtractionSettings> ExtractionSettings(const std::string & workDir)
{
    EFloat htc = 5000;
    EPrismThermalModelExtractionSettings prismSettings;
    prismSettings.threads = EDataMgr::Instance().Threads();
    prismSettings.workDir = workDir;
    prismSettings.botUniformBC.type = EThermalBondaryCondition::BCType::HTC;
    prismSettings.botUniformBC.value = htc;
    prismSettings.meshSettings.genMeshByLayer = false;
    if (prismSettings.meshSettings.genMeshByLayer)
        prismSettings.meshSettings.imprintUpperLayer = true;
    prismSettings.meshSettings.iteration = 3e3;
    prismSettings.meshSettings.minAlpha = 15;
    prismSettings.meshSettings.minLen = 1e-3;
    prismSettings.meshSettings.maxLen = 1000;
    prismSettings.meshSettings.tolerance = 0;
    prismSettings.layerCutSettings.layerTransitionRatio = 3;
    prismSettings.meshSettings.dumpMeshFile = true;
    prismSettings.layerCutSettings.dumpSketchImg = true;

    EFloat topHTC = htc;
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, 4.7}, {-20.35, 8.7}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, -8.7}, {-20.35, -4.7}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, 11.5}, {9.75, 17}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, -17}, {9.75, -11.5}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, 11.5}, {-2.55, 17}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, -17}, {-2.55, -11.5}), EThermalBondaryCondition::BCType::HTC, topHTC);
    return std::make_unique<EPrismThermalModelExtractionSettings>(prismSettings);
}