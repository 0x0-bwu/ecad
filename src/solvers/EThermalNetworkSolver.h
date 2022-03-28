#ifndef ECAD_ESOLVER_ETHERMALNETWORKSOLVER_HPP
#define ECAD_ESOLVER_ETHERMALNETWORKSOLVER_HPP
#include "models/EThermalModel.h"
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {
namespace esolver {
using namespace emodel;

ECAD_API ESimVal CalculateResidual(const std::vector<ESimVal> & v1, const std::vector<ESimVal> & v2);

class ECAD_API EThermalNetworkSolver
{
public:
    explicit EThermalNetworkSolver(const EGridThermalModel & model);
    virtual ~EThermalNetworkSolver();

    void SetSolveSettings(const EThermalNetworkSolveSettings & settings);
    bool Solve(ESimVal refT, std::vector<ESimVal> & results);

private:
    EThermalNetworkSolveSettings m_settings;
    const EGridThermalModel & m_model;
};

}//namesapce esolver
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalNetworkSolver.cpp"
#endif

#endif//ECAD_ESOLVER_ETHERMALNETWORKSOLVER_HPP