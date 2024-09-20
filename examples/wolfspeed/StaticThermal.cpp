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
    EFloat elevation, thickness;
    std::vector<FPoint3D> monitors;
    utils::ELayoutRetriever retriever(layout);
    std::vector<std::string> cellInsts{"TopBridge1", "TopBridge2", "BotBridge1", "BotBridge2"};
    std::vector<std::string> components{"Die1", "Die2", "Die3"};
    for(const auto & cellInst : cellInsts) {
        for (const auto & component : components) {
            std::string name = cellInst + eDataMgr.HierSep() + component;
            auto cp = layout->FindComponentByName(name);
            retriever.GetComponentHeightThickness(cp, elevation, thickness);
            auto ct = cp->GetBoundingBox().Center();
            auto loc = layout->GetCoordUnits().toUnit(ct);
            monitors.emplace_back(loc[0], loc[1], elevation - 0.1 * thickness);
        }
    }
    return monitors;
}

void StaticThermalFlow(Ptr<ILayoutView> layout, const std::string & workDir)
{
    EThermalStaticSimulationSetup setup(workDir, eDataMgr.Threads(), {});
    setup.settings.iteration = 100;
    setup.settings.dumpHotmaps = true;
    setup.settings.envTemperature = {25, ETemperatureUnit::Celsius};
    setup.monitors = GetDieMonitors(layout);
    setup.extractionSettings = ExtractionSettings(workDir);
    std::vector<EFloat> temperatures;
    auto [minT, maxT] = layout->RunThermalSimulation(setup, temperatures);    
    ECAD_TRACE("minT: %1%, maxT: %2%", minT, maxT)
    ECAD_TRACE("temperatures:[%1%]", fmt::Fmt2Str(temperatures, ","));
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
    StaticThermalFlow(layout, workDir);
    ecad::EDataMgr::Instance().ShutDown();
    return EXIT_SUCCESS;
}