#pragma once
#include "models/thermal/EGridThermalModel.h"
#include "ECadSettings.h"
#include "ECadCommon.h"
#include <atomic>
namespace ecad {
namespace esolver {
using namespace emodel;
using namespace emodel::etherm;
ECAD_API ESimVal CalculateResidual(const std::vector<ESimVal> & v1, const std::vector<ESimVal> & v2);

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
    bool Solve(ESimVal refT, std::vector<ESimVal> & results);
};

class ECAD_API EGridThermalNetworkTransientSolver : public EGridThermalNetworkSolver
{
public:
    explicit EGridThermalNetworkTransientSolver(const EGridThermalModel & model);
    virtual ~EGridThermalNetworkTransientSolver() = default;
    bool Solve(ESimVal refT, std::vector<ESimVal> & results);
};

}//namesapce esolver
}//namespace ecad