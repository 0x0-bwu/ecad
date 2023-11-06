#include "thermal/solver/ThermalNetworkSolver.hpp"
#include "thermal/utilities/ThermalNetlistWriter.hpp"
#include "thermal/solver/ThermalModelReduction.hpp"
#include <iostream>

int main(int argc, char * argv[])
{
    using namespace thermal;
    using float_t = float;

    model::ThermalNetwork<float_t> network(3);
    // network.SetHTC(1, 2);
    network.SetHTC(2, 2);
    network.SetR(0, 1, 1);
    network.SetR(0, 2, 1);
    network.SetR(1, 2, 1);
    network.SetHF(0, 5);
    network.SetC(0, 1);
    network.SetC(1, 1);
    network.SetC(2, 1);

    utils::ThermalNetlistWriter<float_t> writer(network);
    writer.WriteSpiceNetlist(std::filesystem::path(__FILE__).parent_path().string() + "/test.sp");


    float_t refT = 25;
    size_t threads{1};
    solver::ThermalNetworkSolver<float_t> staticSolver(network, threads);
    staticSolver.SetVerbose(1);
    staticSolver.Solve(refT);

    const auto & nodes = network.GetNodes();
    for(size_t i = 0; i < nodes.size(); ++i) {
        std::cout << "node " << i + 1 <<": " << nodes[i].t << std::endl;
    }

    {
        std::vector<size_t> probs{0};
        using TransSolver = solver::ThermalNetworkTransientSolver<float_t>;
        using StateType = typename TransSolver::StateType;
        auto in = typename TransSolver::Input(network, refT, threads);
        auto recorder = typename TransSolver::Recorder(std::cout, probs, 0.01);

        StateType initT(nodes.size(), refT);
        using namespace boost::numeric;
        using ErrorStepperType = odeint::runge_kutta_cash_karp54<std::vector<float_t> >;
        odeint::integrate_adaptive(
            odeint::make_controlled(1e-12, 1e-10,ErrorStepperType{}),
            TransSolver(&in), initT, 0.0, 10.0, 0.01, recorder);
    }

    {
        std::vector<size_t> probs{0};
        using TransSolver = solver::ThermalNetworkReducedTransientSolver<float_t>;
        using StateType = typename TransSolver::StateType;
        auto in = typename TransSolver::Input(network, refT, 2, threads, 1);
        auto recorder = typename TransSolver::Recorder(std::cout, &in, probs, 0.01);
        StateType state(in.StateSize());
        StateType initT(network.Size(), refT);
        Eigen::Map<const Eigen::Matrix<float_t, Eigen::Dynamic, 1>> init(initT.data(), initT.size(), 1);
        Eigen::Map<Eigen::Matrix<float_t, Eigen::Dynamic, 1>> result(state.data(), state.size(), 1);
        result = in.x.transpose() * init;
        std::cout << "init State: \n" << result << std::endl;//wbtest
        using namespace boost::numeric;
        using ErrorStepperType = odeint::runge_kutta_cash_karp54<std::vector<float_t> >;
        odeint::integrate_adaptive(
            odeint::make_controlled(1e-12, 1e-10,ErrorStepperType{}),
            TransSolver(&in), initT, 0.0, 10.0, 0.01, recorder);
    }
    return 0;
}