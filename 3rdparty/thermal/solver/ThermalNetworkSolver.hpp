#pragma once
#include "thermal/model/ThermalNetwork.hpp"
#include "generic/thread/ThreadPool.hpp" 
#include "generic/math/MathUtility.hpp"
#include "generic/tools/Tools.hpp"

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/functional/hash.hpp>
#include <boost/numeric/odeint.hpp>
#include <memory>

#include "Eigen/IterativeLinearSolvers"
#include "Eigen/SparseCholesky"
#include "Eigen/SparseLU"
#include "Eigen/Sparse"

#include "ThermalModelReduction.hpp"

namespace thermal {
namespace solver {

using namespace model;
using namespace generic;

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
class ThermalMatrixBuilder
{
public:
    using Matrix = Eigen::SparseMatrix<num_type>;
    explicit ThermalMatrixBuilder(const ThermalNetwork<num_type> & network)
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

    std::tuple<Matrix, Matrix, Matrix> GetMatrices(num_type refT, std::vector<num_type> & rhs) const //G, C, B
    {
        auto size = m_network.Size();
        const auto & nodes = m_network.GetNodes();
        size_t colIdex = 0;
        std::vector<Eigen::Triplet<num_type> > tG, tC, tB;
        for (size_t i = 0; i < size; ++i) {
            num_type diag = 0;
            const auto & node = nodes.at(i);
            for (size_t j = 0; j < node.ns.size(); ++j) {
                auto g = 1 / node.rs.at(j);
                tG.emplace_back(i, node.ns.at(j), -g);
                diag += g;
            }

            if (node.htc != 0) diag += node.htc;
            if (diag > 0) tG.emplace_back(i, i, diag);
            if (node.c > 0) tC.emplace_back(i, i, node.c);
            if (auto q = node.hf + node.htc * refT; q != 0) {
                tB.emplace_back(i, colIdex++, 1);
                rhs.emplace_back(q);
            }
        }

        Eigen::SparseMatrix<num_type> G(size, size), C(size, size), B(size, rhs.size());
        G.setFromTriplets(tG.begin(), tG.end());
        C.setFromTriplets(tC.begin(), tC.end());
        B.setFromTriplets(tB.begin(), tB.end());
        return {G, C, B};
    }  
private:
    const ThermalNetwork<num_type> & m_network;
};

template <typename num_type> 
class ThermalNetworkTransientSolver
{
public:
    // using StateType = typename boost::numeric::ublas::vector<num_type>;
    using StateType = std::vector<num_type>;

    struct Recorder
    { 
        num_type prev{0};
        num_type count{0};
        num_type interval;
        std::ostream & os;
        const std::vector<size_t> & probs;
        Recorder(std::ostream & os, const std::vector<size_t> & probs, num_type interval)
         : interval(interval), os(os), probs(probs) {}
        void operator() (const StateType & x, num_type t)
        {
            if (count += t - prev; count > interval) {
                os << t;
                for (auto p : probs)
                    os << ',' << x[p];
                os << GENERIC_DEFAULT_EOL;
                count = 0;
            }
            prev = t;
        }
    };
    struct Input
    {
        num_type refT = 25;
        size_t threads = 1;
        ThermalSpMatrix<num_type> G;
        ThermalSpVector<num_type> Rhs;
        const ThermalNetwork<num_type> & network;
        std::unique_ptr<thread::ThreadPool> pool;
        
        Input(const ThermalNetwork<num_type> & network, num_type refT, size_t threads)
         : refT(refT), threads(threads), network(network)
        {
            pool.reset(new thread::ThreadPool(threads));
            ThermalMatrixBuilder<num_type> builder(network);
            G = builder.GetMatrixG();
            Rhs = builder.GetRhs(refT);
        }

        num_type C(size_t i) const { return network[i].c; }
    };
    struct Jacobi
    {
        using VectorType = typename ThermalNetworkTransientSolver::StateType;
        using MatrixType = boost::numeric::ublas::matrix<num_type>;
        const Input * in;
        explicit Jacobi(const Input * in) : in(in) {}

        void operator() (const VectorType & /* x */ , MatrixType &J , const num_type & /* t */ , VectorType &dfdt)
        {
            for(size_t i = 0; i < J.size1(); ++i) {
                for(size_t j = 0; j < J.size2(); ++j)
                    J(i, j) = -1 * G(i, j) / C(i);
            }
            for(size_t i = 0; i < dfdt.size(); ++i)
                dfdt[i] = 0;

        };
        num_type G(size_t i, size_t j) const { return (in->G)(i, j); }
        num_type C(size_t i) const { return in->C(i); }
    };
    const Input * in;
    ThermalNetworkTransientSolver(const Input * in) : in(in) {}
    virtual ~ThermalNetworkTransientSolver() = default;
    
    // void operator() (const StateType  x, StateType & dxdt, num_type t)
    // {
    //     // #pragma omp parallel for
    //     for (size_t i = 0; i < dxdt.size(); ++i) {
    //         dxdt[i] = 0;
    //         // #pragma omp parallel for
    //         for (size_t j = 0; j < x.size(); ++j) {
    //             dxdt[i] += G(i, j) * x.at(j);
    //         }
    //         dxdt[i] *= -1 / C(i);
    //         dxdt[i] += Rhs(i) / C(i);
    //     }
    // }

