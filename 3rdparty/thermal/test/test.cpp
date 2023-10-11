#include "thermal/solver/ThermalNetworkSolver.hpp"
#include <iostream>
int main(int argc, char * argv[])
{
    using namespace thermal;
    using float_t = double;

    model::ThermalNetwork<float_t> network(3);
    network.SetHF(0, 25);
    network.SetHTC(1, 5);
    network.SetHTC(2, 5);
    network.SetR(0, 1, 1);
    network.SetR(0, 2, 1);
    network.SetR(1, 2, 1);
    network.SetT(1, 25);
    network.SetT(2, 30);
    
    solver::ThermalNetworkSolver<float_t> solver(network, 4);
    solver.Solve(20);

    const auto & nodes = network.GetNodes();
    for(size_t i = 0; i < nodes.size(); ++i) {
        std::cout << "node " << i + 1 <<": " << nodes[i].t << std::endl;
    }

    return 0;
}