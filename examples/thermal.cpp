#include "generic/tools/FileSystem.hpp"
#include "simulation/EThermalNetworkExtraction.h"
#include "extension/ECadExtension.h"
#include "../test/TestData.hpp"
#include "EDataMgr.h"
#include <cassert>
int main(int argc, char * argv[])
{
    std::cout << "hello world" << std::endl;
    using namespace ecad;
    using namespace generic::filesystem;

    std::string err;
    std::string filename = ecad_test::GetTestDataPath() + "/gdsii/4004.gds";
    std::string layermap = ecad_test::GetTestDataPath() + "/gdsii/4004.layermap";
    auto database = ext::CreateDatabaseFromGds("4004", filename, layermap, &err);
    // std::string filename = ecad_test::GetTestDataPath() + "/xfl/fccsp.xfl";
    // auto database = ext::CreateDatabaseFromXfl("fccsp", filename, &err);
    if (not err.empty()) std::cout << err << std::endl;
    if (nullptr == database) return EXIT_FAILURE;

    std::vector<Ptr<ICell> > cells;
    database->GetCircuitCells(cells);
    assert(cells.size() == 1);
    
    auto layout = cells.front()->GetLayoutView();
    auto bbox = layout->GetBoundary()->GetBBox();
    std::cout << "nets:" << layout->GetNetCollection()->Size() << std::endl;//wbtest
    auto iter = layout->GetLayerCollection()->GetLayerIter();
    while (auto layer = iter->Next())
        std::cout << "thickness: " << layer->GetStackupLayerFromLayer()->GetThickness() << std::endl;

    // ELayoutPolygonMergeSettings mergeSettings;
    // mergeSettings.threads = 1;
    // mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    // layout->MergeLayerPolygons(mergeSettings);

    EThermalNetworkExtractionSettings extSettings;
    extSettings.threads = 4;
    extSettings.outDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
#ifdef BOOST_GIL_IO_PNG_SUPPORT
    extSettings.dumpHotmaps = true;
#endif//#ifdef BOOST_GIL_IO_PNG_SUPPORT
    extSettings.dumpDensityFile = true;
    extSettings.dumpTemperatureFile = true;

    size_t xGrid = 2000;
    extSettings.grid = {xGrid, static_cast<size_t>(xGrid * EValue(bbox.Width()) / bbox.Length())};
    extSettings.mergeGeomBeforeMetalMapping = false;

    esim::EThermalNetworkExtraction ne;
    ne.SetExtractionSettings(extSettings);
    ne.GenerateThermalNetwork(layout);

    EDataMgr::Instance().ShutDown();
}