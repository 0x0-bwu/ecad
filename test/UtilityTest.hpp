#ifndef ECAD_TEST_UTILITY_HPP
#define ECAD_TEST_UTILITY_HPP
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "generic/geometry/Utility.hpp"
#include "extension/ECadExtension.h"
#include "TestData.hpp"
#include "Interface.h"
#include "EDataMgr.h"
using namespace boost::unit_test;
using namespace ecad;

void t_flatten_utility()
{
    std::string err;
    std::string name = "ringo";
    std::string gds = ecad_test::GetTestDataPath() + "/extension/gdsii/ringo.gds";
    auto database = ext::CreateDatabaseFromGds(name, gds, &err);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(database != nullptr);

    std::vector<Ptr<ICell> > topCells;
    BOOST_CHECK(database->GetTopCells(topCells));
    BOOST_CHECK(topCells.size() == 1);

    std::vector<Ptr<ICell> > cells;
    database->GetCircuitCells(cells);
    BOOST_CHECK(cells.size() == 9);

    auto topCell = topCells.front();
    auto res = database->Flatten(topCell);
    BOOST_CHECK(res);

    EDataMgr::Instance().ShutDown();
}

void t_connectivity_extraction()
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
    layout->ConnectivityExtraction();

    EDataMgr::Instance().ShutDown();
}

test_suite * create_ecad_utility_test_suite()
{
    test_suite * utility_suite = BOOST_TEST_SUITE("s_utility_test");
    //
    utility_suite->add(BOOST_TEST_CASE(&t_flatten_utility));
    utility_suite->add(BOOST_TEST_CASE(&t_connectivity_extraction));
    //
    return utility_suite;
}
#endif//ECAD_TEST_UTILITY_HPP
