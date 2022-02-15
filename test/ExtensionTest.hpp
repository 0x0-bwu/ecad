#ifndef ECAD_TEST_EXTENSIONTEST_HPP
#define ECAD_TEST_EXTENSIONTEST_HPP
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "extension/ECadExtension.h"
#include "TestData.hpp"
#include "Interface.h"
#include "EDataMgr.h"
using namespace boost::unit_test;
using namespace ecad;

void t_extension_dmcdom()
{
    std::string err;
    std::string dmc = ecad_test::GetTestDataPath() + "/extension/dmcdom/import.dmc";
    std::string dom = ecad_test::GetTestDataPath() + "/extension/dmcdom/import.dom";
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
    std::string ringoGds = ecad_test::GetTestDataPath() + "/extension/gdsii/ringo.gds";
    auto ringo = ext::CreateDatabaseFromGds("ringo", ringoGds, std::string{}, &err);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(ringo != nullptr);

    std::string testGds = ecad_test::GetTestDataPath() + "/extension/gdsii/test.gds";
    std::string testLyrMap = ecad_test::GetTestDataPath() + "/extension/gdsii/test.layermap";
    auto test = ext::CreateDatabaseFromGds("test", testGds, testLyrMap, &err);
    // BOOST_CHECK(err.empty());
    // BOOST_CHECK(test != nullptr);//wbtest

    EDataMgr::Instance().ShutDown();
}

void t_extension_xfl()
{
    std::string err;
    std::string fccspXfl = ecad_test::GetTestDataPath() + "/extension/xfl/fccsp.xfl";
    auto fccsp = ext::CreateDatabaseFromXfl("test", fccspXfl, &err);
    // BOOST_CHECK(err.empty());
    // BOOST_CHECK(ringo != nullptr);

    EDataMgr::Instance().ShutDown(); 
}

test_suite * create_ecad_extension_test_suite()
{
    test_suite * extension_suite = BOOST_TEST_SUITE("s_extension_test");
    //
    extension_suite->add(BOOST_TEST_CASE(&t_extension_dmcdom));
    extension_suite->add(BOOST_TEST_CASE(&t_extension_gds));
    extension_suite->add(BOOST_TEST_CASE(&t_extension_xfl));
    //
    return extension_suite;
}
#endif//ECAD_TEST_EXTENSIONTEST_HPP
