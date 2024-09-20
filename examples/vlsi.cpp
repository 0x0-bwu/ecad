#include <boost/stacktrace.hpp>
#include <string_view>
#include <filesystem>
#include <cassert>
#include <csignal>

#include "../test/TestData.hpp"
#include "EDataMgr.h"

void SignalHandler(int signum)
{
    ::signal(signum, SIG_DFL);
    std::cout << boost::stacktrace::stacktrace();
    ::raise(SIGABRT);
}

int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &SignalHandler);
    ::signal(SIGABRT, &SignalHandler);

    using namespace ecad;
    using namespace generic::fs;

    auto & eDataMgr = EDataMgr::Instance();
    eDataMgr.SetThreads(1);
    std::string filename = ecad_test::GetTestDataPath() + "/gdsii/4004.gds";
    std::string layermap = ecad_test::GetTestDataPath() + "/gdsii/4004.layermap";
    auto database = eDataMgr.CreateDatabaseFromGds("4004", filename, layermap);
    // std::string filename = ecad_test::GetTestDataPath() + "/xfl/fccsp.xfl";
    // auto database = eDataMgr.CreateDatabaseFromXfl("fccsp", filename);
    if (nullptr == database) return EXIT_FAILURE;

    std::vector<Ptr<ICell> > cells;
    database->GetCircuitCells(cells);
    assert(cells.size() == 1);
    
    auto layout = cells.front()->GetLayoutView();
    // auto bbox = layout->GetBoundary()->GetBBox();
    auto iter = layout->GetLayerCollection()->GetLayerIter();
    while (auto layer = iter->Next())
        std::cout << "thickness: " << layer->GetStackupLayerFromLayer()->GetThickness() << std::endl;

    ELayoutPolygonMergeSettings mergeSettings(eDataMgr.Threads(), {});
    mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->MergeLayerPolygons(mergeSettings);

    EDataMgr::Instance().ShutDown();

    return EXIT_SUCCESS;
}