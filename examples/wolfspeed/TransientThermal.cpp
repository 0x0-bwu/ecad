#include <boost/stacktrace.hpp>
#include <string_view>
#include <filesystem>
#include <cassert>
#include <csignal>

#include "Common.hpp"
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

void TransientThermalFlow(Ptr<ILayoutView> layout, const std::string & workDir, EFloat period, EFloat duty)
{
    auto extractionSettings = ExtractionSettings(workDir);
    // extractionSettings->meshSettings.genMeshByLayer = true;
    // extractionSettings->meshSettings.imprintUpperLayer = true;
    // extractionSettings->meshSettings.maxLen = 2;
    // extractionSettings->meshSettings.minAlpha = 20;
    // extractionSettings->layerCutSettings.layerTransitionRatio = 0;

    EThermalTransientSimulationSetup setup;
    setup.workDir = workDir;
    setup.monitors = GetDieMonitors(layout);
    setup.settings.envTemperature = {25, ETemperatureUnit::Celsius};
    setup.settings.mor = true;
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
    setup.extractionSettings = std::move(extractionSettings);
    EThermalTransientExcitation excitation = [period, duty](EFloat t, size_t scen) -> EFloat { 
        EFloat tm = std::fmod(t, period); 
        switch (scen) {
            case 0 : return 1;
            case 1 : return tm < duty * period ? 1 : 0;
            case 2 : return 0.5 * period < tm && tm < (duty + 0.5) * period ? 1 : 0; 
            default : { ECAD_ASSERT(false) return 0; }
        }
    };
    auto [minT, maxT] = layout->RunThermalSimulation(setup, excitation);    
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
    std::string workDir = generic::fs::DirName(__FILE__).string() + ECAD_SEPS + "data" + ECAD_SEPS + "simulation" + ECAD_SEPS + "transient";
    auto layout = cell->GetFlattenedLayoutView();
    TransientThermalFlow(layout, workDir, 0.02, 0.5);
    ecad::EDataMgr::Instance().ShutDown();
    return EXIT_SUCCESS;
}