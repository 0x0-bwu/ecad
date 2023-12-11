#pragma once
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "generic/tools/FileSystem.hpp"
#include "simulation/EThermalNetworkExtraction.h"
#include "extension/ECadExtension.h"
#include "TestData.hpp"
#include "EDataMgr.h"
using namespace boost::unit_test;
using namespace ecad;

void t_thermal_network_extraction()
{
    using namespace generic::fs;

    std::string err;
    std::string qcomXfl = ecad_test::GetTestDataPath() + "/xfl/qcom.xfl";
    auto qcom = ext::CreateDatabaseFromXfl("qcom", qcomXfl, &err);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(qcom != nullptr);

    std::vector<Ptr<ICell> > cells;
    qcom->GetCircuitCells(cells);
    BOOST_CHECK(cells.size() == 1);
    
    auto layout = cells.front()->GetLayoutView();
    
    EThermalNetworkExtractionSettings settings;
    settings.outDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    settings.dumpHotmaps = true;
    settings.dumpDensityFile = true;
    settings.dumpTemperatureFile = true;
    settings.grid = {25, 25};
    settings.mergeGeomBeforeMetalMapping = true;

    esim::EThermalNetworkExtraction ne;
    ne.SetExtractionSettings(settings);
    BOOST_CHECK(ne.GenerateGridThermalModel(layout));

    EDataMgr::Instance().ShutDown();
}

test_suite * create_ecad_simulation_test_suite()
{
    test_suite * simulation_suite = BOOST_TEST_SUITE("s_simulation_test");
    //
    simulation_suite->add(BOOST_TEST_CASE(&t_thermal_network_extraction));
    //
    return simulation_suite;
}