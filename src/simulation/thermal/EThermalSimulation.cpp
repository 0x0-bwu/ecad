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

ECAD_API bool EThermalSimulation::Run(CPtr<IModel> model, EFloat & minT, EFloat & maxT) const
{
    generic::fs::CreateDir(m_setup.workDir);
    auto modelType = model->GetModelType();
    switch (modelType)
    {
        case EModelType::ThermalGrid : {
            if (auto grid = dynamic_cast<CPtr<EGridThermalModel> >(model); grid)
                return EGridThermalSimulator(grid, m_setup).Run(minT, maxT);
        }
        case EModelType::ThermalPrisma : {
            if (auto prisma = dynamic_cast<CPtr<EPrismaThermalModel> >(model); prisma)
                return EPrismaThermalSimulator(prisma, m_setup).Run(minT, maxT);
            break;
        }
        default :
            ECAD_ASSERT(false)
            return false;
    }
    return false;
}

ECAD_API EThermalSimulator::EThermalSimulator(CPtr<IModel> model, const EThermalSimulationSetup & setup)
 : m_model(model), m_setup(setup)
{
}

ECAD_API bool EThermalSimulator::Run(EFloat & minT, EFloat & maxT) const
{
    if (auto * ss = dynamic_cast<CPtr<EThermalStaticSimulationSetup> >(&m_setup); ss)
        return RunStaticSimulation(minT, maxT);
    else if(auto * ts = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup); ts)
        return RunTransientSimulation(minT, maxT);
    ECAD_ASSERT(false)
    return false;
}

ECAD_API EGridThermalSimulator::EGridThermalSimulator(CPtr<EGridThermalModel> model, const EThermalSimulationSetup & setup)
 : EThermalSimulator(model, setup)
{
}

ECAD_API bool EGridThermalSimulator::RunStaticSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("grid thermal static simulation")
    auto model = dynamic_cast<CPtr<EGridThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalStaticSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return false;

    std::vector<EFloat> results;
    EGridThermalNetworkStaticSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings.iniT = setup->GetInitTemperature();
    solver.settings = setup->settings;
    if (not solver.Solve(minT, maxT)) return false;
    
    auto modelSize = model->ModelSize();
    auto htMap = std::unique_ptr<ELayoutMetalFraction>(new ELayoutMetalFraction);
    for (size_t z = 0; z < modelSize.z; ++z)
        htMap->push_back(std::make_shared<ELayerMetalFraction>(modelSize.x, modelSize.y));

    for (size_t i = 0; i < results.size(); ++i){
        auto gridIndex = model->GetGridIndex(i);
        auto lyrHtMap = htMap->at(gridIndex.z);
        (*lyrHtMap)(gridIndex.x, gridIndex.y) = results[i];
    }

    using ValueType = typename ELayerMetalFraction::ResultType;
    if (not setup->workDir.empty() && setup->settings.dumpHotmaps) {        
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
            std::string filepng = setup->workDir + ECAD_SEPS + std::to_string(index) + ".png";
            lyr->WriteImgProfile(filepng, rgbaFunc);
        }
    }
    return true;
}

ECAD_API bool EGridThermalSimulator::RunTransientSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("grid thermal transient simulation")
    auto model = dynamic_cast<CPtr<EGridThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return false;

    std::vector<EFloat> results;
    EGridThermalNetworkTransientSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    if (not solver.Solve(minT, maxT)) return false;
    return true;
}

ECAD_API EPrismaThermalSimulator::EPrismaThermalSimulator(CPtr<EPrismaThermalModel> model, const EThermalSimulationSetup & setup)
 : EThermalSimulator(model, setup)
{
}

ECAD_API bool EPrismaThermalSimulator::RunStaticSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("prisma thermal static simulation")
    auto model = dynamic_cast<CPtr<EPrismaThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalStaticSimulationSetup> >(&m_setup);
    if (nullptr == model ||  nullptr == setup) return false;

    std::vector<EFloat> results;
    EPrismaThermalNetworkStaticSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    return solver.Solve(minT, maxT);
}

ECAD_API bool EPrismaThermalSimulator::RunTransientSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("prisma thermal transient simulation")
    auto model = dynamic_cast<CPtr<EPrismaThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return false;

    std::vector<EFloat> results;
    EPrismaThermalNetworkTransientSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    if (not solver.Solve(minT, maxT)) return false;
    return true;
}

} // namespace ecad::simulation