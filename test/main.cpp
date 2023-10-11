#include "EDataMgr.h"
#include "extension/ECadExtension.h"
#include <cassert>
using namespace ecad;
int main()
{
    std::string err;
    std::string dmc = "/home/bwu/code/myRepo/ecad/test/data/dmcdom/import.dmc";
    std::string dom = "/home/bwu/code/myRepo/ecad/test/data/dmcdom/import.dom";
    auto database = ext::CreateDatabaseFromDomDmc("test_dmcdom", dmc, dom);
    assert(err.empty());
    assert(database != nullptr);
}