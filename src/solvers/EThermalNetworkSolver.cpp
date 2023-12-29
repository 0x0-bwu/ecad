#include "solvers/EThermalNetworkSolver.h"
#include "thermal/utilities/BoundaryNodeReduction.hpp"
#include "thermal/utilities/ThermalNetlistWriter.hpp"
#include "utilities/EPrismaThermalNetworkBuilder.h"
#include "thermal/solver/ThermalNetworkSolver.hpp"
#include "utilities/EGridThermalNetworkBuilder.h"
#include "generic/thread/ThreadPool.hpp"
#include "generic/tools/Format.hpp"
#include "EDataMgr.h"
namespace ecad::esolver {

using namespace emodel;

ECAD_INLINE EFloat CalculateResidual(const std::vector<EFloat> & v1, const std::vector<EFloat> & v2)
{
    ECAD_ASSERT(v1.size() == v2.size());
    EFloat residual = 0;
    size_t size = v1.size();
    for(size_t i = 0; i < size; ++i) {
        residual += std::fabs(v1[i] - v2[i]) / size;
    }
    return residual;
}

ECAD_INLINE void EThermalNetworkSolver::SetSolveSettings(const EThermalNetworkSolveSettings & settings)
{
    m_settings = settings;
}

ECAD_INLINE EGridThermalNetworkSolver::EGridThermalNetworkSolver(const EGridThermalModel & model)
 : m_model(model)
{
}

ECAD_INLINE EGridThermalNetworkStaticSolver::EGridThermalNetworkStaticSolver(const EGridThermalModel & model)
 : EGridThermalNetworkSolver(model)
{
}

ECAD_INLINE bool EGridThermalNetworkStaticSolver::Solve(EFloat refT, std::vector<EFloat> & results)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("grid thermal network static solve")

    results.assign(m_model.TotalGrids(), refT);
    bool needIteration = m_model.NeedIteration();
    if (m_settings.iteration > 0 && math::GT<EFloat>(m_settings.residual, 0)) {

        EFloat residual = m_settings.residual;
        size_t iteration = m_settings.iteration;
        EGridThermalNetworkBuilder builder(m_model);
        do {
            std::vector<EFloat> lastRes(results);
            auto network = builder.Build(lastRes);
            if (nullptr == network) return false;

            if (not m_settings.spiceFile.empty()) {
                auto spiceWriter = ThermalNetlistWriter(*network);
                spiceWriter.SetReferenceTemperature(refT);
                auto res = spiceWriter.WriteSpiceNetlist(m_settings.spiceFile);
                if (res) std::cout << "write out spice file " << m_settings.spiceFile << " successfully!" << std::endl;
            }
            generic::log::Trace("total nodes: %1%", network->Size());
            generic::log::Trace("intake  heat flow: %1%w", builder.summary.iHeatFlow);
            generic::log::Trace("outtake heat flow: %1%w", builder.summary.oHeatFlow);
            
            size_t threads = EDataMgr::Instance().DefaultThreads();
            ThermalNetworkSolver<EFloat> solver(*network, threads);
            solver.SetVerbose(true);
            solver.Solve(refT);

            const auto & nodes = network->GetNodes();
            for (size_t n = 0; n < results.size(); ++n)
                results[n] = nodes[n].t;

            iteration -= 1;
            if(!needIteration || iteration == 0) break;

            residual = CalculateResidual(results, lastRes);
            needIteration = math::GE(residual, m_settings.residual);
            std::cout << "Residual: " << residual << ", Remain Iteration: " << (needIteration ? iteration : 0) << std::endl;
    
        } while (needIteration);
    }

    return true;
}

ECAD_INLINE EGridThermalNetworkTransientSolver::EGridThermalNetworkTransientSolver(const EGridThermalModel & model)
 : EGridThermalNetworkSolver(model)
{
}

