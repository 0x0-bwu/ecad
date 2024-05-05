#pragma once
#include "basic/ECadSettings.h"
#include "model/thermal/EStackupPrismThermalModel.h"
#include "model/thermal/EPrismThermalModel.h"
#include "model/thermal/EGridThermalModel.h"
namespace ecad {
namespace solver {
using namespace ecad::model;
ECAD_API EFloat CalculateResidual(const std::vector<EFloat> & v1, const std::vector<EFloat> & v2);

class ECAD_API EThermalNetworkSolver
{
public:
    virtual ~EThermalNetworkSolver() = default;
    virtual bool Solve(EFloat & minT, EFloat & maxT) = 0;
};

class ECAD_API EThermalNetworkStaticSolver : public EThermalNetworkSolver
{
public:
    EThermalNetworkStaticSolveSettings settings;
    virtual ~EThermalNetworkStaticSolver() = default;

    template <typename ThermalNetworkBuilder>
    bool Solve(const typename ThermalNetworkBuilder::ModelType & model, std::vector<EFloat> & results) const;
};

class ECAD_API EThermalNetworkTransientSolver : public EThermalNetworkSolver
{
public:
    EThermalNetworkTransientSolveSettings settings;
    virtual ~EThermalNetworkTransientSolver() = default;

    template <typename ThermalNetworkBuilder>
    bool Solve(const typename ThermalNetworkBuilder::ModelType & model, EFloat & minT, EFloat & maxT) const;
};

class ECAD_API EGridThermalNetworkSolver
{
public:
    virtual ~EGridThermalNetworkSolver() = default;    

protected:
    explicit EGridThermalNetworkSolver(const EGridThermalModel & model);
    const EGridThermalModel & m_model;
};

class ECAD_API EGridThermalNetworkStaticSolver : public EGridThermalNetworkSolver, EThermalNetworkStaticSolver
{
public:
    using EThermalNetworkStaticSolver::settings;
    explicit EGridThermalNetworkStaticSolver(const EGridThermalModel & model);
    virtual ~EGridThermalNetworkStaticSolver() = default;
    bool Solve(EFloat & minT, EFloat & maxT) override;
};

class ECAD_API EGridThermalNetworkTransientSolver : public EGridThermalNetworkSolver, EThermalNetworkTransientSolver
{
public:
    using EThermalNetworkTransientSolver::settings;
    explicit EGridThermalNetworkTransientSolver(const EGridThermalModel & model);
    virtual ~EGridThermalNetworkTransientSolver() = default;
    bool Solve(EFloat & minT, EFloat & maxT) override;
};

class ECAD_API EPrismThermalNetworkSolver
{
public:
    virtual ~EPrismThermalNetworkSolver() = default;

protected:
    explicit EPrismThermalNetworkSolver(const EPrismThermalModel & model);
    const EPrismThermalModel & m_model;
};

class ECAD_API EPrismThermalNetworkStaticSolver : public EPrismThermalNetworkSolver, EThermalNetworkStaticSolver
{
public:
    using EThermalNetworkStaticSolver::settings;
    explicit EPrismThermalNetworkStaticSolver(const EPrismThermalModel & model);
    virtual ~EPrismThermalNetworkStaticSolver() = default;
    bool Solve(EFloat & minT, EFloat & maxT) override;
};

class ECAD_API EPrismThermalNetworkTransientSolver : public EPrismThermalNetworkSolver, EThermalNetworkTransientSolver
{
public:
    using EThermalNetworkTransientSolver::settings;
    explicit EPrismThermalNetworkTransientSolver(const EPrismThermalModel & model);
    virtual ~EPrismThermalNetworkTransientSolver() = default;
    bool Solve(EFloat & minT, EFloat & maxT) override;
};

class ECAD_API EStackupPrismThermalNetworkSolver
{
public:
    virtual ~EStackupPrismThermalNetworkSolver() = default;

protected:
    explicit EStackupPrismThermalNetworkSolver(const EStackupPrismThermalModel & model);
    const EStackupPrismThermalModel & m_model;
};

class ECAD_API EStackupPrismThermalNetworkStaticSolver : public EStackupPrismThermalNetworkSolver, EThermalNetworkStaticSolver
{
public:
    using EThermalNetworkStaticSolver::settings;
    explicit EStackupPrismThermalNetworkStaticSolver(const EStackupPrismThermalModel & model);
    virtual ~EStackupPrismThermalNetworkStaticSolver() = default;
    bool Solve(EFloat & minT, EFloat & maxT) override;
};

class ECAD_API EStackupPrismThermalNetworkTransientSolver : public EStackupPrismThermalNetworkSolver, EThermalNetworkTransientSolver
{
public:
    using EThermalNetworkTransientSolver::settings;
    explicit EStackupPrismThermalNetworkTransientSolver(const EStackupPrismThermalModel & model);
    virtual ~EStackupPrismThermalNetworkTransientSolver() = default;
    bool Solve(EFloat & minT, EFloat & maxT) override;
};

}//namesapce solver
}//namespace ecad