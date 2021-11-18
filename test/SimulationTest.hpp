#ifndef ECAD_TEST_SIMULATION_HPP
#define ECAD_TEST_SIMULATION_HPP
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "generic/tools/FileSystem.hpp"
#include "generic/geometry/Utility.hpp"
#include "extension/ECadExtension.h"
#include "simulation/EThermalNetworkExtraction.h"
#include "Interface.h"
#include "EDataMgr.h"
using namespace boost::unit_test;
using namespace ecad;

extern const std::string testDataPath;

void t_metal_fraction_mapping()
{
    using namespace generic::filesystem;

    std::string err;
    std::string name = "4004";
    std::string gds = testDataPath + "/extension/gdsii/" + name + ".gds";
    auto database = ext::CreateDatabaseFromGds(name, gds, &err);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(database != nullptr);

    std::vector<Ptr<ICell> > topCells;
    BOOST_CHECK(database->GetTopCells(topCells));
    BOOST_CHECK(topCells.size() >= 1);
    
    auto topCell = topCells.front();
    auto flattened = topCell->GetFlattenedLayoutView();

    esim::EMetalFractionMappingSettings settings;
    settings.threads = 16;
    settings.resolution = 10;//um
    settings.coordUnits = database->GetCoordUnits();
    settings.outFile = testDataPath + "/simulation/result.mf";

    BOOST_CHECK(flattened->GenerateMetalFractionMapping(settings));

    EDataMgr::Instance().ShutDown();
}

void t_metal_fraction_mapping_select_nets()
{
    using namespace generic::filesystem;

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
    layout->ConnectivityExtraction();

    esim::EMetalFractionMappingSettings settings;
    settings.threads = 16;
    settings.resolution = 1000;//um
    settings.coordUnits = database->GetCoordUnits();
    settings.outFile = testDataPath + "/simulation/result.mf";
    settings.selectNets.insert(1);

    BOOST_CHECK(layout->GenerateMetalFractionMapping(settings));

    EDataMgr::Instance().ShutDown();
}

void t_thermal_network_extraction()
{
    using namespace generic::filesystem;

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

    esim::EThermalNetworkExtraction ne;
    BOOST_CHECK(ne.GenerateThermalNetwork(layout));

    EDataMgr::Instance().ShutDown();
}

test_suite * create_ecad_simulation_test_suite()
{
    test_suite * simulation_suite = BOOST_TEST_SUITE("s_simulation_test");
    //
    simulation_suite->add(BOOST_TEST_CASE(&t_metal_fraction_mapping));
    simulation_suite->add(BOOST_TEST_CASE(&t_metal_fraction_mapping_select_nets));
    //simulation_suite->add(BOOST_TEST_CASE(&t_thermal_network_extraction));
    //
    return simulation_suite;
}
#endif//ECAD_TEST_SIMULATION_HPP
