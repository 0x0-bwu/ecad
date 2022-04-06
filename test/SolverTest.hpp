#ifndef ECAD_TEST_SOLVER_HPP
#define ECAD_TEST_SOLVER_HPP
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "generic/tools/FileSystem.hpp"
#include "solvers/EThermalNetworkSolver.h"
#include "models/thermal/io/EThermalModelIO.h"
#include "models/thermal/utilities/EThermalModelReduction.h"
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
    auto model = io::makeGridThermalModelFromCTMv1File(ctm, 0, &err);
    BOOST_CHECK(generic::filesystem::PathExists(ctmFolder));
    generic::filesystem::RemoveDir(ctmFolder);
    BOOST_CHECK(model);

    // etherm::utils::EGridThermalModelReduction r(*model);
    // BOOST_CHECK(r.Reduce());
    
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
    // EGridThermalNetworkDirectSolver solver(*model);
    // BOOST_CHECK(solver.Solve(iniT, results));

    EGridThermalNetworkReductionSolver solver(*model, 2);
    BOOST_CHECK(solver.Solve(iniT, results));
   
    auto htMap = std::unique_ptr<ELayoutMetalFraction>(new ELayoutMetalFraction);
    for(size_t z = 0; z < size.z; ++z)
        htMap->push_back(std::make_shared<ELayerMetalFraction>(size.x, size.y));

    for(size_t i = 0; i < results.size(); ++i){
        auto gridIndex = model->GetGridIndex(i);
        auto lyrHtMap = htMap->at(gridIndex.z);
        (*lyrHtMap)(gridIndex.x, gridIndex.y) = results[i];
    }

    std::string outDir = ecad_test::GetTestDataPath() + "/simulation/ctm/rhsc_ctm5";
    // auto htMapInfo = *mfInfo;
    // if(!m_settings.outDir.empty() && m_settings.dumpTemperatureFile) {
    //     auto tFile = outDir + GENERIC_FOLDER_SEPS + "temperature.txt";
    //     WriteThermalProfile(htMapInfo, *htMap, tFile);
    // }

#ifdef BOOST_GIL_IO_PNG_SUPPORT

    using ValueType = typename ELayerMetalFraction::ResultType;
    if(!outDir.empty()) { 
        ValueType min = std::numeric_limits<ValueType>::max(), max = -min;
        for(auto index = 0; index < size.z; ++index){
            auto lyr = htMap->at(index);
            min = std::min(min, lyr->MaxOccupancy(std::less<ValueType>()));
            max = std::max(max, lyr->MaxOccupancy(std::greater<ValueType>()));
        }
        auto range = max - min;
        std::cout << "min: " << min << ", max: " << max << std::endl;
        //min: 175.204, max: 520.823

        for(auto index = 0; index < size.z; ++index){
            auto lyr = htMap->at(index);
            min = lyr->MaxOccupancy(std::less<ValueType>());
            max = lyr->MaxOccupancy(std::greater<ValueType>());
            range = max - min;
            auto rgbaFunc = [&min, &range](ValueType d) {
                int r, g, b, a = 255;
                generic::color::RGBFromScalar((d - min) / range, r, g, b);
                return std::make_tuple(r, g, b, a);
            };
            std::cout << "layer: " << index + 1 << ", min: " << min << ", max: " << max << std::endl;   
            std::string filepng = outDir + GENERIC_FOLDER_SEPS + std::to_string(index) + ".png";
            lyr->WriteImgProfile(filepng, rgbaFunc);
        }
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
#endif//ECAD_TEST_SOLVER_HPP
