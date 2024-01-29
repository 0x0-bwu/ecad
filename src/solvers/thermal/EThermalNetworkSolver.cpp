#include "EThermalNetworkSolver.h"
#include "solvers/thermal/network/utils/BoundaryNodeReduction.hpp"
#include "solvers/thermal/network/utils/ThermalNetlistWriter.hpp"
#include "solvers/thermal/network/ThermalNetworkSolver.hpp"
#include "models/thermal/traits/EThermalModelTraits.hpp"
#include "models/thermal/io/EPrismaThermalModelIO.h"
#include "utils/EPrismaThermalNetworkBuilder.h"
#include "utils/EGridThermalNetworkBuilder.h"
#include "generic/thread/ThreadPool.hpp"
#include "generic/tools/Format.hpp"
namespace ecad::solver {

using namespace ecad::model;

ECAD_INLINE EFloat CalculateResidual(const std::vector<EFloat> & v1, const std::vector<EFloat> & v2, bool maximumRes)
{
    ECAD_ASSERT(v1.size() == v2.size());
    EFloat residual = 0;
    size_t size = v1.size();
    for(size_t i = 0; i < size; ++i) {
        if (maximumRes) residual = std::max<EFloat>(std::fabs(v1[i] - v2[i]), residual);
        else residual += std::fabs(v1[i] - v2[i]) / size;
    }
    return residual;
}

template <typename ThermalNetworkBuilder>
ECAD_INLINE bool EThermalNetworkStaticSolver::Solve(const typename ThermalNetworkBuilder::ModelType & model, std::vector<EFloat> & results) const
{
    auto envT = settings.envTemperature.inKelvins();
    ThermalNetworkBuilder builder(model);
    using Model = typename ThermalNetworkBuilder::ModelType;
    results.assign(traits::EThermalModelTraits<Model>::Size(model), envT);

    EFloat residual = maxFloat;
    size_t iteration = traits::EThermalModelTraits<Model>::NeedIteration(model) ? settings.iteration : 1;
    do {
        std::vector<EFloat> prevRes(results);
        auto network = builder.Build(prevRes);
        if (nullptr == network) return false;
        ECAD_TRACE("total nodes: %1%", network->Size())
        ECAD_TRACE("intake  heat flow: %1%w", builder.summary.iHeatFlow)
        ECAD_TRACE("outtake heat flow: %1%w", builder.summary.oHeatFlow)

        using namespace thermal::solver;
        ThermalNetworkSolver<EFloat> solver(*network, settings.threads);
        solver.Solve(envT, results);

        residual = CalculateResidual(results, prevRes, settings.maximumRes);
        ECAD_TRACE("Residual: %1%, Remain Iteration: %2%", residual, --iteration)
        ECAD_TRACE("Max T: %1%C, ", ETemperature::Kelvins2Celsius(*std::max_element(results.begin(), results.end())))
    } while (residual > settings.residual && iteration > 0);

    if (settings.envTemperature.unit == ETemperatureUnit::Celsius) 
        std::for_each(results.begin(), results.end(), [](auto & t){ t = ETemperature::Kelvins2Celsius(t); });

    return true;   
}

ECAD_INLINE template bool EThermalNetworkStaticSolver::Solve<EGridThermalNetworkBuilder>(const EGridThermalModel & model, std::vector<EFloat> & results) const;
ECAD_INLINE template bool EThermalNetworkStaticSolver::Solve<EPrismaThermalNetworkBuilder>(const EPrismaThermalModel & model, std::vector<EFloat> & results) const;

template <typename ThermalNetworkBuilder>
ECAD_INLINE bool EThermalNetworkTransientSolver::Solve(const typename ThermalNetworkBuilder::ModelType & model, EFloat & minT, EFloat & maxT) const
{
    using namespace thermal::solver;

    minT = maxFloat;
    maxT = -maxFloat;
    auto envT = settings.envTemperature.inKelvins();
    ThermalNetworkBuilder builder(model);
    UPtr<ThermalNetwork<EFloat> > network;
    using Model = typename ThermalNetworkBuilder::ModelType;
    std::vector<EFloat> results(traits::EThermalModelTraits<Model>::Size(model), envT);
    auto probs = settings.probs;
    if (probs.empty()) {
        network = builder.Build(results);
        if (nullptr == network) return false;
        ECAD_TRACE("total nodes: %1%", network->Size())
        ECAD_TRACE("intake  heat flow: %1%w", builder.summary.iHeatFlow)
        ECAD_TRACE("outtake heat flow: %1%w", builder.summary.oHeatFlow)
                
        ThermalNetworkSolver<EFloat> solver(*network, settings.threads);
        solver.Solve(envT, results);

        size_t maxId = std::distance(results.begin(), std::max_element(results.begin(), results.end()));
        ECAD_TRACE("no input probs, will use static simulation hotspot id:%1% as prob!", maxId);
        probs.emplace(maxId);
    }
    if (nullptr == network) network = builder.Build(results);
    
    size_t steps;
    Samples<EFloat> samples;
    if (not settings.mor) {
        ECAD_EFFICIENCY_TRACK("transient orig")
        using TransSolver = ThermalNetworkTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Sampler = typename TransSolver::Sampler;
        TransSolver solver(*network, envT, probs);
        Sampler sampler(solver, samples, settings.samplingWindow, settings.minSamplingInterval, settings.verbose);
        StateType initState(solver.StateSize(), envT);
        ECAD_TRACE("duration: %1%, step: %2%, abs error: %3%, rel error: %4%", settings.duration, settings.step, settings.absoluteError, settings.relativeError);
        if (settings.adaptive)
            steps = solver.SolveAdaptive(initState, EFloat{0}, settings.duration, settings.step, settings.absoluteError, settings.relativeError, std::move(sampler), settings.excitation);
        else steps = solver.Solve(initState, EFloat{0}, settings.duration, settings.step, std::move(sampler), settings.excitation);
    }
    else {
        ECAD_EFFICIENCY_TRACK("transient mor")
        using TransSolver = ThermalNetworkReducedTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Sampler = typename TransSolver::Sampler;
        StateType initT(network->Size(), envT);
        TransSolver solver(*network, envT, probs);
        Sampler sampler(solver, samples, settings.samplingWindow, settings.minSamplingInterval, settings.verbose);
        if (settings.adaptive)
            steps = solver.SolveAdaptive(initT, EFloat{0}, settings.duration, settings.step, settings.absoluteError, settings.relativeError, std::move(sampler), settings.excitation);
        else steps = solver.Solve(initT, EFloat{0}, settings.duration, settings.step, std::move(sampler), settings.excitation);
    }
    for (auto & sample : samples) {
        if (settings.envTemperature.unit == ETemperatureUnit::Celsius)
            std::for_each(sample.begin(), sample.end(), [](auto & t){ t = ETemperature::Kelvins2Celsius(t); });
       
        minT = std::min(minT, *std::min_element(++sample.begin(), sample.end()));
        maxT = std::max(maxT, *std::max_element(++sample.begin(), sample.end()));
    }
    if (settings.dumpRawData) {
        auto filename = settings.workDir + ECAD_SEPS + "trans.txt";
        std::ofstream out(filename);
        if (out.is_open()) {
            for (const auto & sample : samples)
                out << generic::fmt::Fmt2Str(sample, ",") << ECAD_EOL;
            out.close();
        }
    }
    return steps > 0;  
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
    ECAD_EFFICIENCY_TRACK("grid thermal network static solve")

    std::vector<EFloat> results;
    auto res = EThermalNetworkStaticSolver::template Solve<EGridThermalNetworkBuilder>(m_model, results);
    minT = *std::min_element(results.begin(), results.end());
    maxT = *std::max_element(results.begin(), results.end());

    if (not settings.workDir.empty() && settings.dumpHotmaps) {        

        auto modelSize = m_model.ModelSize();
        auto htMap = std::unique_ptr<ELayoutMetalFraction>(new ELayoutMetalFraction);
        for (size_t z = 0; z < modelSize.z; ++z)
            htMap->push_back(std::make_shared<ELayerMetalFraction>(modelSize.x, modelSize.y));

        for (size_t i = 0; i < results.size(); ++i) {
            auto gridIndex = m_model.GetGridIndex(i);
            auto lyrHtMap = htMap->at(gridIndex.z);
            (*lyrHtMap)(gridIndex.x, gridIndex.y) = results[i];
        }

        using ValueType = typename ELayerMetalFraction::ResultType;
        for (size_t index = 0; index < htMap->size(); ++index) {
            auto lyr = htMap->at(index);
            auto min = lyr->MaxOccupancy(std::less<ValueType>());
            auto max = lyr->MaxOccupancy(std::greater<ValueType>());
            auto range = max - min;
            auto rgbaFunc = [&min, &range](ValueType d) {
                int r, g, b, a = 255;
                generic::color::RGBFromScalar((d - min) / range, r, g, b);
                return std::make_tuple(r, g, b, a);
            };
            ECAD_TRACE("layer: %1%, maxT: %2%, minT: %3%", index + 1, max, min)
            std::string filepng = settings.workDir + ECAD_SEPS + std::to_string(index) + ".png";
            lyr->WriteImgProfile(filepng, rgbaFunc);
        }
    }
    return res;
}

ECAD_INLINE EGridThermalNetworkTransientSolver::EGridThermalNetworkTransientSolver(const EGridThermalModel & model)
 : EGridThermalNetworkSolver(model)
{
}

ECAD_INLINE bool EGridThermalNetworkTransientSolver::Solve(EFloat & minT, EFloat & maxT)
{
    ECAD_EFFICIENCY_TRACK("grid thermal network transient solve")
    return EThermalNetworkTransientSolver::template Solve<EGridThermalNetworkBuilder>(m_model, minT, maxT);
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
    ECAD_EFFICIENCY_TRACK("prisma thermal network static solve")

    std::vector<EFloat> results;
    auto res = EThermalNetworkStaticSolver::template Solve<EPrismaThermalNetworkBuilder>(m_model, results);
    minT = *std::min_element(results.begin(), results.end());
    maxT = *std::max_element(results.begin(), results.end());

    if (settings.dumpHotmaps) {
        auto hotmapFile = settings.workDir + ECAD_SEPS + "hotmap.vtk";
        ECAD_TRACE("dump vtk hotmap: %1%", hotmapFile)
        res &= io::GenerateVTKFile(hotmapFile, m_model, &results);
    }
    return res;
}

ECAD_INLINE EPrismaThermalNetworkTransientSolver::EPrismaThermalNetworkTransientSolver(const EPrismaThermalModel & model)
 : EPrismaThermalNetworkSolver(model)
{
}

ECAD_INLINE bool EPrismaThermalNetworkTransientSolver::Solve(EFloat & minT, EFloat & maxT)
{
    ECAD_EFFICIENCY_TRACK("prisma thermal network transient solve")
    return EThermalNetworkTransientSolver::template Solve<EPrismaThermalNetworkBuilder>(m_model, minT, maxT);
}

} //namespace ecad::solver
