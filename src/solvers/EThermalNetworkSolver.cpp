#include "solvers/EThermalNetworkSolver.h"
#include "thermal/utilities/BoundaryNodeReduction.hpp"
#include "thermal/utilities/ThermalNetlistWriter.hpp"
#include "thermal/solver/ThermalNetworkSolver.hpp"
#include "utilities/EThermalNetworkBuilder.h"
#include "generic/thread/ThreadPool.hpp"
#include "generic/tools/Format.hpp"
#include "EDataMgr.h"
namespace ecad::esolver {

using namespace emodel;

ECAD_INLINE ESimVal CalculateResidual(const std::vector<ESimVal> & v1, const std::vector<ESimVal> & v2)
{
    ECAD_ASSERT(v1.size() == v2.size());
    ESimVal residual = 0;
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

ECAD_INLINE bool EGridThermalNetworkStaticSolver::Solve(ESimVal refT, std::vector<ESimVal> & results)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("thermal network static solve")

    results.assign(m_model.TotalGrids(), refT);
    bool needIteration = m_model.NeedIteration();
    if (m_settings.iteration > 0 && math::GT<EValue>(m_settings.residual, 0)) {

        EValue residual = m_settings.residual;
        size_t iteration = m_settings.iteration;
        EGridThermalNetworkBuilder builder(m_model);
        do {
            std::vector<ESimVal> lastRes(results);
            auto network = builder.Build(lastRes);
            if (nullptr == network) return false;

            if (not m_settings.spiceFile.empty()) {
                auto spiceWriter = ThermalNetlistWriter(*network);
                spiceWriter.SetReferenceTemperature(refT);
                auto res = spiceWriter.WriteSpiceNetlist(m_settings.spiceFile);
                if (res) std::cout << "write out spice file " << m_settings.spiceFile << " successfully!" << std::endl;
            }
            std::cout << "total nodes: " << network->Size() << std::endl;
            std::cout << "intake  heat flow: " << builder.summary.iHeatFlow << "w" << std::endl;
            std::cout << "outtake heat flow: " << builder.summary.oHeatFlow << "w" << std::endl;
            
            size_t threads = EDataMgr::Instance().DefaultThreads();
            ThermalNetworkSolver<ESimVal> solver(*network, threads);
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

ECAD_INLINE bool EGridThermalNetworkTransientSolver::Solve(ESimVal refT, std::vector<ESimVal> & results)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("thermal network transient solve")

    results.assign(m_model.TotalGrids(), refT);
    EGridThermalNetworkBuilder builder(m_model);
    auto network = builder.Build(results);
    if (nullptr == network) return false;
    std::cout << "total nodes: " << network->Size() << std::endl;
    std::cout << "intake  heat flow: " << builder.summary.iHeatFlow << "w" << std::endl;
    std::cout << "outtake heat flow: " << builder.summary.oHeatFlow << "w" << std::endl;
            
    size_t threads = EDataMgr::Instance().DefaultThreads();
    ThermalNetworkSolver<ESimVal> solver(*network, threads);
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
        using TransSolver = ThermalNetworkTransientSolver<ESimVal>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;
        size_t threads = EDataMgr::Instance().DefaultThreads();
        
        struct Excitation
        {
            ESimVal cycle, duty;
            Excitation(ESimVal cycle, ESimVal duty = 0.5) : cycle(cycle), duty(duty * cycle) {}
            ESimVal operator() (ESimVal t) const
            {
                return std::fmod(t, cycle) < duty ? 1 : 0.1;
            }
        };

        std::vector<ESimVal> cycles;
        for (size_t i = 0; i < 12; ++i)
            cycles.emplace_back(std::pow(10, 0.5 * i - 2.5));
        std::vector<ESimVal> dutys {0.01, 0.02, 0.05, 0.1, 0.3, 0.5};
        TransSolver solver(*network, refT, probs);
        generic::thread::ThreadPool pool(threads);
        std::cout << "available threads: " << pool.Threads() << std::endl;
    
        std::vector<std::ofstream> oftreams;
        for (size_t i = 0; i < cycles.size(); ++i)
            for (size_t j = 0; j < dutys.size(); ++j)
                oftreams.emplace_back(std::ofstream(fmt::Fmt2Str("./origin_%1%_%2%.txt", i, j)));
                
        for (size_t i = 0, index = 0; i < cycles.size(); ++i) {
            for (size_t j = 0; j < dutys.size(); ++j) {
                auto cycle = cycles.at(i);
                auto duty = dutys.at(j);
                pool.Submit(
                    [cycle, duty, maxId, refT, index, &solver, &oftreams, &network] {
                        Excitation excitation(cycle, duty);
                        StateType initState(network->Size(), refT);
                        ESimVal t0 = 0, t1 = std::min<ESimVal>(std::max<ESimVal>(10, cycle * 50), 20), dt = 0.5 * cycle * duty;
                        solver.Solve(initState, t0, t1, dt, Recorder(solver, oftreams[index], dt), excitation);
                        oftreams[index].close();
                        std::cout << "done with c: " << cycle << ", d: " << duty << std::endl;
                    }
                );
                index++;
            }
        }
    }  
    bool reduceBounds = true;
    if (reduceBounds) {
        auto minT = network->MinT();
        auto maxT = network->MaxT();
        BoundaryNodeReduction bnReduction(*network);
        bnReduction.Reduce(minT, minT + (maxT - minT) * 0.3, 3);
    }
    if (false) {
        ECAD_EFFICIENCY_TRACK("impedence reduce")
        using TransSolver = ThermalNetworkReducedTransientSolver<ESimVal>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;
        size_t threads = EDataMgr::Instance().DefaultThreads();
        
        struct Excitation
        {
            ESimVal cycle, duty;
            Excitation(ESimVal cycle, ESimVal duty = 0.5) : cycle(cycle), duty(duty * cycle) {}
            ESimVal operator() (ESimVal t) const
            {
                return std::fmod(t, cycle) < duty ? 1 : 0.1;
            }
        };

        std::vector<ESimVal> cycles;
        for (size_t i = 0; i < 12; ++i)
            cycles.emplace_back(std::pow(10, 0.5 * i - 2.5));
        std::vector<ESimVal> dutys {0.01, 0.02, 0.05, 0.1, 0.3, 0.5};
        TransSolver solver(*network, refT, probs);
        generic::thread::ThreadPool pool(threads);
        std::cout << "available threads: " << pool.Threads() << std::endl;
    
        std::vector<std::ofstream> oftreams;
        for (size_t i = 0; i < cycles.size(); ++i)
            for (size_t j = 0; j < dutys.size(); ++j)
                oftreams.emplace_back(std::ofstream(fmt::Fmt2Str("./reduce_%1%_%2%.txt", i, j)));
                
        for (size_t i = 0, index = 0; i < cycles.size(); ++i) {
            for (size_t j = 0; j < dutys.size(); ++j) {
                auto cycle = cycles.at(i);
                auto duty = dutys.at(j);
                pool.Submit(
                    [cycle, duty, maxId, refT, index, &solver, &oftreams, &network] {
                        Excitation excitation(cycle, duty);
                        StateType initState(network->Size(), refT);
                        ESimVal t0 = 0, t1 = std::min<ESimVal>(std::max<ESimVal>(10, cycle * 50), 20), dt = 0.5 * cycle * duty;
                        solver.Solve(initState, t0, t1, dt, Recorder(solver, oftreams[index], dt), excitation);
                        oftreams[index].close();
                        std::cout << "done with c: " << cycle << ", d: " << duty << std::endl;
                    }
                );
                index++;
            }
        }
    }  

    if (true) {
        ECAD_EFFICIENCY_TRACK("transient mor")
        using TransSolver = ThermalNetworkReducedTransientSolver<ESimVal>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;
        StateType initT(network->Size(), refT);
        TransSolver solver(*network, refT, probs);
        std::ofstream ofs("./mor.txt");
        Recorder recorder(solver, ofs, 0.1);
        solver.Solve(initT, float_t{0}, float_t{10}, float_t{0.1}, std::move(recorder));
    }   
    if (true) {
        ECAD_EFFICIENCY_TRACK("transient orig")
        using TransSolver = ThermalNetworkTransientSolver<ESimVal>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;
        TransSolver solver(*network, refT, probs);
        std::ofstream ofs("./orig.txt");
        Recorder recorder(solver, ofs, 0.1);
        StateType initState(solver.StateSize(), refT);
        solver.Solve(initState, float_t{0}, float_t{10}, float_t{0.1}, std::move(recorder));
    }
    return true;
}
} //namespace ecad::esolver
