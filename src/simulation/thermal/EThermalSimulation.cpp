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

ECAD_API EThermalSimulation::EThermalSimulation(CPtr<IModel> model, const EThermalSimulationSetup & setup)
 : m_model(model), m_setup(setup)
{
}

ECAD_API bool EThermalSimulation::RunStaticSimulation(EFloat & minT, EFloat & maxT) const
{
    if (nullptr == m_model) return false;
    generic::fs::CreateDir(m_setup.workDir);
    auto modelType = m_model->GetModelType();
    switch (modelType)
    {
        case EModelType::ThermalGrid : {
            if (auto grid = dynamic_cast<CPtr<EGridThermalModel> >(m_model); grid)
                return EGridThermalSimulator(grid, m_setup).RunStaticSimulation(minT, maxT);
        }
        case EModelType::ThermalPrism : {
            if (auto prism = dynamic_cast<CPtr<EPrismThermalModel> >(m_model); prism)
                return EPrismThermalSimulator(prism, m_setup).RunStaticSimulation(minT, maxT);
        }
        case EModelType::ThermalStackupPrism : {
            if (auto prism = dynamic_cast<CPtr<EStackupPrismThermalModel> >(m_model); prism)
                return EStackupPrismThermalSimulator(prism, m_setup).RunStaticSimulation(minT, maxT);
        }
        default :
            ECAD_ASSERT(false)
            return false;
    }
    return false;
}

ECAD_API bool EThermalSimulation::RunTransientSimulation(const EThermalTransientExcitation & excitation, EFloat & minT, EFloat & maxT) const
{
    if (nullptr == m_model) return false;
    generic::fs::CreateDir(m_setup.workDir);
    auto modelType = m_model->GetModelType();
    switch (modelType)
    {
        case EModelType::ThermalGrid : {
            if (auto grid = dynamic_cast<CPtr<EGridThermalModel> >(m_model); grid)
                return EGridThermalSimulator(grid, m_setup).RunTransientSimulation(excitation, minT, maxT);
        }
        case EModelType::ThermalPrism : {
            if (auto prism = dynamic_cast<CPtr<EPrismThermalModel> >(m_model); prism)
                return EPrismThermalSimulator(prism, m_setup).RunTransientSimulation(excitation, minT, maxT);
        }
        case EModelType::ThermalStackupPrism : {
            if (auto prism = dynamic_cast<CPtr<EStackupPrismThermalModel> >(m_model); prism)
                return EStackupPrismThermalSimulator(prism, m_setup).RunTransientSimulation(excitation, minT, maxT);
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

ECAD_API bool EGridThermalSimulator::RunTransientSimulation(const EThermalTransientExcitation & excitation, EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("grid thermal transient simulation")
    auto model = dynamic_cast<CPtr<EGridThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return false;

    std::vector<EFloat> results;
    EGridThermalNetworkTransientSolver solver(*model, excitation);
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

ECAD_API bool EPrismThermalSimulator::RunTransientSimulation(const EThermalTransientExcitation & excitation, EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("prism thermal transient simulation")
    auto model = dynamic_cast<CPtr<EPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return false;

    std::vector<EFloat> results;
    EPrismThermalNetworkTransientSolver solver(*model, excitation);
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

ECAD_API bool EStackupPrismThermalSimulator::RunTransientSimulation(const EThermalTransientExcitation & excitation, EFloat & minT, EFloat & maxT) const
{
    ECAD_EFFICIENCY_TRACK("stackup prism thermal transient simulation")
    auto model = dynamic_cast<CPtr<EStackupPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return false;

    std::vector<EFloat> results;
    EStackupPrismThermalNetworkTransientSolver solver(*model, excitation);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(minT, maxT);
}

} // namespace ecad::simulation