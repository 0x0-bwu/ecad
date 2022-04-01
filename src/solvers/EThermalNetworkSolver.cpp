#ifndef ECAD_HEADER_ONLY
#include "solvers/EThermalNetworkSolver.h"
#endif

#include "thermal/solver/ThermalNetworkSolver.hpp"
#include "utilities/EThermalNetworkBuilder.h"

namespace ecad {
namespace esolver {

using namespace emodel;

ECAD_INLINE ESimVal CalculateResidual(const std::vector<ESimVal> & v1, const std::vector<ESimVal> & v2)
{
    GENERIC_ASSERT(v1.size() == v2.size());
    ESimVal residual = 0;
    size_t size = v1.size();
    for(size_t i = 0; i < size; ++i) {
        residual += std::fabs(v1[i] - v2[i]) / size;
    }
    return residual;
}


ECAD_INLINE EThermalNetworkSolver::EThermalNetworkSolver(const EGridThermalModel & model)
 : m_model(model)
{
}

ECAD_INLINE EThermalNetworkSolver::~EThermalNetworkSolver()
{
}

ECAD_INLINE void EThermalNetworkSolver::SetSolveSettings(const EThermalNetworkSolveSettings & settings)
{
    m_settings = settings;
}

ECAD_INLINE bool EThermalNetworkSolver::Solve(ESimVal refT, std::vector<ESimVal> & results)
{
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("thermal network solve")

    auto size = m_model.TotalGrids();
    results.assign(size, refT);
    if(m_settings.iteration > 0 && math::GT<EValue>(m_settings.residual, 0)) {

        EValue residual = m_settings.residual;
        size_t iteration = m_settings.iteration;
        EGridThermalNetworkBuilder builder(m_model);
        while(iteration != 0 && math::GE(residual, m_settings.residual)) {
            std::vector<ESimVal> lastRes(results);
            auto network = builder.Build(lastRes);
            if(nullptr == network) return false;

            std::cout << "intake  heat flow: " << builder.summary.iHeatFlow << "w" << std::endl;
            std::cout << "outtake heat flow: " << builder.summary.oHeatFlow << "w" << std::endl;
            
            ThermalNetworkSolver<ESimVal> solver(*network);
            solver.Solve(refT);

            const auto & nodes = network->GetNodes();
            for(size_t n = 0; n < nodes.size(); ++n)
                results[n] = nodes[n].t;

            residual = CalculateResidual(results, lastRes);
            iteration -= 1;

            std::cout << "Residual: " << residual << ", Remain Iteration: ";
            std::cout << (math::GE(residual, m_settings.residual) ? iteration : 0) << std::endl;//wbtest; 
        }
    }

    return true;
}
}//namespace esolver
}//namespace ecad