ECAD_INLINE bool EGridThermalNetworkTransientSolver::Solve(EFloat refT, std::vector<EFloat> & results)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("grid thermal network transient solve")

    results.assign(m_model.TotalGrids(), refT);
    EGridThermalNetworkBuilder builder(m_model);
    auto network = builder.Build(results);
    if (nullptr == network) return false;
    generic::log::Trace("total nodes: %1%", network->Size());
    generic::log::Trace("intake  heat flow: %1%w", builder.summary.iHeatFlow);
    generic::log::Trace("outtake heat flow: %1%w", builder.summary.oHeatFlow);
            
    size_t threads = EDataMgr::Instance().DefaultThreads();
    ThermalNetworkSolver<EFloat> solver(*network, threads);
    solver.SetVerbose(true);
    solver.Solve(refT);

    const auto & nodes = network->GetNodes();
    for (size_t n = 0; n < results.size(); ++n)
        results[n] = nodes[n].t;

    size_t maxId = std::distance(results.begin(), std::max_element(results.begin(), results.end()));
    std::cout << "hotspot: " << maxId << std::endl;
    std::set<size_t> probs{maxId};
    if (false) {
        ECAD_EFFICIENCY_TRACK("impedence origin")
        using TransSolver = ThermalNetworkTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;
        size_t threads = EDataMgr::Instance().DefaultThreads();
        
        struct Excitation
        {
            EFloat cycle, duty;
            Excitation(EFloat cycle, EFloat duty = 0.5) : cycle(cycle), duty(duty * cycle) {}
            EFloat operator() (EFloat t) const
            {
                return std::fmod(t, cycle) < duty ? 1 : 0.1;
            }
        };

        std::vector<EFloat> cycles;
        for (size_t i = 0; i < 12; ++i)
            cycles.emplace_back(std::pow(10, 0.5 * i - 2.5));
        std::vector<EFloat> dutys {0.01, 0.02, 0.05, 0.1, 0.3, 0.5};
        TransSolver solver(*network, refT, probs);
        generic::thread::ThreadPool pool(threads);
        std::cout << "available threads: " << pool.Threads() << std::endl;
            
        for (size_t i = 0, index = 0; i < cycles.size(); ++i) {
            for (size_t j = 0; j < dutys.size(); ++j) {
                auto cycle = cycles.at(i);
                auto duty = dutys.at(j);
                pool.Submit(
                    [cycle, duty, refT, index, &solver, &network] {
                        Excitation excitation(cycle, duty);
                        StateType initState(network->Size(), refT);
                        EFloat t0 = 0, t1 = std::min<EFloat>(std::max<EFloat>(10, cycle * 50), 20), dt = std::min<EFloat>(0.5 * cycle * duty, 0.1);
                        std::ofstream ofs(fmt::Fmt2Str("./origin_%1%.txt", index));
                        Recorder recorder(solver, ofs, dt * 0.05);
                        solver.Solve(initState, t0, t1, dt, recorder, excitation);
                        ofs.close();
                        std::cout << "done with c: " << cycle << ", d: " << duty << std::endl;
                    }
                );
                index++;
            }
        }
    }  
    if (false) {
        ECAD_EFFICIENCY_TRACK("impedence reduce")
        using TransSolver = ThermalNetworkReducedTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;
        size_t threads = EDataMgr::Instance().DefaultThreads();
        
        struct Excitation
        {
            EFloat cycle, duty;
            Excitation(EFloat cycle, EFloat duty = 0.5) : cycle(cycle), duty(duty * cycle) {}
            EFloat operator() (EFloat t) const
            {
                return std::fmod(t, cycle) < duty ? 1 : 0.1;
            }
        };

        std::vector<EFloat> cycles;
        for (size_t i = 0; i < 12; ++i)
            cycles.emplace_back(std::pow(10, 0.5 * i - 2.5));
        std::vector<EFloat> dutys {0.01, 0.02, 0.05, 0.1, 0.3, 0.5};
        TransSolver solver(*network, refT, probs);
        generic::thread::ThreadPool pool(threads);
        std::cout << "available threads: " << pool.Threads() << std::endl;            
        for (size_t i = 0, index = 0; i < cycles.size(); ++i) {
            for (size_t j = 0; j < dutys.size(); ++j) {
                auto cycle = cycles.at(i);
                auto duty = dutys.at(j);
                pool.Submit(
                    [cycle, duty, refT, index, &solver, &network] {
                        Excitation excitation(cycle, duty);
                        StateType initState(network->Size(), refT);
                        EFloat t0 = 0, t1 = std::min<EFloat>(std::max<EFloat>(10, cycle * 50), 20), dt = std::min<EFloat>(0.5 * cycle * duty, 0.1);
                        std::ofstream ofs(fmt::Fmt2Str("./reduce_%1%.txt", index));
                        Recorder recorder(solver, ofs, dt * 0.05);
                        solver.Solve(initState, t0, t1, dt, recorder, excitation);
                        ofs.close();
                        std::cout << "done with c: " << cycle << ", d: " << duty << std::endl;
                    }
                );
                index++;
            }
        }
    }  

    struct Excitation
    {
        EFloat cycle, duty;
        Excitation(EFloat cycle, EFloat duty = 0.5) : cycle(cycle), duty(duty * cycle) {}
        EFloat operator() (EFloat t) const
        {
            return std::abs(std::sin(math::pi * t / cycle));
        }
    };
    EFloat cycle = 0.5, duty = 0.5;
    Excitation excitation(cycle, duty);  
    generic::thread::ThreadPool pool(2);
    pool.Submit([&]{
        ECAD_EFFICIENCY_TRACK("transient orig")
        using TransSolver = ThermalNetworkTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;
        TransSolver solver(*network, refT, probs);
        std::ofstream ofs("./orig.txt");
        Recorder recorder(solver, ofs, 0.01);
        StateType initState(solver.StateSize(), refT);
        solver.Solve(initState, EFloat{0}, EFloat{10}, EFloat{0.01}, std::move(recorder), excitation);
    });
    // bool reduceBounds = false;
    // if (reduceBounds) {
    //     auto minT = network->MinT();
    //     auto maxT = network->MaxT();
    //     BoundaryNodeReduction bnReduction(*network);
    //     bnReduction.Reduce(minT, minT + (maxT - minT) * 0.3, 3);
    // }
    pool.Submit([&]{
        ECAD_EFFICIENCY_TRACK("transient mor")
        using TransSolver = ThermalNetworkReducedTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;
        StateType initT(network->Size(), refT);
        TransSolver solver(*network, refT, probs);
        std::ofstream ofs("./mor.txt");
        Recorder recorder(solver, ofs, 0.01);
        solver.Solve(initT, EFloat{0}, EFloat{10}, EFloat{0.01}, std::move(recorder), excitation);
    }); 
    return true;
}

