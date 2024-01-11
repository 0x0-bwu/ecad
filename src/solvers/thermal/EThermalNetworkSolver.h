#pragma once
#include "models/thermal/EPrismaThermalModel.h"
#include "models/thermal/EGridThermalModel.h"
#include "ECadSettings.h"
#include "ECadCommon.h"
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
    bool Solve(const typename ThermalNetworkBuilder::ModelType & model, std::vector<EFloat> & results);
};

class ECAD_API EThermalNetworkTransientSolver : public EThermalNetworkSolver
{
public:
    EThermalNetworkTransientSolveSettings settings;
    virtual ~EThermalNetworkTransientSolver() = default;
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

class ECAD_API EPrismaThermalNetworkSolver
{
public:
    virtual ~EPrismaThermalNetworkSolver() = default;

protected:
    explicit EPrismaThermalNetworkSolver(const EPrismaThermalModel & model);
    const EPrismaThermalModel & m_model;
};

class ECAD_API EPrismaThermalNetworkStaticSolver : public EPrismaThermalNetworkSolver, EThermalNetworkStaticSolver
{
public:
    using EThermalNetworkStaticSolver::settings;
    explicit EPrismaThermalNetworkStaticSolver(const EPrismaThermalModel & model);
    virtual ~EPrismaThermalNetworkStaticSolver() = default;
    bool Solve(EFloat & minT, EFloat & maxT) override;
};

class ECAD_API EPrismaThermalNetworkTransientSolver : public EPrismaThermalNetworkSolver, EThermalNetworkTransientSolver
{
public:
    using EThermalNetworkTransientSolver::settings;
    explicit EPrismaThermalNetworkTransientSolver(const EPrismaThermalModel & model);
    virtual ~EPrismaThermalNetworkTransientSolver() = default;
    bool Solve(EFloat & minT, EFloat & maxT) override;
};

}//namesapce solver
}//namespace ecad