#ifndef ECAD_ESOLVER_ETHERMALNETWORKSOLVER_HPP
#define ECAD_ESOLVER_ETHERMALNETWORKSOLVER_HPP
#include "models/thermal/EGridThermalModel.h"
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {
namespace esolver {
using namespace emodel;
using namespace emodel::etherm;
ECAD_API ESimVal CalculateResidual(const std::vector<ESimVal> & v1, const std::vector<ESimVal> & v2);

class ECAD_API EThermalNetworkSolver
{
public:
    virtual ~EThermalNetworkSolver();
    void SetSolveSettings(const EThermalNetworkSolveSettings & settings);
protected:
    EThermalNetworkSolveSettings m_settings;
};

class ECAD_API EGridThermalNetworkSolver : public EThermalNetworkSolver
{
public:
    explicit EGridThermalNetworkSolver(const EGridThermalModel & model);
    virtual ~EGridThermalNetworkSolver();
    bool Solve(ESimVal refT, std::vector<ESimVal> & results);
private:
    const EGridThermalModel & m_model;
};

class ECAD_API EGridThermalNetworkReductionSolver : public EThermalNetworkSolver
{

};

}//namesapce esolver
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalNetworkSolver.cpp"
#endif

#endif//ECAD_ESOLVER_ETHERMALNETWORKSOLVER_HPP