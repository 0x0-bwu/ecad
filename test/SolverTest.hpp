#pragma once
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "generic/tools/Format.hpp"
#include "generic/tools/FileSystem.hpp"
#include "solvers/EThermalNetworkSolver.h"
#include "models/thermal/io/EThermalModelIO.h"
#include "models/thermal/io/EGridThermalModelIO.h"
#include "models/thermal/utilities/EThermalModelReduction.h"
#include "TestData.hpp"
using namespace boost::unit_test;
using namespace ecad;
using namespace ecad::esolver;
using namespace ecad::emodel::etherm;
void t_grid_thermal_model_solver_test()
{
    std::string err;
    std::string ctm = ecad_test::GetTestDataPath() + "/ctm/rhsc_ctm5.tar.gz";
    std::string ctmFolder = ecad_test::GetTestDataPath() + "/ctm/rhsc_ctm5";
    auto model = io::makeGridThermalModelFromCTMv1File(ctm, 0, &err);
    BOOST_CHECK(generic::filesystem::PathExists(ctmFolder));
    generic::filesystem::RemoveDir(ctmFolder);
    BOOST_CHECK(model);
    
    auto size = model->ModelSize();
    std::cout << "size: (" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
    std::cout << "total nodes: " << model->TotalGrids() << std::endl;

    //htc
    ESimVal iniT = 25.0;
    auto bcModel = std::make_shared<EGridBCModel>(ESize2D(size.x, size.y));
    bcModel->AddSample(iniT, EGridData(size.x, size.y, 200000));

    model->SetTopBotBCModel(bcModel, bcModel);
    model->SetTopBotBCType(EGridThermalModel::BCType::HTC, EGridThermalModel::BCType::HTC);

    std::vector<ESimVal> results;
    // etherm::utils::EGridThermalModelReduction r(*model);
    // BOOST_CHECK(r.Reduce());

    EGridThermalNetworkStaticSolver solver(*model);
    BOOST_CHECK(solver.Solve(iniT, results));

    auto resModel = std::make_unique<EGridThermalModel>(*model);
    auto resSize = resModel->GridSize();
    auto & layer = resModel->GetLayers();
    for(size_t z = 0; z < layer.size(); ++z) {
        auto htMap = std::make_shared<ELayerMetalFraction>(resSize.x, resSize.y);
        for(size_t x = 0; x < resSize.x; ++x) {
            for(size_t y = 0; y < resSize.y; ++y) {
                (*htMap)(x, y) = results[resModel->GetFlattenIndex(ESize3D(x, y, z))];
            }
        }
        layer[z].SetMetalFraction(htMap);
    }

    std::string outDir = ecad_test::GetTestDataPath() + "/simulation/ctm/rhsc_ctm5";
    std::string txtProfile = outDir + "/ThermalProfile_rhsc_ctm5.txt";
    io::GenerateTxtProfile(*resModel, txtProfile);

#ifdef BOOST_GIL_IO_PNG_SUPPORT

    using ValueType = typename ELayerMetalFraction::ResultType;
    ValueType min = std::numeric_limits<ValueType>::max(), max = -min;
    for(const auto & layer :  resModel->GetLayers()) {
        auto htMap = layer.GetMetalFraction();
        if(nullptr == htMap) continue;
        min = std::min(min, htMap->MaxOccupancy(std::less<ValueType>()));
        max = std::max(max, htMap->MaxOccupancy(std::greater<ValueType>()));
    }
    auto range = max - min;
    std::cout << "min: " << min << ", max: " << max << std::endl;
    //min: 175.204, max: 520.823

    size_t i = 0;
    for(const auto & layer :  resModel->GetLayers()) {
        auto htMap = layer.GetMetalFraction();
        if(nullptr == htMap) continue;
        min = std::min(min, htMap->MaxOccupancy(std::less<ValueType>()));
        max = std::max(max, htMap->MaxOccupancy(std::greater<ValueType>()));
        range = max - min;
        auto rgbaFunc = [&min, &range](ValueType d) {
            int r, g, b, a = 255;
            generic::color::RGBFromScalar((d - min) / range, r, g, b);
            return std::make_tuple(r, g, b, a);
        };
        std::cout << generic::fmt::Fmt2Str("%1%: [%2%, %3%]", layer.GetName(), min, max) << std::endl;   
        std::string filepng = outDir + GENERIC_FOLDER_SEPS + "layer_" + std::to_string(++i) + ".png";
        htMap->WriteImgProfile(filepng, rgbaFunc);
    }
#endif//BOOST_GIL_IO_PNG_SUPPORT
}

test_suite * create_ecad_solver_test_suite()
{
    test_suite * solver_suite = BOOST_TEST_SUITE("s_solver_test");
    //
    solver_suite->add(BOOST_TEST_CASE(&t_grid_thermal_model_solver_test));
    //
    return solver_suite;
}