#ifndef ECAD_TEST_SOLVER_HPP
#define ECAD_TEST_SOLVER_HPP
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "generic/tools/FileSystem.hpp"
#include "solvers/EThermalNetworkSolver.h"
#include "models/thermal/io/EThermalModelIO.h"
#include "TestData.hpp"
using namespace boost::unit_test;
using namespace ecad;
using namespace ecad::esolver;
using namespace ecad::emodel::etherm;
void t_grid_thermal_model_solver_test()
{
    std::string err;
    std::string ctm = ecad_test::GetTestDataPath() + "/extension/ctm/rhsc_ctm5.tar.gz";
    std::string ctmFolder = ecad_test::GetTestDataPath() + "/extension/ctm/rhsc_ctm5";
    auto model = io::makeGridThermalModelFromCTMv1File(ctm, &err);
    BOOST_CHECK(generic::filesystem::PathExists(ctmFolder));
    generic::filesystem::RemoveDir(ctmFolder);
    BOOST_CHECK(model == nullptr);
}

test_suite * create_ecad_solver_test_suite()
{
    test_suite * solver_suite = BOOST_TEST_SUITE("s_solver_test");
    //
    solver_suite->add(BOOST_TEST_CASE(&t_grid_thermal_model_solver_test));
    //
    return solver_suite;
}
#endif//ECAD_TEST_SOLVER_HPP
