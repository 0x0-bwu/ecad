#ifndef ECAD_TEST_EXTENSIONTEST_HPP
#define ECAD_TEST_EXTENSIONTEST_HPP
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "extension/ECadExtension.h"
#include "Interface.h"
#include "EDataMgr.h"
using namespace boost::unit_test;
using namespace ecad;

extern const std::string testDataPath;

void t_extension_dmcdom()
{
    std::string err;
    std::string dmc = testDataPath + "/extension/dmcdom/import.dmc";
    std::string dom = testDataPath + "/extension/dmcdom/import.dom";
    auto database = ext::CreateDatabaseFromDomDmc("test_dmcdom", dmc, dom);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(database != nullptr);

    std::vector<Ptr<ICell> > cells;
    database->GetCircuitCells(cells);
    BOOST_CHECK(cells.size() == 1);
    
    auto layout = cells.front()->GetLayoutView();
    BOOST_CHECK(76  == layout->GetNetCollection()->Size());
    BOOST_CHECK(7   == layout->GetLayerCollection()->Size());
    BOOST_CHECK(728 == layout->GetConnObjCollection()->Size());
    BOOST_CHECK(0   == layout->GetCellInstCollection()->Size());
    BOOST_CHECK(481 == layout->GetPrimitiveCollection()->Size());
    BOOST_CHECK(247 == layout->GetPadstackInstCollection()->Size());

    EDataMgr::Instance().ShutDown();
}

void t_extension_gds()
{
    std::string err;
    std::string gds = testDataPath + "/extension/gdsii/ringo.gds";
    auto database = ext::CreateDatabaseFromGds("test_gds", gds, &err);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(database != nullptr);

    EDataMgr::Instance().ShutDown();
}

test_suite * create_ecad_extension_test_suite()
{
    test_suite * extension_suite = BOOST_TEST_SUITE("s_extension_test");
    //
    extension_suite->add(BOOST_TEST_CASE(&t_extension_dmcdom));
    extension_suite->add(BOOST_TEST_CASE(&t_extension_gds));
    //
    return extension_suite;
}
#endif//ECAD_TEST_EXTENSIONTEST_HPP