ECAD_INLINE EPrismaThermalNetworkSolver::EPrismaThermalNetworkSolver(const EPrismaThermalModel & model)
 : m_model(model)
{
}

ECAD_INLINE EPrismaThermalNetworkStaticSolver::EPrismaThermalNetworkStaticSolver(const EPrismaThermalModel & model)
 : EPrismaThermalNetworkSolver(model)
{
}

ECAD_INLINE bool EPrismaThermalNetworkStaticSolver::Solve(EFloat refT, std::vector<EFloat> & results)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("prisma thermal network static solve")

    results.assign(m_model.TotalElements(), refT);
    bool needIteration = m_model.NeedIteration();
    if (m_settings.iteration > 0 && math::GT<EFloat>(m_settings.residual, 0)) {

        EFloat residual = m_settings.residual;
        size_t iteration = m_settings.iteration;
        EPrismaThermalNetworkBuilder builder(m_model);
        do {
            std::vector<EFloat> lastRes(results);
            auto network = builder.Build(lastRes);
            if (nullptr == network) return false;

            generic::log::Trace("total nodes: %1%", network->Size());
            generic::log::Trace("intake  heat flow: %1%w", builder.summary.iHeatFlow);
            generic::log::Trace("outtake heat flow: %1%w", builder.summary.oHeatFlow);
            
            size_t threads = EDataMgr::Instance().DefaultThreads();
            ThermalNetworkSolver<EFloat> solver(*network, threads);
            solver.SetVerbose(true);
            solver.Solve(refT);

            const auto & nodes = network->GetNodes();
            for (size_t n = 0; n < results.size(); ++n)
                results[n] = nodes[n].t;

            auto maxIter = std::max_element(results.begin(), results.end());
            auto minIter = std::min_element(results.begin(), results.end());
            generic::log::Trace("hotspot: %1%, maxT: %2%, minT: %3%", std::distance(results.begin(), maxIter), *maxIter, *minIter);

            iteration -= 1;
            if(!needIteration || iteration == 0) break;

            residual = CalculateResidual(results, lastRes);
            needIteration = math::GE(residual, m_settings.residual);
            std::cout << "Residual: " << residual << ", Remain Iteration: " << (needIteration ? iteration : 0) << std::endl;
    
        } while (needIteration);
    }

    return true;
}

ECAD_INLINE EPrismaThermalNetworkTransientSolver::EPrismaThermalNetworkTransientSolver(const EPrismaThermalModel & model)
 : EPrismaThermalNetworkSolver(model)
{
}

ECAD_INLINE bool EPrismaThermalNetworkTransientSolver::Solve(EFloat refT, std::vector<EFloat> & results)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("prisma thermal network transient solve")
    return false;
}

} //namespace ecad::esolver
