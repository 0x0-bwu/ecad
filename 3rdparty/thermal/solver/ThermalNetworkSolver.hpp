#pragma once
#include "thermal/model/ThermalNetwork.hpp"
#include "generic/math/MathUtility.hpp"
#include "generic/tools/Tools.hpp"

#include <boost/functional/hash.hpp>
#include <boost/numeric/odeint.hpp>
#include <memory>

#include "Eigen/IterativeLinearSolvers"
#include "Eigen/SparseCholesky"
#include "Eigen/SparseLU"
#include "Eigen/Sparse"

namespace thermal {
namespace solver {

using namespace model;

template <typename num_type>
class ThermalSpVector
{
public:
    num_type operator[] (size_t index) const
    {
        if (auto iter = m_indexMap.find(index); iter != m_indexMap.cend())
            return iter->second;
        return 0;
    }
    
    void emplace(size_t index, num_type value)
    {
        m_indexMap.emplace(index, value);
    }
private:
    std::unordered_map<size_t, num_type> m_indexMap;
};
template <typename num_type>
class ThermalSpMatrix
{
    using Index2D = std::array<size_t, 2>;
    using IndexMap = std::unordered_map<Index2D, num_type, boost::hash<Index2D> >; 
public:
    num_type operator() (size_t x, size_t y) const
    {
        if (x > y) std::swap(x, y);
        if (auto iter = m_indexMap.find({x, y}); iter != m_indexMap.cend())
            return iter->second;
        return 0;
    }

    void emplace(size_t x, size_t y, num_type value)
    {
        if (x > y) std::swap(x, y);
        m_indexMap.emplace(Index2D{x, y}, value);
    }

private:
    IndexMap m_indexMap;
};

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

template <typename num_type>
class EigenMatrixBuilder
{
public:
    using Matrix = Eigen::SparseMatrix<num_type>;
    explicit EigenMatrixBuilder(const ThermalNetwork<num_type> & network)
     : m_network(network)
    {
    }
    
    ThermalSpMatrix<num_type> GetMatrixG() const
    {
        auto size = m_network.Size();
        const auto & nodes = m_network.GetNodes();
    
        ThermalSpMatrix<num_type> spMat;
        for (size_t i = 0; i < size; ++i) {
            num_type diag = 0;
            const auto & node = nodes.at(i);
            for (size_t j = 0; j < node.ns.size(); ++j) {
                auto g = 1 / node.rs.at(j);
                spMat.emplace(i, node.ns.at(j), -g);
                diag += g;
            }

            if (node.htc != 0) diag += node.htc;
            if (diag > 0) spMat.emplace(i, i, diag);
        }
        return spMat;
    }

    ThermalSpVector<num_type> GetRhs(num_type refT) const
    {
        ThermalSpVector<num_type> spVec;
        const auto & nodes = m_network.GetNodes();
        for (size_t i = 0; i < nodes.size(); ++i) {
            const auto & node = nodes.at(i);
            auto res = node.hf + node.htc * refT;
            if (res != 0) spVec.emplace(i, res);
        }
        return spVec;
    }

    // std::pair<Matrix, Matrix> GetMatrices() const //G, C
    // {
    //     auto size = m_network.Size();
    //     const auto & nodes = m_network.GetNodes();
    //     std::vector<Eigen::Triplet<num_type> > tG, tC;
    //     for (size_t i = 0; i < size; ++i) {
    //         num_type diag = 0;
    //         const auto & node = nodes.at(i);
    //         for (size_t j = 0; j < node.ns.size(); ++j) {
    //             auto g = 1 / node.rs.at(j);
    //             tG.emplace_back(i, node.ns.at(j), -g);
    //             diag += g;
    //         }

    //         if (node.htc != 0) diag += node.htc;
    //         if (diag > 0) tG.emplace_back(i, i, diag);
    //         if (node.c > 0) tC.emplace_back(i, i, node.c);
    //     }

    //     Eigen::SparseMatrix<double> G(size, size), C(size, size);
    //     G.setFromTriplets(tG.begin(), tG.end());
    //     C.setFromTriplets(tC.begin(), tC.end());
    //     return {G, C};
    // }  
private:
    const ThermalNetwork<num_type> & m_network;
};

template <typename num_type> 
class ThermalNetworkTransientSolver
{
public:
    using StateType = std::vector<num_type>;
    using Matrix = typename EigenMatrixBuilder<num_type>::Matrix;
   
    struct Recorder
    { 
        size_t maxId = 0; 
        Recorder(size_t id) : maxId(id) {}
        void operator() (const StateType & x, num_type t)
        {
            std::cout << "t: " << t << ',';
            std::cout << "max: " << x.at(maxId) << std::endl;
        }
    };
    ThermalNetworkTransientSolver(const ThermalNetwork<num_type> & network, num_type refT, size_t threads = 1)
     : m_refT(refT), m_threads(threads), m_network(network)
    {
#if defined(_OPENMP)
    omp_set_num_threads(threads);
#endif
        EigenMatrixBuilder<num_type> builder(m_network);
        m_G = builder.GetMatrixG();
        m_Rhs = builder.GetRhs(refT);
    }

    virtual ~ThermalNetworkTransientSolver() = default;

    void operator() (const StateType  x, StateType & dxdt, num_type t)
    {
        #pragma omp parallel for
        for (size_t i = 0; i < dxdt.size(); ++i) {
            dxdt[i] = 0;
            #pragma omp parallel for
            for (size_t j = 0; j < x.size(); ++j) {
                dxdt[i] += G(i, j) * x.at(j);
            }
            dxdt[i] *= -1 / C(i);
            dxdt[i] += Rhs(i) / C(i);
        }
    }

private:
    num_type G(size_t i, size_t j) const
    {
        // const auto & node = m_network.GetNodes().at(i);
        // if (i == j) {
        //     num_type diag = 0;
        //     for (size_t n = 0; n < node.ns.size(); ++n)
        //         diag += 1 / node.rs.at(n);

        //     if (node.htc != 0) diag += node.htc;
        //     return diag;
        // }
        // else {
        //     for (size_t n = 0; n < node.ns.size(); ++n)
        //         if (node.ns.at(n) == j) return -1 / node.rs.at(n);
        // }
        // return 0;
        return m_G(i, j);
    }

    num_type C(size_t i) const
    {
        return m_network[i].c;
    }

    num_type Rhs(size_t i) const
    {
        // const auto & node = m_network[i];
        // return node.hf + node.htc * m_refT;
        return m_Rhs[i];
    }

private:

    int m_verbose = 0;
    num_type m_refT = 0;
    size_t m_threads = 1;
    ThermalSpMatrix<num_type> m_G;
    ThermalSpVector<num_type> m_Rhs;
    const ThermalNetwork<num_type> & m_network;
};

}//namespace solver
}//namespace thermal