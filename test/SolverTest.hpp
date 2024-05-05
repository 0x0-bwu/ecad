#pragma once
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "generic/tools/Format.hpp"
#include "generic/tools/FileSystem.hpp"
#include "solver/thermal/EThermalNetworkSolver.h"
#include "model/thermal/io/EThermalModelIO.h"
#include "model/thermal/io/EGridThermalModelIO.h"
#include "model/thermal/utils/EThermalModelReduction.h"
#include "TestData.hpp"
using namespace boost::unit_test;
using namespace ecad;
using namespace ecad::solver;
using namespace ecad::model;
void t_grid_thermal_model_solver_test()
{
    std::string err;
    EDataMgr::Instance().Init();
    std::string ctm = ecad_test::GetTestDataPath() + "/ctm/rhsc_ctm5.tar.gz";
    std::string ctmFolder = ecad_test::GetTestDataPath() + "/ctm/rhsc_ctm5";
    auto model = io::makeGridThermalModelFromCTMv1File(ctm, 0, &err);
    BOOST_CHECK(generic::fs::PathExists(ctmFolder));
    generic::fs::RemoveDir(ctmFolder);
    BOOST_CHECK(model);
    
    auto size = model->ModelSize();
    ECAD_TRACE("size: (%1%, %2%)", size.x, size.y)
    ECAD_TRACE("total nodes: %1%", model->TotalGrids())

    //htc
    model->SetUniformBC(EOrientation::Top, EThermalBondaryCondition(200000, EThermalBondaryConditionType::HTC));
    model->SetUniformBC(EOrientation::Bot, EThermalBondaryCondition(200000, EThermalBondaryConditionType::HTC));

    std::vector<EFloat> results;
    EGridThermalNetworkStaticSolver solver(*model);
    solver.settings.envTemperature.value = 25;
    solver.settings.iteration = 3;
    EFloat minT, maxT;
    BOOST_CHECK(solver.Solve(minT, maxT));
    ECAD_TRACE("maxT: %1%, minT: %2%", maxT, minT)
    //max: 99.4709, min: 81.9183
}

test_suite * create_ecad_solver_test_suite()
{
    test_suite * solver_suite = BOOST_TEST_SUITE("s_solver_test");
    //
    solver_suite->add(BOOST_TEST_CASE(&t_grid_thermal_model_solver_test));
    //
    return solver_suite;
}