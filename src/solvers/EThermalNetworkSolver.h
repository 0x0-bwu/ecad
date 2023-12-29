#pragma once
#include "models/thermal/EPrismaThermalModel.h"
#include "models/thermal/EGridThermalModel.h"
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {
namespace esolver {
using namespace emodel;
using namespace emodel::etherm;
ECAD_API EFloat CalculateResidual(const std::vector<EFloat> & v1, const std::vector<EFloat> & v2);

class ECAD_API EThermalNetworkSolver
{
public:
    virtual ~EThermalNetworkSolver() = default;
    void SetSolveSettings(const EThermalNetworkSolveSettings & settings);
protected:
    EThermalNetworkSolveSettings m_settings;
};

class ECAD_API EGridThermalNetworkSolver : public EThermalNetworkSolver
{
public:
    virtual ~EGridThermalNetworkSolver() = default;    

protected:
    explicit EGridThermalNetworkSolver(const EGridThermalModel & model);
    const EGridThermalModel & m_model;
};

class ECAD_API EGridThermalNetworkStaticSolver : public EGridThermalNetworkSolver
{
public:
    explicit EGridThermalNetworkStaticSolver(const EGridThermalModel & model);
    virtual ~EGridThermalNetworkStaticSolver() = default;
    bool Solve(EFloat refT, std::vector<EFloat> & results);
};

class ECAD_API EGridThermalNetworkTransientSolver : public EGridThermalNetworkSolver
{
public:
    explicit EGridThermalNetworkTransientSolver(const EGridThermalModel & model);
    virtual ~EGridThermalNetworkTransientSolver() = default;
    bool Solve(EFloat refT, std::vector<EFloat> & results);
};

class ECAD_API EPrismaThermalNetworkSolver : public EThermalNetworkSolver
{
public:
    virtual ~EPrismaThermalNetworkSolver() = default;

protected:
    explicit EPrismaThermalNetworkSolver(const EPrismaThermalModel & model);
    const EPrismaThermalModel & m_model;
};

class ECAD_API EPrismaThermalNetworkStaticSolver : public EPrismaThermalNetworkSolver
{
public:
    explicit EPrismaThermalNetworkStaticSolver(const EPrismaThermalModel & model);
    virtual ~EPrismaThermalNetworkStaticSolver() = default;
    bool Solve(EFloat refT, std::vector<EFloat> & results);
};

class ECAD_API EPrismaThermalNetworkTransientSolver : public EPrismaThermalNetworkSolver
{
public:
    explicit EPrismaThermalNetworkTransientSolver(const EPrismaThermalModel & model);
    virtual ~EPrismaThermalNetworkTransientSolver() = default;
    bool Solve(EFloat refT, std::vector<EFloat> & results);
};

}//namesapce esolver
}//namespace ecad