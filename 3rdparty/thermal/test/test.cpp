#include "thermal/utilities/ThermalNetlistWriter.hpp"
#include "thermal/solver/ThermalNetworkSolver.hpp"
#include <boost/stacktrace.hpp>
#include <iostream>
#include <cassert>
#include <csignal>

void SignalHandler(int signum)
{
    ::signal(signum, SIG_DFL);
    std::cout << boost::stacktrace::stacktrace();
    ::raise(SIGABRT);
}

int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &SignalHandler);
    ::signal(SIGABRT, &SignalHandler);

    using namespace thermal;
    using float_t = double;

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
    solver::ThermalNetworkSolver<float_t> staticSolver(network, 1);
    staticSolver.SetVerbose(1);
    staticSolver.Solve(refT);

    const auto & nodes = network.GetNodes();
    for(size_t i = 0; i < nodes.size(); ++i) {
        std::cout << "node " << i + 1 <<": " << nodes[i].t << std::endl;
    }


    {
        std::vector<size_t> probs{0};
        using TransSolver = solver::ThermalNetworkReducedTransientSolver<float_t>;
        using StateType = typename TransSolver::StateType;

        StateType initState;
        TransSolver transSolver(network, refT);
        transSolver.Solve(initState, float_t{0}, float_t{10}, float_t{0.1});
    }
    return 0;
}