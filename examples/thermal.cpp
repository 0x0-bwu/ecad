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
    std::string fccspXfl = ecad_test::GetTestDataPath() + "/xfl/fccsp.xfl";
    auto fccsp = ext::CreateDatabaseFromXfl("fccsp", fccspXfl, &err);
    assert(err.empty());
    assert(fccsp != nullptr);

    std::vector<Ptr<ICell> > cells;
    fccsp->GetCircuitCells(cells);
    assert(cells.size() == 1);
    
    auto layout = cells.front()->GetLayoutView();

    ELayoutPolygonMergeSettings mergeSettings;
    mergeSettings.threads = 1;
    mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->MergeLayerPolygons(mergeSettings);

    EThermalNetworkExtractionSettings extSettings;
    extSettings.threads = 4;
    extSettings.outDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
#ifdef BOOST_GIL_IO_PNG_SUPPORT
    extSettings.dumpHotmaps = true;
#endif//#ifdef BOOST_GIL_IO_PNG_SUPPORT
    extSettings.dumpDensityFile = true;
    extSettings.dumpTemperatureFile = true;
    extSettings.grid = {500, 500};
    extSettings.mergeGeomBeforeMetalMapping = false;

    esim::EThermalNetworkExtraction ne;
    ne.SetExtractionSettings(extSettings);
    ne.GenerateThermalNetwork(layout);

    EDataMgr::Instance().ShutDown();
}