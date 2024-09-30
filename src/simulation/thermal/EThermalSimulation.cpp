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
#include "interface/Interface.h"

namespace ecad::simulation {

using namespace ecad::model;
using namespace ecad::utils;
using namespace ecad::solver;

ECAD_API EThermalSimulation::EThermalSimulation(CPtr<IModel> model, const EThermalSimulationSetup & setup)
 : m_model(model), m_setup(setup)
{
}

ECAD_API EPair<EFloat, EFloat> EThermalSimulation::RunStaticSimulation(std::vector<EFloat> & temperatures) const
{
    ECAD_TRACE("run static thermal simulation at %1%", m_setup.workDir);
    if (nullptr == m_model) {
        ECAD_ASSERT(false)
        return {invalidFloat, invalidFloat};
    }
    if (not generic::fs::CreateDir(m_setup.workDir)) {
        ThrowException("failed to create folder: " + m_setup.workDir);
        return {invalidFloat, invalidFloat};
    }
    auto modelType = m_model->GetModelType();
    switch (modelType) {
    case EModelType::ThermalGrid : {
        if (auto grid = dynamic_cast<CPtr<EGridThermalModel> >(m_model); grid)
            return EGridThermalSimulator(grid, m_setup).RunStaticSimulation(temperatures);
    }
    case EModelType::ThermalPrism : {
        if (auto prism = dynamic_cast<CPtr<EPrismThermalModel> >(m_model); prism)
            return EPrismThermalSimulator(prism, m_setup).RunStaticSimulation(temperatures);
    }
    case EModelType::ThermalStackupPrism : {
        if (auto prism = dynamic_cast<CPtr<EStackupPrismThermalModel> >(m_model); prism)
            return EStackupPrismThermalSimulator(prism, m_setup).RunStaticSimulation(temperatures);
    }
    default :
        ECAD_ASSERT(false)
        return {invalidFloat, invalidFloat};
    }
    return {invalidFloat, invalidFloat};
}

ECAD_API EPair<EFloat, EFloat> EThermalSimulation::RunTransientSimulation(const EThermalTransientExcitation & excitation) const
{
    if (nullptr == m_model){
        ECAD_ASSERT(false)
        return {invalidFloat, invalidFloat};
    }
    if (not generic::fs::CreateDir(m_setup.workDir)) {
        ThrowException("failed to create folder: " + m_setup.workDir);
        return {invalidFloat, invalidFloat};
    }
    auto modelType = m_model->GetModelType();
    switch (modelType) {
    case EModelType::ThermalGrid : {
        if (auto grid = dynamic_cast<CPtr<EGridThermalModel> >(m_model); grid)
            return EGridThermalSimulator(grid, m_setup).RunTransientSimulation(excitation);
    }
    case EModelType::ThermalPrism : {
        if (auto prism = dynamic_cast<CPtr<EPrismThermalModel> >(m_model); prism)
            return EPrismThermalSimulator(prism, m_setup).RunTransientSimulation(excitation);
    }
    case EModelType::ThermalStackupPrism : {
        if (auto prism = dynamic_cast<CPtr<EStackupPrismThermalModel> >(m_model); prism)
            return EStackupPrismThermalSimulator(prism, m_setup).RunTransientSimulation(excitation);
    }
    default :
        ECAD_ASSERT(false)
        return {invalidFloat, invalidFloat};
    }
    return {invalidFloat, invalidFloat};;
}

ECAD_API EThermalSimulator::EThermalSimulator(CPtr<IModel> model, const EThermalSimulationSetup & setup)
 : m_model(model), m_setup(setup)
{
}

ECAD_API EGridThermalSimulator::EGridThermalSimulator(CPtr<EGridThermalModel> model, const EThermalSimulationSetup & setup)
 : EThermalSimulator(model, setup)
{
}

ECAD_API EPair<EFloat, EFloat> EGridThermalSimulator::RunStaticSimulation(std::vector<EFloat> & temperatures) const
{
    ECAD_EFFICIENCY_TRACK("grid thermal static simulation")
    auto model = dynamic_cast<CPtr<EGridThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalStaticSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return {invalidFloat, invalidFloat};

    EGridThermalNetworkStaticSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(temperatures);
}

ECAD_API EPair<EFloat, EFloat> EGridThermalSimulator::RunTransientSimulation(const EThermalTransientExcitation & excitation) const
{
    ECAD_EFFICIENCY_TRACK("grid thermal transient simulation")
    auto model = dynamic_cast<CPtr<EGridThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return {invalidFloat, invalidFloat};

    EGridThermalNetworkTransientSolver solver(*model, excitation);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve();
}

ECAD_API EPrismThermalSimulator::EPrismThermalSimulator(CPtr<EPrismThermalModel> model, const EThermalSimulationSetup & setup)
 : EThermalSimulator(model, setup)
{
}

ECAD_API EPair<EFloat, EFloat> EPrismThermalSimulator::RunStaticSimulation(std::vector<EFloat> & temperatures) const
{
    ECAD_EFFICIENCY_TRACK("prism thermal static simulation")
    auto model = dynamic_cast<CPtr<EPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalStaticSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return {invalidFloat, invalidFloat};

    EPrismThermalNetworkStaticSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(temperatures);
}

ECAD_API EPair<EFloat, EFloat> EPrismThermalSimulator::RunTransientSimulation(const EThermalTransientExcitation & excitation) const
{
    ECAD_EFFICIENCY_TRACK("prism thermal transient simulation")
    auto model = dynamic_cast<CPtr<EPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return {invalidFloat, invalidFloat};

    EPrismThermalNetworkTransientSolver solver(*model, excitation);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);

    return solver.Solve();
}

ECAD_API EStackupPrismThermalSimulator::EStackupPrismThermalSimulator(CPtr<EStackupPrismThermalModel> model, const EThermalSimulationSetup & setup)
 : EThermalSimulator(model, setup)
{
}

ECAD_API EPair<EFloat, EFloat> EStackupPrismThermalSimulator::RunStaticSimulation(std::vector<EFloat> & temperatures) const
{
    ECAD_EFFICIENCY_TRACK("stackup prism thermal static simulation")
    auto model = dynamic_cast<CPtr<EStackupPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalStaticSimulationSetup> >(&m_setup);
    if (nullptr == model ||  nullptr == setup) return {invalidFloat, invalidFloat};

    EStackupPrismThermalNetworkStaticSolver solver(*model);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    return solver.Solve(temperatures);
}

ECAD_API EPair<EFloat, EFloat> EStackupPrismThermalSimulator::RunTransientSimulation(const EThermalTransientExcitation & excitation) const
{
    ECAD_EFFICIENCY_TRACK("stackup prism thermal transient simulation")
    auto model = dynamic_cast<CPtr<EStackupPrismThermalModel> >(m_model);
    auto setup = dynamic_cast<CPtr<EThermalTransientSimulationSetup> >(&m_setup);
    if (nullptr == model || nullptr == setup) return {invalidFloat, invalidFloat};

    EStackupPrismThermalNetworkTransientSolver solver(*model, excitation);
    solver.settings.workDir = setup->workDir;
    solver.settings = setup->settings;
    model->SearchElementIndices(setup->monitors, solver.settings.probs);
    
    return solver.Solve();
}

} // namespace ecad::simulation