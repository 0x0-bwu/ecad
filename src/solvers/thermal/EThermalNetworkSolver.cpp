#include "EThermalNetworkSolver.h"
#include "solvers/thermal/network/utilities/BoundaryNodeReduction.hpp"
#include "solvers/thermal/network/utilities/ThermalNetlistWriter.hpp"
#include "solvers/thermal/network/ThermalNetworkSolver.hpp"
#include "models/thermal/io/EPrismaThermalModelIO.h"
#include "utilities/EPrismaThermalNetworkBuilder.h"
#include "utilities/EGridThermalNetworkBuilder.h"
#include "generic/thread/ThreadPool.hpp"
#include "generic/tools/Format.hpp"
namespace ecad::solver {

using namespace ecad::model;

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

ECAD_INLINE EGridThermalNetworkSolver::EGridThermalNetworkSolver(const EGridThermalModel & model)
 : m_model(model)
{
}

ECAD_INLINE EGridThermalNetworkStaticSolver::EGridThermalNetworkStaticSolver(const EGridThermalModel & model)
 : EGridThermalNetworkSolver(model)
{
}

ECAD_INLINE bool EGridThermalNetworkStaticSolver::Solve(EFloat & minT, EFloat & maxT)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("grid thermal network static solve")

    EGridThermalNetworkBuilder builder(m_model);
    std::vector<EFloat> results(m_model.TotalGrids(), settings.iniT);
    
    EFloat residual = maxFloat;
    size_t iteration = m_model.NeedIteration() ? settings.iteration : 1;
    do {
        std::vector<EFloat> prevRes(results);
        auto network = builder.Build(prevRes);
        if (nullptr ==  network) return false;
        ECAD_TRACE("total nodes: %1%", network->Size())
        ECAD_TRACE("intake  heat flow: %1%w", builder.summary.iHeatFlow)
        ECAD_TRACE("outtake heat flow: %1%w", builder.summary.oHeatFlow)

        ThermalNetworkSolver<EFloat> solver(*network, settings.threads);
        solver.Solve(settings.iniT, results);

        residual = CalculateResidual(results, prevRes);
        ECAD_TRACE("Residual: %1%, Remain Iteration: %2%", residual, --iteration);
    } while (residual > settings.residual && iteration > 0);
    minT = *std::min_element(results.begin(), results.end());
    maxT = *std::max_element(results.begin(), results.end());
    return true;
}

ECAD_INLINE EGridThermalNetworkTransientSolver::EGridThermalNetworkTransientSolver(const EGridThermalModel & model)
 : EGridThermalNetworkSolver(model)
{
}

ECAD_INLINE bool EGridThermalNetworkTransientSolver::Solve(EFloat & minT, EFloat & maxT)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("grid thermal network transient solve")

    UPtr<ThermalNetwork<EFloat> > network;
    EGridThermalNetworkBuilder builder(m_model);
    std::vector<EFloat> results(m_model.TotalGrids(), settings.iniT);
    auto probs = settings.probs;
    if (probs.empty()) {
        network = builder.Build(results);
        if (nullptr == network) return false;
        ECAD_TRACE("total nodes: %1%", network->Size())
        ECAD_TRACE("intake  heat flow: %1%w", builder.summary.iHeatFlow)
        ECAD_TRACE("outtake heat flow: %1%w", builder.summary.oHeatFlow)
                
        ThermalNetworkSolver<EFloat> solver(*network, settings.threads);
        solver.Solve(settings.iniT, results);

        size_t maxId = std::distance(results.begin(), std::max_element(results.begin(), results.end()));
        ECAD_TRACE("no input probs, will use static simulation hotspot id:%1% as prob!", maxId);
        probs.emplace(maxId);
    }
    if (nullptr == network) network = builder.Build(results);
    
    if (not settings.mor) {
        ECAD_EFFICIENCY_TRACK("impedence origin")
        using TransSolver = ThermalNetworkTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;        
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
        TransSolver solver(*network, settings.iniT, probs);
        generic::thread::ThreadPool pool(settings.threads);
        std::cout << "available threads: " << pool.Threads() << std::endl;
            
        for (size_t i = 0, index = 0; i < cycles.size(); ++i) {
            for (size_t j = 0; j < dutys.size(); ++j) {
                auto cycle = cycles.at(i);
                auto duty = dutys.at(j);
                pool.Submit(
                    [&] {
                        Excitation excitation(cycle, duty);
                        StateType initState(network->Size(), settings.iniT);
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
    else {
        ECAD_EFFICIENCY_TRACK("impedence reduce")
        using TransSolver = ThermalNetworkReducedTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;        
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
        TransSolver solver(*network, settings.iniT, probs);
        generic::thread::ThreadPool pool(settings.threads);
        std::cout << "available threads: " << pool.Threads() << std::endl;            
        for (size_t i = 0, index = 0; i < cycles.size(); ++i) {
            for (size_t j = 0; j < dutys.size(); ++j) {
                auto cycle = cycles.at(i);
                auto duty = dutys.at(j);
                pool.Submit(
                    [&] {
                        Excitation excitation(cycle, duty);
                        StateType initState(network->Size(), settings.iniT);
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
        TransSolver solver(*network, settings.iniT, probs);
        std::ofstream ofs("./orig.txt");
        Recorder recorder(solver, ofs, 0.01);
        StateType initState(solver.StateSize(), settings.iniT);
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
        StateType initT(network->Size(), settings.iniT);
        TransSolver solver(*network, settings.iniT, probs);
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

ECAD_INLINE bool EPrismaThermalNetworkStaticSolver::Solve(EFloat & minT, EFloat & maxT)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("prisma thermal network static solve")

    EPrismaThermalNetworkBuilder builder(m_model);
    std::vector<EFloat> results(m_model.TotalElements(), settings.iniT);

    EFloat residual = maxFloat;
    size_t iteration = m_model.NeedIteration() ? settings.iteration : 1;
    do {
        std::vector<EFloat> prevRes(results);
        auto network = builder.Build(prevRes);
        if (nullptr ==  network) return false;
        ECAD_TRACE("total nodes: %1%", network->Size())
        ECAD_TRACE("intake  heat flow: %1%w", builder.summary.iHeatFlow)
        ECAD_TRACE("outtake heat flow: %1%w", builder.summary.oHeatFlow)

        ThermalNetworkSolver<EFloat> solver(*network, settings.threads);
        solver.Solve(settings.iniT, results);

        residual = CalculateResidual(results, prevRes);
        ECAD_TRACE("Residual: %1%, Remain Iteration: %2%", residual, --iteration);
    } while (residual > settings.residual && iteration > 0);
    minT = *std::min_element(results.begin(), results.end());
    maxT = *std::max_element(results.begin(), results.end());

    if (settings.dumpHotmaps) {
        auto hotmapFile = settings.workDir + ECAD_SEPS + "hotmap.vtk";
        ECAD_TRACE("dump vtk hotmap: %1%", hotmapFile)
        io::GenerateVTKFile(hotmapFile, m_model, &results);
    }
    return true;
}

ECAD_INLINE EPrismaThermalNetworkTransientSolver::EPrismaThermalNetworkTransientSolver(const EPrismaThermalModel & model)
 : EPrismaThermalNetworkSolver(model)
{
}

ECAD_INLINE bool EPrismaThermalNetworkTransientSolver::Solve(EFloat &, EFloat &)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("prisma thermal network transient solve")
    return false;
}

} //namespace ecad::solver
