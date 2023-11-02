#ifndef THERMAL_SOLVER_THERMALNETWORKSOLVER_HPP
#define THERMAL_SOLVER_THERMALNETWORKSOLVER_HPP
#include "thermal/model/ThermalNetwork.hpp"
#include "generic/math/MathUtility.hpp"
#include "generic/tools/Tools.hpp"
#include <memory>

#include "Eigen/IterativeLinearSolvers"
#include "Eigen/SparseCholesky"
#include "Eigen/SparseLU"
#include "Eigen/Sparse"

namespace thermal {
namespace solver {

using namespace model;

template <typename num_type> 
class ThermalNetworkSolver
{
public:
    explicit ThermalNetworkSolver(ThermalNetwork<num_type> & network, size_t threads = 1)
     : m_threads(threads), m_network(network) {}

    virtual ~ThermalNetworkSolver() = default;

    void SetVerbose(int verbose) { m_verbose = verbose; }

    void Solve(num_type refT) const
    {
        SolveEigen(refT);
    }
    void SolveEigen(num_type refT) const
    {

        ThermalNetworkMatrixBuilder<num_type> builder(m_network);
        size_t size = builder.GetMatrixSize();
        const auto & mnMap = builder.GetMatrixNodeIndicesMap();
        // const auto & nmMap = builder.GetNodeMatrixIndicesMap();

        Eigen::SparseMatrix<double> spMat(size, size);
        spMat.reserve(Eigen::VectorXi::Constant(size, 10));

        auto diagCoeffs = std::unique_ptr<std::vector<num_type> >(new std::vector<num_type>{}) ;
        auto edges = std::unique_ptr<std::list<typename ThermalNetwork<num_type>::Edge> >(new std::list<typename ThermalNetwork<num_type>::Edge>{});

        builder.GetCoeffs(*edges);
        builder.GetDiagCoeffs(*diagCoeffs);
        for(size_t i = 0; i < size; ++i)
            spMat.insert(i, i) = (*diagCoeffs)[i];

        for(const auto & edge : *edges)
            spMat.insert(edge.x, edge.y) = edge.r;
        
        edges.reset();
        diagCoeffs.reset();
        spMat.makeCompressed();

        Eigen::VectorXd b(size);
        for(size_t i = 0; i < size; ++i)
            b[i] = builder.GetRhs(i, refT);
        
        if (m_verbose) {
            std::cout << "G:\n" << spMat << std::endl;
            std::cout << "u:\n" << b << std::endl;
        }

        Eigen::setNbThreads(std::max<size_t>(1, m_threads));
        
        Eigen::VectorXd x;
        //iterator
        Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower|Eigen::Upper> solver;
        solver.compute(spMat);
        x = solver.solve(b);
        std::cout << "#iterations:     " << solver.iterations() << std::endl;
        std::cout << "estimated error: " << solver.error()      << std::endl;

        //direct
        // Eigen::SparseLU<Eigen::SparseMatrix<double> > solver;
        // // Eigen::SimplicialCholesky<Eigen::SparseMatrix<double> > solver;
        // solver.analyzePattern(spMat);
        // solver.factorize(spMat);
        // x = solver.solve(b); 

        auto & nodes = m_network.GetNodes();
        for(size_t i = 0; i < size; ++i)
            nodes[mnMap.at(i)].t = x[i];
    }
private:
    int m_verbose = 0;
    size_t m_threads = 1;
    ThermalNetwork<num_type> & m_network;
};

}//namespace solver
}//namespace thermal
#endif//THERMAL_SOLVER_THERMALNETWORKSOLVER_HPP