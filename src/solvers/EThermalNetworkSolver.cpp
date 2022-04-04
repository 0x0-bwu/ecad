#ifndef ECAD_HEADER_ONLY
#include "solvers/EThermalNetworkSolver.h"
#endif

#include "models/thermal/utilities/EThermalModelReduction.h"
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

ECAD_INLINE EThermalNetworkSolver::~EThermalNetworkSolver()
{
}

ECAD_INLINE void EThermalNetworkSolver::SetSolveSettings(const EThermalNetworkSolveSettings & settings)
{
    m_settings = settings;
}

ECAD_INLINE EGridThermalNetworkDirectSolver::EGridThermalNetworkDirectSolver(const EGridThermalModel & model)
 : m_model(model)
{
}

ECAD_INLINE EGridThermalNetworkDirectSolver::~EGridThermalNetworkDirectSolver()
{
}

ECAD_INLINE bool EGridThermalNetworkDirectSolver::Solve(ESimVal refT, std::vector<ESimVal> & results)
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

ECAD_INLINE EGridThermalNetworkReductionSolver::EGridThermalNetworkReductionSolver(const EGridThermalModel & model)
 : m_model(model)
{
}

ECAD_INLINE EGridThermalNetworkReductionSolver::~EGridThermalNetworkReductionSolver()
{
}

ECAD_INLINE bool EGridThermalNetworkReductionSolver::Solve(ESimVal refT, std::vector<ESimVal> & results)
{

}

