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
    std::string filename = ecad_test::GetTestDataPath() + "/kicad/test.kicad_pcb";
    auto database = eDataMgr.CreateDatabaseFromKiCad("test", filename);

    if (nullptr == database) return EXIT_FAILURE;
    EDataMgr::Instance().ShutDown();

    return EXIT_SUCCESS;
}