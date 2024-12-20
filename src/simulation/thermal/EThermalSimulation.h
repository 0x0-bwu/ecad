#pragma once
#include "basic/ECadSettings.h"
namespace ecad {
class IModel;
namespace model {
class EGridThermalModel;
class EPrismThermalModel;
class EStackupPrismThermalModel;
} // model

namespace simulation {
using namespace ecad::model;
class ECAD_API EThermalSimulation
{
public:
    explicit EThermalSimulation(CPtr<IModel> model, const EThermalSimulationSetup & setup);
    virtual ~EThermalSimulation() = default;
    virtual EPair<EFloat, EFloat> RunStaticSimulation(std::vector<EFloat> & temperatures) const;
    virtual EPair<EFloat, EFloat> RunTransientSimulation(const EThermalTransientExcitation & excitation) const;
protected:
    CPtr<IModel> m_model{nullptr};
    const EThermalSimulationSetup & m_setup;
};

class ECAD_API EThermalSimulator
{
public:
    virtual ~EThermalSimulator() = default;
    virtual EPair<EFloat, EFloat> RunStaticSimulation(std::vector<EFloat> & temperatures) const = 0;
    virtual EPair<EFloat, EFloat> RunTransientSimulation(const EThermalTransientExcitation & excitation) const = 0;
protected:
    EThermalSimulator(CPtr<IModel> model, const EThermalSimulationSetup & setup);
    CPtr<IModel> m_model{nullptr};
    const EThermalSimulationSetup & m_setup;
};

class ECAD_API EGridThermalSimulator : public EThermalSimulator
{
public:
    explicit EGridThermalSimulator(CPtr<EGridThermalModel> model, const EThermalSimulationSetup & setup);
    virtual ~EGridThermalSimulator() = default;
    EPair<EFloat, EFloat> RunStaticSimulation(std::vector<EFloat> & temperatures) const override;
    EPair<EFloat, EFloat> RunTransientSimulation(const EThermalTransientExcitation & excitation) const override;
};

class ECAD_API EPrismThermalSimulator : public EThermalSimulator
{
public:
    explicit EPrismThermalSimulator(CPtr<EPrismThermalModel> model, const EThermalSimulationSetup & setup);
    virtual ~EPrismThermalSimulator() = default;
    EPair<EFloat, EFloat> RunStaticSimulation(std::vector<EFloat> & temperatures) const override;
    EPair<EFloat, EFloat> RunTransientSimulation(const EThermalTransientExcitation & excitation) const override;
};

class ECAD_API EStackupPrismThermalSimulator : public EThermalSimulator
{
public:
    explicit EStackupPrismThermalSimulator(CPtr<EStackupPrismThermalModel> model, const EThermalSimulationSetup & setup);
    virtual ~EStackupPrismThermalSimulator() = default;
    EPair<EFloat, EFloat> RunStaticSimulation(std::vector<EFloat> & temperatures) const override;
    EPair<EFloat, EFloat> RunTransientSimulation(const EThermalTransientExcitation & excitation) const override;
};
}//namespace simulations
}//namespace ecad