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

class ECAD_API EGridThermalNetworkDirectSolver : public EThermalNetworkSolver
{
public:
    explicit EGridThermalNetworkDirectSolver(const EGridThermalModel & model);
    virtual ~EGridThermalNetworkDirectSolver();
    bool Solve(ESimVal refT, std::vector<ESimVal> & results);
private:
    const EGridThermalModel & m_model;
};

struct EGridThermalModelResultPair
{
    const EGridThermalModel & model;
    std::vector<ESimVal> & resultsT;

    EGridThermalModelResultPair(const EGridThermalModel & model, std::vector<ESimVal> & resultsT)
     : model(model), resultsT(resultsT) {}
    virtual ~EGridThermalModelResultPair() = default;
};

class ECAD_API EGridThermalNetworkReductionSolver : public EThermalNetworkSolver
{
public:
    explicit EGridThermalNetworkReductionSolver(const EGridThermalModel & model, size_t reduceOrder = 0);
    virtual ~EGridThermalNetworkReductionSolver();
    bool Solve(ESimVal refT, std::vector<ESimVal> & results);

private:
    bool SolveRecursively(const EGridThermalModel & model, ESimVal refT, std::vector<ESimVal> & results, size_t reduceOrder);
    bool SolveDirectly(const EGridThermalModel & model, ESimVal refT, std::vector<ESimVal> & results);

private:
    static ESize2D LowLeftIndexFromReducedModelIndex(const ESize2D & index);
    static ESize2D ReducedModelIndexFromModelIndex(const ESize2D & index);

private:
    const EGridThermalModel & m_model;
    size_t m_reduceOrder = 0;
};

}//namesapce esolver
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalNetworkSolver.cpp"
#endif

#endif//ECAD_ESOLVER_ETHERMALNETWORKSOLVER_HPP