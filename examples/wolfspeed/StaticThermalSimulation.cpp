#include <boost/stacktrace.hpp>
#include <string_view>
#include <filesystem>
#include <cassert>
#include <csignal>

#include "../test/TestData.hpp"
#include "generic/tools/StringHelper.hpp"
#include "utils/ELayoutRetriever.h"
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

EPrismaThermalModelExtractionSettings ExtractionSettings(const std::string & workDir)
{
    EFloat htc = 5000;
    EPrismaThermalModelExtractionSettings prismaSettings;
    prismaSettings.threads = eDataMgr.Threads();
    prismaSettings.workDir = workDir;
    prismaSettings.botUniformBC.type = EThermalBondaryCondition::BCType::HTC;
    prismaSettings.botUniformBC.value = htc;
    prismaSettings.meshSettings.genMeshByLayer = false;
    if (prismaSettings.meshSettings.genMeshByLayer)
        prismaSettings.meshSettings.imprintUpperLayer = true;
    prismaSettings.meshSettings.iteration = 1e5;
    prismaSettings.meshSettings.minAlpha = 20;
    prismaSettings.meshSettings.minLen = 1e-5;
    prismaSettings.meshSettings.maxLen = 1;
    prismaSettings.meshSettings.tolerance = 0;
    prismaSettings.layerCutSettings.layerTransitionRatio = 3;
    prismaSettings.meshSettings.dumpMeshFile = true;
    prismaSettings.layerCutSettings.dumpSketchImg = true;

    EFloat topHTC = htc;
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, 4.7}, {-20.35, 8.7}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, -8.7}, {-20.35, -4.7}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, 11.5}, {9.75, 17}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, -17}, {9.75, -11.5}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, 11.5}, {-2.55, 17}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, -17}, {-2.55, -11.5}), EThermalBondaryCondition::BCType::HTC, topHTC);
    return prismaSettings;
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

void TransientThermalFlow(Ptr<ILayoutView> layout, const std::string & workDir, EFloat period, EFloat duty)
{
    auto extractionSettings = ExtractionSettings(workDir);
    extractionSettings.meshSettings.genMeshByLayer = true;
    extractionSettings.meshSettings.imprintUpperLayer = true;
    extractionSettings.meshSettings.maxLen = 2;
    extractionSettings.meshSettings.minAlpha = 20;
    extractionSettings.layerCutSettings.layerTransitionRatio = 0;

    EThermalTransientSimulationSetup setup;
    setup.workDir = workDir;
    setup.monitors = GetDieMonitors(layout);
    setup.settings.envTemperature = {25, ETemperatureUnit::Celsius};
    setup.settings.mor = false;
    setup.settings.verbose = true;
    setup.settings.dumpResults = true;
    setup.settings.duration = 10;
    setup.settings.step = period * duty / 10;
    setup.settings.temperatureDepend = true;
    setup.settings.samplingWindow = setup.settings.duration;
    setup.settings.minSamplingInterval = period * duty / 10;
    setup.settings.absoluteError = 1e-5;
    setup.settings.relativeError = 1e-5;
    setup.settings.threads = eDataMgr.Threads();
    EThermalTransientExcitation excitation = [period, duty](EFloat t, size_t scen) -> EFloat { 
        EFloat tm = std::fmod(t, period); 
        switch (scen) {
            case 0 : return 1;
            case 1 : return tm < duty * period ? 1 : 0;
            case 2 : return 0.5 * period < tm && tm < (duty + 0.5) * period ? 1 : 0; 
            default : { ECAD_ASSERT(false); return 0; }
        }
    };
    setup.settings.excitation = &excitation;
    auto [minT, maxT] = layout->RunThermalSimulation(extractionSettings, setup);    
    ECAD_TRACE("minT: %1%, maxT: %2%", minT, maxT)
}

int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &SignalHandler);
    ::signal(SIGABRT, &SignalHandler);

    ecad::EDataMgr::Instance().Init(ecad::ELogLevel::Trace);

    std::string filename = generic::fs::DirName(__FILE__).string() + ECAD_SEPS + "data" + ECAD_SEPS + "design" + ECAD_SEPS + "CAS300M12BM2.ecad";

    eDataMgr.Load
    StaticThermalFlow(layout, workDir);
    // TransientThermalFlow(layout, workDir, 1, 0.5);
    ecad::EDataMgr::Instance().ShutDown();
    return EXIT_SUCCESS;
}