#include "thermal/solver/ThermalNetworkSolver.hpp"
#include "thermal/utilities/ThermalNetlistWriter.hpp"
#include <iostream>
int main(int argc, char * argv[])
{
    using namespace thermal;
    using float_t = double;

    model::ThermalNetwork<float_t> network(3);
    network.SetHTC(1, 2);
    network.SetHTC(2, 2);
    network.SetR(0, 1, 1);
    network.SetR(0, 2, 1);
    network.SetR(1, 2, 1);
    network.SetHF(0, 5);
    network.SetC(0, 1);
    network.SetC(1, 1);
    network.SetC(2, 1);

    utils::ThermalNetlistWriter<float_t> netlistWriter(network);
    netlistWriter.WriteSpiceNetlist(std::filesystem::path(__FILE__).parent_path().string() + "/test.sp");

    solver::ThermalNetworkSolver<float_t> solver(network, 4);
    solver.SetVerbose(1);
    solver.Solve(20);

    const auto & nodes = network.GetNodes();
    for(size_t i = 0; i < nodes.size(); ++i) {
        std::cout << "node " << i + 1 <<": " << nodes[i].t << std::endl;
    }

    return 0;
}