#pragma once
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {
class IModel;
namespace model {
class EGridThermalModel;
class EPrismaThermalModel;
} // model

namespace simulation {
using namespace ecad::model;
class ECAD_API EThermalSimulation
{
public:
    explicit EThermalSimulation(const EThermalSimulationSetup & setup) : m_setup(setup) {}
    virtual ~EThermalSimulation() = default;
    virtual bool Run(CPtr<IModel> model, EFloat & minT, EFloat & maxT) const;
protected:
    const EThermalSimulationSetup & m_setup;
};

class ECAD_API EThermalSimulator
{
public:
    virtual ~EThermalSimulator() = default;
    virtual bool Run(EFloat & min, EFloat & max) const;

protected:
    EThermalSimulator(CPtr<IModel> model, const EThermalSimulationSetup & setup);
    virtual bool RunStaticSimulation(EFloat & minT, EFloat & maxT) const = 0;
    virtual bool RunTransientSimulation(EFloat & minT, EFloat & maxT) const = 0;
    CPtr<IModel> m_model{nullptr};
    const EThermalSimulationSetup & m_setup;
};

class ECAD_API EGridThermalSimulator : public EThermalSimulator
{
public:
    explicit EGridThermalSimulator(CPtr<EGridThermalModel> model, const EThermalSimulationSetup & setup);
    virtual ~EGridThermalSimulator() = default;

protected:
    bool RunStaticSimulation(EFloat & minT, EFloat & maxT) const override;
    bool RunTransientSimulation(EFloat & minT, EFloat & maxT) const override;
};

class ECAD_API EPrismaThermalSimulator : public EThermalSimulator
{
public:
    explicit EPrismaThermalSimulator(CPtr<EPrismaThermalModel> model, const EThermalSimulationSetup & setup);
    virtual ~EPrismaThermalSimulator() = default;

protected:
    bool RunStaticSimulation(EFloat & minT, EFloat & maxT) const override;
    bool RunTransientSimulation(EFloat & minT, EFloat & maxT) const override;
};
}//namespace simulations
}//namespace ecad