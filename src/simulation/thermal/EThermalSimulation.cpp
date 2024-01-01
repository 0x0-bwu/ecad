#include "EThermalSimulation.h"
#include "models/thermal/io/EPrismaThermalModelIO.h"
#include "models/thermal/EPrismaThermalModel.h"
#include "models/thermal/EGridThermalModel.h"
#include "utilities/EMetalFractionMapping.h"
#include "solvers/thermal/EThermalNetworkSolver.h"
#include "models/thermal/EThermalModel.h"
#include "utilities/ELayoutRetriever.h"
#include "generic/tools/FileSystem.hpp"


#include "Mesher2D.h"
#include "Interface.h"

namespace ecad::simulation {

using namespace ecad::model;
using namespace ecad::utils;
using namespace ecad::solver;

ECAD_API bool EThermalSimulation::Run(CPtr<IModel> model, EFloat & maxT) const
{
    auto modelType = model->GetModelType();
    switch (modelType)
    {
        case EModelType::ThermalGrid : {
            auto gridModel = dynamic_cast<CPtr<EGridThermalModel> >(model);
            if (gridModel) {
                EGridThermalSimulator simulator(gridModel, setup);
                if (EThermalSimuType::Static == setup.simuType)
                    return simulator.RunStaticSimulation(maxT);
                else return simulator.RunTransientSimulation();
            }
            break;
        }
        case EModelType::ThermalPrisma : {
            auto prismaModel = dynamic_cast<CPtr<EPrismaThermalModel> >(model);
            if (prismaModel) {
                EPrismaThermalSimulator simulator(prismaModel, setup);
                if (EThermalSimuType::Static == setup.simuType)
                    return simulator.RunStaticSimulation(maxT);
                else return simulator.RunTransientSimulation();
            }
            break;
        }
        default :
            ECAD_ASSERT(false)
            return false;
    }
    return false;
}

ECAD_API EGridThermalSimulator::EGridThermalSimulator(CPtr<EGridThermalModel> model, const EThermalSimulationSetup & setup)
 : m_model(model)
{
    m_settings.workDir = setup.workDir;
    m_settings.spiceFile = setup.workDir + ECAD_SEPS + "spice.sp";
    m_settings.iniT = setup.environmentTemperature;
}

ECAD_API bool EGridThermalSimulator::RunStaticSimulation(EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("grid thermal static simulation")
    std::vector<EFloat> results;
    EGridThermalNetworkStaticSolver solver(*m_model);
    solver.SetSolveSettings(m_settings);
    if (not solver.Solve(m_settings.iniT, results)) return false;
    
    maxT = *std::max_element(results.begin(), results.end());

    auto modelSize = m_model->ModelSize();
    auto htMap = std::unique_ptr<ELayoutMetalFraction>(new ELayoutMetalFraction);
    for (size_t z = 0; z < modelSize.z; ++z)
        htMap->push_back(std::make_shared<ELayerMetalFraction>(modelSize.x, modelSize.y));

    for (size_t i = 0; i < results.size(); ++i){
        auto gridIndex = m_model->GetGridIndex(i);
        auto lyrHtMap = htMap->at(gridIndex.z);
        (*lyrHtMap)(gridIndex.x, gridIndex.y) = results[i];
    }

    using ValueType = typename ELayerMetalFraction::ResultType;
    if (not m_settings.workDir.empty() && m_settings.dumpHotmaps) {        
        for (size_t index = 0; index < htMap->size(); ++index) {
            auto lyr = htMap->at(index);
            auto min = lyr->MaxOccupancy(std::less<ValueType>());
            auto max = lyr->MaxOccupancy(std::greater<ValueType>());
            auto range = max - min;
            auto rgbaFunc = [&min, &range](ValueType d) {
                int r, g, b, a = 255;
                generic::color::RGBFromScalar((d - min) / range, r, g, b);
                return std::make_tuple(r, g, b, a);
            };
            ECAD_TRACE("layer: %1%, maxT: %2%, minT: %3%", index + 1, max, min)
            std::string filepng = m_settings.workDir + ECAD_SEPS + std::to_string(index) + ".png";
            lyr->WriteImgProfile(filepng, rgbaFunc);
        }
    }
    return true;
}

ECAD_API bool EGridThermalSimulator::RunTransientSimulation() const
{
    ECAD_EFFICIENCY_TRACK("grid thermal transient simulation")
    std::vector<EFloat> results;
    EGridThermalNetworkTransientSolver solver(*m_model);
    solver.SetSolveSettings(m_settings);
    if (not solver.Solve(m_settings.iniT, results)) return false;
    return true;
}

ECAD_API EPrismaThermalSimulator::EPrismaThermalSimulator(CPtr<EPrismaThermalModel> model, const EThermalSimulationSetup & setup)
 : m_model(model)
{
    m_settings.workDir = setup.workDir;
    m_settings.spiceFile = setup.workDir + ECAD_SEPS + "spice.sp";
    m_settings.iniT = setup.environmentTemperature;
}

ECAD_API bool EPrismaThermalSimulator::RunStaticSimulation(EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("prisma thermal static simulation")
    std::vector<EFloat> results;
    EPrismaThermalNetworkStaticSolver solver(*m_model);
    solver.SetSolveSettings(m_settings);
    if (not solver.Solve(m_settings.iniT, results)) return false;

    maxT = *std::max_element(results.begin(), results.end());

    auto hotmapFile = m_settings.workDir + ECAD_SEPS + "hotmap.vtk";
    ECAD_TRACE("dump vtk hotmap: %1%", hotmapFile)
    io::GenerateVTKFile(hotmapFile, *m_model, &results);
    return true;
}

ECAD_API bool EPrismaThermalSimulator::RunTransientSimulation() const
{
    ECAD_EFFICIENCY_TRACK("grid thermal transient simulation")
    return false;
}

} // namespace ecad::simulation