ECAD_INLINE bool EGridThermalNetworkReductionSolver::SolveRecursively(const EGridThermalModel & model, ESimVal refT, std::vector<ESimVal> & results, size_t reduceOrder)
{
    if(reduceOrder == 0) return SolveDirectly(model, refT, results);

    reduceOrder--;
    auto reduced = etherm::utils::makeReductionModel(model);
    if(nullptr == reduced) return false;

    std::vector<ESimVal> rdResults;
    if(!SolveRecursively(*reduced, refT, rdResults, reduceOrder)) return false;

    results.resize(model.TotalGrids());
    EGridThermalNetworkBuilder builder(model);
    EGridThermalNetworkBuilder rdBuilder(*reduced);

    //bc
    SPtr<EGridBCModel> topBC = nullptr, botBC = nullptr;
    model.GetTopBotBCModel(topBC, botBC);

    EGridThermalModel::BCType topT, botT;
    model.GetTopBotBCType(topT, botT);

    auto getRdT = [&rdResults, &rdBuilder](const ESize3D & index)
    {
        return rdResults.at(rdBuilder.GetFlattenIndex(index));
    };

    //todo, multi-threads
    using namespace thermal::model;
    using Orient = typename EGridThermalNetworkBuilder::Orientation;
    auto rdSize = reduced->ModelSize();
    for(size_t z = 0; z < rdSize.z; ++z) {
        auto pwrModel = model.GetPowerModel(z);
        for(size_t x = 0; x < rdSize.x; ++x) {
            for(size_t y = 0; y < rdSize.y; ++y) {
                ESize3D rdIndex(x, y, z);
                auto t = getRdT(rdIndex);
                ESize2D ll = LowLeftIndexFromReducedModelIndex(ESize2D(x, y));
                ESize2D lr = ESize2D(ll.x + 1, ll.y);
                ESize2D ul = ESize2D(ll.x, ll.y + 1);
                ESize2D ur = ESize2D(ll.x + 1, ll.y + 1);
                if(model.isValid(lr) && model.isValid(ul)) {
                    auto network = std::make_unique<ThermalNetwork<ESimVal> >(4);
                    auto k0 = builder.GetCompositeMatK(ESize3D(ll, z), t);
                    auto k1 = builder.GetCompositeMatK(ESize3D(lr, z), t);
                    auto k2 = builder.GetCompositeMatK(ESize3D(ur, z), t);
                    auto k3 = builder.GetCompositeMatK(ESize3D(ul, z), t);

                    auto k01 = 0.5 * k0[0] + 0.5 * k1[0];
                    auto r01 = k01 * builder.GetXGridArea(z) / builder.GetXGridLength();
                    network->SetR(0, 1, r01);

                    auto k02 = 0.5 * k0[1] + 0.5 * k2[1];
                    auto r02 = k02 * builder.GetYGridArea(z) / builder.GetYGridLength();
                    network->SetR(0, 2, r02);

                    auto k13 = 0.5 * k1[1] + 0.5 * k3[1];
                    auto r13 = k13 * builder.GetYGridArea(z) / builder.GetYGridLength();
                    network->SetR(1, 3, r13);

                    auto k23 = 0.5 * k2[0] + 0.5 * k3[0];
                    auto r23 = k23 * builder.GetXGridArea(z) / builder.GetXGridLength();
                    network->SetR(2, 3, r23);
                    
                    size_t node;

                    //left
                    auto rdLeft = rdBuilder.GetNeighbor(rdIndex, Orient::Left);
                    if(reduced->isValid(rdLeft)) {
                        auto t = getRdT(rdLeft);
                        //0
                        auto kL0 = builder.GetCompositeMatK(ESize3D(ll.x - 1, ll.y, z), t);
                        auto k0L = 0.5 * k0[0] + 0.5 * kL0[0];
                        auto r0L = k0L * builder.GetXGridArea(z) / builder.GetXGridLength();
                        node = network->AppendNode(t);
                        network->SetR(0, node, r0L);
                        //2
                        auto kL2 = builder.GetCompositeMatK(ESize3D(ul.x - 1, ll.y, z), t);
                        auto k2L = 0.5 * k2[0] + 0.5 * kL2[0];
                        auto r2L = k2L * builder.GetXGridArea(z) / builder.GetXGridLength();
                        node = network->AppendNode(t);
                        network->SetR(2, node, r2L);
                    }

                    //front
                    auto rdFront = rdBuilder.GetNeighbor(rdIndex, Orient::Front);
                    if(reduced->isValid(rdFront)) {
                        auto t = getRdT(rdFront);
                        //0
                        auto kF0 = builder.GetCompositeMatK(ESize3D(ll.x, ll.y - 1, z), t);
                        auto k0F = 0.5 * k0[1] + 0.5 * kF0[1];
                        auto r0F = k0F * builder.GetYGridArea(z) / builder.GetYGridLength();
                        node = network->AppendNode(t);
                        network->SetR(0, node, r0F);

                        //1
                        auto kF1 = builder.GetCompositeMatK(ESize3D(lr.x, lr.y - 1, z), t);
                        auto k1F = 0.5 * k1[1] + 0.5 * kF1[1];
                        auto r1F = k1F * builder.GetYGridArea(z) / builder.GetYGridLength();
                        node = network->AppendNode(t);
                        network->SetR(1, node, r1F);
                    }

                    //right
                    auto rdRight = rdBuilder.GetNeighbor(rdIndex, Orient::Right);
                    if(reduced->isValid(rdRight)) {
                        auto t = getRdT(rdRight);
                        //1
                        auto kR1 = builder.GetCompositeMatK(ESize3D(lr.x + 1, lr.y, z), t);
                        auto k1R = 0.5 * k1[0] + 0.5 * kR1[0];
                        auto r1R = k1R * builder.GetXGridArea(z) / builder.GetXGridLength();
                        node = network->AppendNode(t);
                        network->SetR(1, node, r1R);
                        
                        //3
                        auto kR3 = builder.GetCompositeMatK(ESize3D(ur.x + 1, ur.y, z), t);
                        auto k3R = 0.5 * k3[0] + 0.5 * kR3[0];
                        auto r3R = k3R * builder.GetXGridArea(z) / builder.GetXGridLength();
                        node = network->AppendNode(t);
                        network->SetR(3, node, r3R);
                    }

                    //end
                    auto rdEnd = rdBuilder.GetNeighbor(rdIndex, Orient::End);
                    if(reduced->isValid(rdEnd)) {
                        auto t = getRdT(rdEnd);
                        //2
                        auto kE2 = builder.GetCompositeMatK(ESize3D(ul.x, ul.y + 1, z), t);
                        auto k2E = 0.5 * k2[1] + 0.5 * kE2[1];
                        auto r2E = k2E * builder.GetYGridArea(z) / builder.GetYGridLength();
                        node = network->AppendNode(t);
                        network->SetR(2, node, r2E);

                        //3
                        auto kE3 = builder.GetCompositeMatK(ESize3D(ur.x, ur.y + 1, z), t);
                        auto k3E = 0.5 * k3[1] + 0.5 * kE3[1];
                        auto r3E = k3E * builder.GetYGridArea(z) / builder.GetYGridLength();
                    }

                    //top
                    auto rdTop = rdBuilder.GetNeighbor(rdIndex, Orient::Top);
                    if(reduced->isValid(rdTop)) {
                        auto t = getRdT(rdTop);
                        for(size_t n = 0; n < 4; ++n) {
                            //todo
                        }
                    }
                }
            }
        }
    }
    return false;//wbtest
}

ECAD_INLINE bool EGridThermalNetworkReductionSolver::SolveDirectly(const EGridThermalModel & model, ESimVal refT, std::vector<ESimVal> & results)
{
    EGridThermalNetworkDirectSolver solver(model);
    solver.SetSolveSettings(m_settings);
    return solver.Solve(refT, results);
}

ECAD_INLINE ESize2D EGridThermalNetworkReductionSolver::LowLeftIndexFromReducedModelIndex(const ESize2D & index)
{
    return ESize2D(index.x * 2, index.y * 2);
}

ECAD_INLINE ESize2D EGridThermalNetworkReductionSolver::ReducedModelIndexFromModelIndex(const ESize2D & index)
{
    return ESize2D(index.x / 2, index.y / 2);
}



}//namespace esolver
}//namespace ecad