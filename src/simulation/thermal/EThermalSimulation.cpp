#include "EThermalSimulation.h"
#include "model/thermal/EStackupPrismThermalModel.h"
#include "model/thermal/io/EPrismThermalModelIO.h"
#include "solver/thermal/EThermalNetworkSolver.h"
#include "model/thermal/EPrismThermalModel.h"
#include "model/thermal/EGridThermalModel.h"
#include "model/thermal/EThermalModel.h"
#include "utility/EMetalFractionMapping.h"
#include "utility/ELayoutRetriever.h"

#include "generic/tools/FileSystem.hpp"

#include "Mesher2D.h"
#include "interface/Interface.h"

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
        case EModelType::ThermalPrism : {
            if (auto prism = dynamic_cast<CPtr<EPrismThermalModel> >(model); prism)
                return EPrismThermalSimulator(prism, m_setup).Run(minT, maxT);
        }
        case EModelType::ThermalStackupPrism : {
            if (auto prism = dynamic_cast<CPtr<EStackupPrismThermalModel> >(model); prism)
                return EStackupPrismThermalSimulator(prism, m_setup).Run(minT, maxT);
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

ECAD_API EPrismThermalSimulator::EPrismThermalSimulator(CPtr<EPrismThermalModel> model, const EThermalSimulationSetup & setup)
 : EThermalSimulator(model, setup)
{
}

ECAD_API bool EPrismThermalSimulator::RunStaticSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("prism thermal static simulation")
    auto model = dynamic_cast<CPtr<EPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalStaticSimulationSetup> >(&m_setup);
    if (nullptr == model ||  nullptr == setup) return false;

    std::vector<EFloat> results;
    EPrismThermalNetworkStaticSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
}

ECAD_API bool EPrismThermalSimulator::RunTransientSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("prism thermal transient simulation")
    auto model = dynamic_cast<CPtr<EPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return false;

    std::vector<EFloat> results;
    EPrismThermalNetworkTransientSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
}

ECAD_API EStackupPrismThermalSimulator::EStackupPrismThermalSimulator(CPtr<EStackupPrismThermalModel> model, const EThermalSimulationSetup & setup)
 : EThermalSimulator(model, setup)
{
}

ECAD_API bool EStackupPrismThermalSimulator::RunStaticSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("stackup prism thermal static simulation")
    auto model = dynamic_cast<CPtr<EStackupPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalStaticSimulationSetup> >(&m_setup);
    if (nullptr == model ||  nullptr == setup) return false;

    std::vector<EFloat> results;
    EStackupPrismThermalNetworkStaticSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
}

ECAD_API bool EStackupPrismThermalSimulator::RunTransientSimulation(EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("stackup prism thermal transient simulation")
    auto model = dynamic_cast<CPtr<EStackupPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return false;

    std::vector<EFloat> results;
    EStackupPrismThermalNetworkTransientSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
}

} // namespace ecad::simulation