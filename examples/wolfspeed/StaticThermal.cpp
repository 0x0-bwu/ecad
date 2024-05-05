#include <boost/stacktrace.hpp>
#include <string_view>
#include <filesystem>
#include <cassert>
#include <csignal>

#include "../test/TestData.hpp"
#include "generic/tools/StringHelper.hpp"
#include "utility/ELayoutRetriever.h"
#include "EDataMgr.h"

void SignalHandler(int signum)
{
    ::signal(signum, SIG_DFL);
    std::cout << boost::stacktrace::stacktrace();
    ::raise(SIGABRT);
}

using namespace ecad;
using namespace generic;
auto & eDataMgr = EDataMgr::Instance();

std::vector<FPoint3D> GetDieMonitors(CPtr<ILayoutView> layout)
{
    std::vector<FPoint3D> monitors;
    utils::ELayoutRetriever retriever(layout);
    std::vector<std::string> cellInsts{"TopBridge1", "TopBridge2", "BotBridge1", "BotBridge2"};
    std::vector<std::string> components{"Die1", "Die2", "Die3"};
    for(const auto & cellInst : cellInsts) {
        for (const auto & component : components) {
            std::string name = cellInst + eDataMgr.HierSep() + component;
            auto comp = layout->FindComponentByName(name);
            EFloat elevation, thickness;
            retriever.GetComponentHeightThickness(comp, elevation, thickness);
            auto loc = layout->GetCoordUnits().toUnit(comp->GetBoundingBox().Center());
            monitors.emplace_back(loc[0], loc[1], elevation - 0.1 * thickness);
        }
    }
    return monitors;
}

EPrismThermalModelExtractionSettings ExtractionSettings(const std::string & workDir)
{
    EFloat htc = 5000;
    EPrismThermalModelExtractionSettings prismSettings;
    prismSettings.threads = eDataMgr.Threads();
    prismSettings.workDir = workDir;
    prismSettings.botUniformBC.type = EThermalBondaryCondition::BCType::HTC;
    prismSettings.botUniformBC.value = htc;
    prismSettings.meshSettings.genMeshByLayer = false;
    if (prismSettings.meshSettings.genMeshByLayer)
        prismSettings.meshSettings.imprintUpperLayer = true;
    prismSettings.meshSettings.iteration = 1e5;
    prismSettings.meshSettings.minAlpha = 20;
    prismSettings.meshSettings.minLen = 1e-5;
    prismSettings.meshSettings.maxLen = 1;
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
    return prismSettings;
}

void StaticThermalFlow(Ptr<ILayoutView> layout, const std::string & workDir)
{
    auto extractionSettings = ExtractionSettings(workDir);
    EThermalStaticSimulationSetup setup;
    setup.settings.iteration = 100;
    setup.settings.dumpHotmaps = true;
    setup.settings.envTemperature = {25, ETemperatureUnit::Celsius};
    setup.workDir = workDir;
    setup.monitors = GetDieMonitors(layout);
    auto [minT, maxT] = layout->RunThermalSimulation(extractionSettings, setup);    
    ECAD_TRACE("minT: %1%, maxT: %2%", minT, maxT)
}

int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &SignalHandler);
    ::signal(SIGABRT, &SignalHandler);

    eDataMgr.Init(ecad::ELogLevel::Trace);

    std::string filename = generic::fs::DirName(__FILE__).string() + ECAD_SEPS + "data" + ECAD_SEPS + "design" + ECAD_SEPS + "CAS300M12BM2.ecad";
    auto database = eDataMgr.LoadDatabase(filename);
    if (nullptr == database) return EXIT_FAILURE;
    auto cell = database->FindCellByName("Base");
    std::string workDir = generic::fs::DirName(__FILE__).string() + ECAD_SEPS + "data" + ECAD_SEPS + "simulation" + ECAD_SEPS + "static";
    auto layout = cell->GetFlattenedLayoutView();
    std::cout << layout->GetName() << std::endl;
    auto iter = layout->GetComponentIter();
    while (auto * comp = iter->Next()) {
        std::cout << comp->GetName() << std::endl;
    }
    StaticThermalFlow(layout, workDir);
    ecad::EDataMgr::Instance().ShutDown();
    return EXIT_SUCCESS;
}