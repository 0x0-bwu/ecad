#include "../test/TestData.hpp"
#include "EDataMgr.h"

int main(int argc, char * argv[])
{
    using namespace ecad;
    using namespace generic::fs;

    auto & eDataMgr = EDataMgr::Instance();
    eDataMgr.SetThreads(1);
    std::string filename = ecad_test::GetTestDataPath() + "/kicad/jetson-nano-baseboard.kicad_pcb";
    auto database = eDataMgr.CreateDatabaseFromKiCad("jetson-nano-baseboard", filename);

    if (nullptr == database) return EXIT_FAILURE;
    EDataMgr::Instance().ShutDown();

    return EXIT_SUCCESS;
}