#include "EThermalSimulation.h"
#include "models/thermal/EStackupPrismaThermalModel.h"
#include "models/thermal/io/EPrismaThermalModelIO.h"
#include "solvers/thermal/EThermalNetworkSolver.h"
#include "models/thermal/EPrismaThermalModel.h"
#include "models/thermal/EGridThermalModel.h"
#include "models/thermal/EThermalModel.h"
#include "utils/EMetalFractionMapping.h"
#include "utils/ELayoutRetriever.h"

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
        }
        case EModelType::ThermalStackupPrisma : {
            if (auto prisma = dynamic_cast<CPtr<EStackupPrismaThermalModel> >(model); prisma)
                return EStackupPrismaThermalSimulator(prisma, m_setup).Run(minT, maxT);
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
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
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
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
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
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
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
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
}

ECAD_API EStackupPrismaThermalSimulator::EStackupPrismaThermalSimulator(CPtr<EStackupPrismaThermalModel> model, const EThermalSimulationSetup & setup)
 : EThermalSimulator(model, setup)
{
}

ECAD_API bool EStackupPrismaThermalSimulator::RunStaticSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("stackup prisma thermal static simulation")
    auto model = dynamic_cast<CPtr<EStackupPrismaThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalStaticSimulationSetup> >(&m_setup);
    if (nullptr == model ||  nullptr == setup) return false;

    std::vector<EFloat> results;
    EStackupPrismaThermalNetworkStaticSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
}

ECAD_API bool EStackupPrismaThermalSimulator::RunTransientSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("stackup prisma thermal transient simulation")
    auto model = dynamic_cast<CPtr<EStackupPrismaThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return false;

    std::vector<EFloat> results;
    EStackupPrismaThermalNetworkTransientSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
}

} // namespace ecad::simulation