    void operator() (const StateType & x, StateType & dxdt, num_type t)
    {
        if (in->threads == 1)
            return UpdateDxDt(x, dxdt, t, 0, dxdt.size());

        size_t begin = 0;
        size_t size = dxdt.size();
        size_t blocks = in->threads;
        size_t blockSize = size / blocks;
        for (size_t i = 0; i < blocks && blockSize > 0; ++i) {
            size_t end = begin + blockSize;
            in->pool->Submit(std::bind(&ThermalNetworkTransientSolver<num_type>::UpdateDxDt, this, std::ref(x), std::ref(dxdt), t, begin, end));
            begin = end;
        }
        size_t end = size;
        if (begin != end)
            in->pool->Submit(std::bind(&ThermalNetworkTransientSolver<num_type>::UpdateDxDt, this, std::ref(x), std::ref(dxdt), t, begin, end));
        
        in->pool->Wait();
    }
private:
    void UpdateDxDt(const StateType & x, StateType & dxdt, num_type t, size_t start, size_t end) const
    {
        for (size_t i = start; i < end; ++i) {
            dxdt[i] = 0;
            for (size_t j = 0; j < x.size(); ++j)
                dxdt[i] += G(i, j) * x[j];
            auto invC = 1 / C(i);
            dxdt[i] *= - invC;
            dxdt[i] += Rhs(i) * invC;
        }
    }
    num_type G(size_t i, size_t j) const { return (in->G)(i, j); }
    num_type C(size_t i) const { return in->C(i); }
    num_type Rhs(size_t i) const { return (in->Rhs)[i]; }
};

template <typename num_type> 
class ThermalNetworkReducedTransientSolver
{
public:
    using StateType = std::vector<num_type>;
    using ProjMatrix = Eigen::Matrix<num_type, Eigen::Dynamic, Eigen::Dynamic>;
    using ReducedMatrix = Eigen::Matrix<num_type, Eigen::Dynamic, Eigen::Dynamic>;

    struct Input
    {
        int verbose{0};
        num_type refT = 25;
        size_t threads = 1;

        ProjMatrix x;
        ReducedMatrix rG;
        ReducedMatrix rC;
        ReducedMatrix rB;
        ReducedMatrix coeff;
        ReducedMatrix input;
        std::vector<num_type> rhs;
        const ThermalNetwork<num_type> & network;
        std::unique_ptr<thread::ThreadPool> pool;
        size_t StateSize() const { return rG.cols(); }
        Input(const ThermalNetwork<num_type> & network, num_type refT, size_t q, size_t threads, int verbose = 0)
         : verbose(verbose), refT(refT), threads(threads), network(network)
        {
            pool.reset(new thread::ThreadPool(threads));
            ThermalMatrixBuilder<num_type> builder(network);
            auto [G, C, B] = builder.GetMatrices(refT, rhs);
            q = std::min(std::max(q, rhs.size()), network.Size());

            x = Prima(C, G, B, q);
            rG = x.transpose() * G * x;
            rC = x.transpose() * C * x;
            rB = x.transpose() * B;

            auto CredQR = rC.fullPivHouseholderQr();
            coeff = CredQR.solve(-1 * rG);
            input = CredQR.solve(rB);
            
            if (verbose > 0) {
                std::cout << "x:\n"  <<  x << std::endl;
                std::cout << "rG:\n" << rG << std::endl;
                std::cout << "rC:\n" << rC << std::endl;
                std::cout << "rB:\n" << rB << std::endl;
                std::cout << "rhs:[";
                for (const auto & v : rhs)
                    std::cout << v << ',' << std::endl;
                std::cout << ']' << std::endl;
                if (verbose > 1) {
                    std::cout << "G:\n" << G << std::endl;
                    std::cout << "C:\n" << C << std::endl;
                    std::cout << "B:\n" << B << std::endl;
                }
            }
        }
    };

    struct Recorder
    { 
        const Input * in;
        num_type prev{0};
        num_type count{0};
        num_type interval;
        std::ostream & os;
        const std::vector<size_t> & probs;
        Recorder(std::ostream & os, const Input * in, const std::vector<size_t> & probs, num_type interval)
         : in(in), interval(interval), os(os), probs(probs) {}
        void operator() (const StateType & x, num_type t)
        {
            if (count += t - prev; count > interval) {
                os << t;

    
                Eigen::Map<const Eigen::Matrix<num_type, Eigen::Dynamic, 1>> state(x.data(), x.size(), 1);
                auto result = in->x * state;
                for (auto p : probs)
                    os << ',' << result(p, 1);
                os << GENERIC_DEFAULT_EOL;
                count = 0;
            }
            prev = t;
        }
    };

    const Input * in;
    ThermalNetworkReducedTransientSolver(const Input * in) : in(in) {}
    virtual ~ThermalNetworkReducedTransientSolver() = default;
    
    void operator() (const StateType & x, StateType & dxdt, num_type t)
    {
        Eigen::Map<const Eigen::Matrix<num_type, Eigen::Dynamic, 1>> xvec(x.data(), in->StateSize(), 1);
        Eigen::Map<const Eigen::Matrix<num_type, Eigen::Dynamic, 1>> u(in->rhs.data(), in->rhs.size(), 1);
        Eigen::Map<Eigen::Matrix<num_type, Eigen::Dynamic, 1>> result(dxdt.data(), in->StateSize(), 1);
        result = in->coeff * xvec + in->input * u;
    }
};

}//namespace solver
}//namespace thermal