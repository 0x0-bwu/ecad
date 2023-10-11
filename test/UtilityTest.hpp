#pragma once
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "generic/geometry/Utility.hpp"
#include "extension/ECadExtension.h"
#include "TestData.hpp"
#include "EDataMgr.h"
using namespace boost::unit_test;
using namespace ecad;

void t_flatten_utility()
{
    std::string err;
    std::string name = "ringo";
    std::string gds = ecad_test::GetTestDataPath() + "/gdsii/ringo.gds";
    auto database = ext::CreateDatabaseFromGds(name, gds, std::string{}, &err);
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
    std::string dmc = ecad_test::GetTestDataPath() + "/dmcdom/import.dmc";
    std::string dom = ecad_test::GetTestDataPath() + "/dmcdom/import.dom";
    auto database = ext::CreateDatabaseFromDomDmc("test_dmcdom", dmc, dom);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(database != nullptr);

    std::vector<Ptr<ICell> > cells;
    database->GetCircuitCells(cells);
    BOOST_CHECK(cells.size() == 1);
    
    auto layout = cells.front()->GetLayoutView();
    layout->ExtractConnectivity();

    EDataMgr::Instance().ShutDown();
}

void t_layout_polygon_merge()
{
    std::string err;
    std::string qcomXfl = ecad_test::GetTestDataPath() + "/xfl/pop.xfl";
    auto qcom = ext::CreateDatabaseFromXfl("qcom", qcomXfl, &err);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(qcom != nullptr);

    std::vector<Ptr<ICell> > cells;
    qcom->GetCircuitCells(cells);
    BOOST_CHECK(cells.size() == 1);
    
    auto layout = cells.front()->GetLayoutView();

    ELayoutPolygonMergeSettings settings;
    settings.threads = 1;
    settings.outFile = ecad_test::GetTestDataPath() + "/simulation/qcom";
    // settings.selectNets = { ENetId(74) };
    BOOST_CHECK(layout->MergeLayerPolygons(settings));

    EDataMgr::Instance().ShutDown();
}

void t_metal_fraction_mapping()
{
    using namespace generic::filesystem;

    std::string err;
    std::string name = "4004";
    std::string gds = ecad_test::GetTestDataPath() + "/gdsii/" + name + ".gds";
    auto database = ext::CreateDatabaseFromGds(name, gds, std::string{}, &err);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(database != nullptr);

    std::vector<Ptr<ICell> > topCells;
    BOOST_CHECK(database->GetTopCells(topCells));
    BOOST_CHECK(topCells.size() >= 1);
    
    auto topCell = topCells.front();
    auto flattened = topCell->GetFlattenedLayoutView();

    EMetalFractionMappingSettings settings;
    settings.threads = 16;
    settings.grid = {50, 50};
    settings.outFile = ecad_test::GetTestDataPath() + "/simulation/result.mf";

    BOOST_CHECK(flattened->GenerateMetalFractionMapping(settings));

    EDataMgr::Instance().ShutDown();
}

void t_metal_fraction_mapping_select_nets()
{
    using namespace generic::filesystem;

    std::string err;
    std::string dmc = ecad_test::GetTestDataPath() + "/dmcdom/import.dmc";
    std::string dom = ecad_test::GetTestDataPath() + "/dmcdom/import.dom";
    auto database = ext::CreateDatabaseFromDomDmc("test_dmcdom", dmc, dom);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(database != nullptr);

    std::vector<Ptr<ICell> > cells;
    database->GetCircuitCells(cells);
    BOOST_CHECK(cells.size() == 1);
    
    auto layout = cells.front()->GetLayoutView();
    layout->ExtractConnectivity();

    EMetalFractionMappingSettings settings;
    settings.threads = 16;
    settings.grid = {50, 50};
    settings.outFile = ecad_test::GetTestDataPath() + "/simulation/result.mf";
    settings.selectNets.insert(static_cast<ENetId>(1));

    BOOST_CHECK(layout->GenerateMetalFractionMapping(settings));

    EDataMgr::Instance().ShutDown();
}

void t_layout_to_ctm()
{
    std::string err;
    std::string popXfl = ecad_test::GetTestDataPath() + "/xfl/pop.xfl";
    auto pop = ext::CreateDatabaseFromXfl("pop", popXfl, &err);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(pop != nullptr);

    std::vector<Ptr<ICell> > cells;
    pop->GetCircuitCells(cells);
    BOOST_CHECK(cells.size() == 1);
    
    auto layout = cells.front()->GetLayoutView();

    ELayout2CtmSettings settings;
    settings.threads = 8;
    settings.dirName = ecad_test::GetTestDataPath() + "/simulation/ctm";
    settings.filename = "pop";
    BOOST_CHECK(layout->GenerateCTMv1File(settings));

    EDataMgr::Instance().ShutDown();
}

test_suite * create_ecad_utility_test_suite()
{
    test_suite * utility_suite = BOOST_TEST_SUITE("s_utility_test");
    //
    utility_suite->add(BOOST_TEST_CASE(&t_flatten_utility));
    utility_suite->add(BOOST_TEST_CASE(&t_connectivity_extraction));
    utility_suite->add(BOOST_TEST_CASE(&t_layout_polygon_merge));
    utility_suite->add(BOOST_TEST_CASE(&t_metal_fraction_mapping));
    utility_suite->add(BOOST_TEST_CASE(&t_metal_fraction_mapping_select_nets));
    utility_suite->add(BOOST_TEST_CASE(&t_layout_to_ctm));
    //
    return utility_suite;
}