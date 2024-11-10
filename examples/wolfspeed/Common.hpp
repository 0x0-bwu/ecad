#pragma once
#include "../test/TestData.hpp"
#include "EDataMgr.h"

using namespace ecad;
UPtr<EPrismThermalModelExtractionSettings> ExtractionSettings(const std::string & workDir)
{
    EFloat htc = 5000;
    EPrismThermalModelExtractionSettings prismSettings(workDir, EDataMgr::Instance().Threads(), {});
    prismSettings.botUniformBC.type = EThermalBoundaryCondition::BCType::HTC;
    prismSettings.botUniformBC.value = htc;
    prismSettings.meshSettings.genMeshByLayer = true;
    if (prismSettings.meshSettings.genMeshByLayer)
        prismSettings.meshSettings.imprintUpperLayer = false;
    prismSettings.meshSettings.iteration = 1e5;
    prismSettings.meshSettings.minAlpha = 15;
    prismSettings.meshSettings.minLen = 1e-3;
    prismSettings.meshSettings.maxLen = 3;
    prismSettings.meshSettings.tolerance = 0;
    prismSettings.meshSettings.dumpMeshFile = true;
    prismSettings.layerCutSettings.layerTransitionRatio = 0;
    prismSettings.layerCutSettings.dumpSketchImg = true;

    EFloat topHTC = htc;
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, 4.7}, {-20.35, 8.7}), EThermalBoundaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, -8.7}, {-20.35, -4.7}), EThermalBoundaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, 11.5}, {9.75, 17}), EThermalBoundaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, -17}, {9.75, -11.5}), EThermalBoundaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, 11.5}, {-2.55, 17}), EThermalBoundaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, -17}, {-2.55, -11.5}), EThermalBoundaryCondition::BCType::HTC, topHTC);
    return std::make_unique<EPrismThermalModelExtractionSettings>(prismSettings);
}