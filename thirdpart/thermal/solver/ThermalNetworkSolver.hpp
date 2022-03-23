#ifndef THERMAL_SOLVER_THERMALNETWORKSOLVER_HPP
#define THERMAL_SOLVER_THERMALNETWORKSOLVER_HPP
#include "thermal/model/ThermalNetwork.hpp"
#include "generic/math/MathUtility.hpp"
#include "generic/tools/Tools.hpp"
#include <memory>

//#define MFSOLVER_SUPPORT
#ifdef MFSOLVER_SUPPORT
#include "mfsolver64_lib.h"
#include "mfsolver64def.h"
#else
#include "Eigen/IterativeLinearSolvers"
#include "Eigen/SparseCholesky"
#include "Eigen/SparseLU"
#include "Eigen/Sparse"
#endif//MFSOLVER_SUPPORT

namespace thermal {
namespace solver {

using namespace model;

template <typename num_type> 
class ThermalNetworkSolver
{
public:
    explicit ThermalNetworkSolver(const ThermalNetwork<num_type> & network)
     : m_network(network){}

    virtual ~ThermalNetworkSolver() = default;

#ifdef MFSOLVER_SUPPORT
    std::vector<num_type> Solve(num_type refT) const
    {
    
        ThermalNetworkMatrixBuilder<num_type> builder(m_network);
        auto diagCoeffs = std::unique_ptr<std::vector<num_type> >(new std::vector<num_type>{}) ;
        auto edges = std::unique_ptr<std::list<ThermalNetwork<num_type>::Edge> >(new std::list<ThermalNetwork<num_type>::Edge>{});
        auto size = builder.GetMatrixSize();

        {
            std::cout << "get coeffs" << std::endl;
            generic::tools::ProgressTimer t;

            builder.GetCoeffs(*edges);
            builder.GetDiagCoeffs(*diagCoeffs);
        }

        GENERIC_ASSERT((edges->size() % 2) == 0)
        size_t nZero = diagCoeffs->size() + edges->size() / 2;

        const auto & mnMap = builder.GetMatrixNodeIndicesMap();
        const auto & nmMap = builder.GetNodeMatrixIndicesMap();

        mfs_options myOptions;
        // myOptions.SetReduceMatrix(2);//specifyareductiontype2.
        myOptions.SetFullPrecision(true);//Setfullprecision.
        myOptions.SetMetisReordering(true);//RequestreorderingusingMETIS.
        myOptions.SetMetis51(true);//UseMETIS5.1
        myOptions.SetCheckSolution(false);//Donotchecksolution.
        myOptions.SetMultithread(true);//Toenablemultithreading.
        myOptions.SetNcpu(32);//Enable4threads.
        myOptions.SetNcpuFbs(32);//Solverfor4RHS.
        myOptions.SetCudaMode(MfCudaAuto);//MfCudaAuto(autoselectGPUmode).
        // myOptions.quiet = true;
        mfsolver64lib :: MfsolverLib mf( solver_type :: REAL_SYM );
        mf.SetOptions (&myOptions);

        ANSLONG *IA;
        int dimA = size, *JA , iarg , flag = 0, Nrhs=1 , error = 0;
        double * AA = nullptr ;
        IA = ( ANSLONG *) malloc ((dimA + 1) * sizeof ( ANSLONG ));
        IA[0] = 0;
        size_t count;
        for(size_t i = 0; i < size; ++i){
            count = 1;
            const auto & ns = m_network.NS(mnMap.at(i));
            for(const auto & n : ns){
                if(nmMap.at(n) < i) count++;
            }
            IA[i + 1] = IA[i] + count;
        }
        size_t NzA = IA[dimA];
        GENERIC_ASSERT(NzA == nZero)
        size_t aaIndex = 0;
        size_t jaIndex = 0;
        AA = ( double *) malloc (NzA * sizeof ( double ));
        JA = (int *) malloc ( NzA * sizeof (int));
        for(size_t i = 0; i < size; ++i){
            JA[jaIndex++] = i;
            AA[aaIndex++] = builder.GetDiagCoeff(i);
            const auto & ns = m_network.NS(mnMap.at(i));
            // std::sort(ns.begin(), ns.end(), std::less<size_t>());
            for(const auto & n : ns){
                auto j = nmMap.at(n);
                if(j < i){
                    JA[jaIndex++] = j;
                    AA[aaIndex++] = builder.GetCoeff(j, i);
                }
            }
        }

        int* iperm = nullptr; //null signals that we want to invoke ordering first
        //4.Performreorderingandsymbolicfactorization.
        mf.FindReordering(dimA,NzA,(/*const*/ ANSLONG*)IA,(/*const*/ int*)JA, iperm);
        //5.Performfactorization.
        error=mf.FindFactorization(dimA,NzA,IA,JA,AA);
        //6.Solve.
        //6.1.ProvideRHS.
        int dimb=dimA;
        //int numRhs=1;
        double*b=(double*)malloc(dimb * Nrhs * sizeof(double));
        for(size_t i = 0; i < size; ++i){
            b[i] = builder.GetRhs(i, refT);
        }
        //6.2.Initializeb-rhstodesiredvalue.
        //6.3.Pre-initializeFBS.
        mf.PostInitialize(dimA,IA,JA,AA,true,false);
        //6.4.DoFBS.
        //error=mf.FindSolutions(dimA,IA,JA,AA,Nrhs,(void**)&b);
        error=mf.FindSolutions(dimA,nullptr,nullptr,nullptr,Nrhs,(void**)&b);
        
        const auto & nodes = network.GetNodes();
        std::vector<num_type> results(network.Size());
        for(size_t i = 0; i < dimb; ++i) results[mnMap.at(i)] = b[i];
        for(size_t i = 0; i < network.Size(); ++i) {
            if(!nmMap.count(i)) results[i] = nodes[i].t;
        }
        return results;
    }
#else
    std::vector<num_type> Solve(num_type refT) const
    {

        ThermalNetworkMatrixBuilder<num_type> builder(m_network);
        size_t size = builder.GetMatrixSize();
        const auto & mnMap = builder.GetMatrixNodeIndicesMap();
        const auto & nmMap = builder.GetNodeMatrixIndicesMap();

        Eigen::SparseMatrix<double> spMat(size, size);
        {
            std::cout << "build matrix" << std::endl;
            generic::tools::ProgressTimer t;

            spMat.reserve(Eigen::VectorXi::Constant(size, 10));
            auto diagCoeffs = std::unique_ptr<std::vector<num_type> >(new std::vector<num_type>{}) ;
            auto edges = std::unique_ptr<std::list<typename ThermalNetwork<num_type>::Edge> >(new std::list<typename ThermalNetwork<num_type>::Edge>{});

            {
                std::cout << "get coeffs" << std::endl;
                generic::tools::ProgressTimer t;
                builder.GetCoeffs(*edges);
                builder.GetDiagCoeffs(*diagCoeffs);
            }

            {
                std::cout << "insert coeffs" << std::endl;
                generic::tools::ProgressTimer t;
                for(size_t i = 0; i < size; ++i)
                    spMat.insert(i, i) = (*diagCoeffs)[i];

                for(const auto & edge : *edges){
                    spMat.insert(edge.x, edge.y) = edge.r;
                }
            }
            edges.reset();
            diagCoeffs.reset();
            spMat.makeCompressed();
        }

        Eigen::VectorXd b(size);
        {
            std::cout << "build rhs" << std::endl;
            generic::tools::ProgressTimer t;
            for(size_t i = 0; i < size; ++i){
                b[i] = builder.GetRhs(i, refT);
            }
        }

        Eigen::setNbThreads(8);
        std::cout << "Eigen threads: " << Eigen::nbThreads() << std::endl;//wbtest
        
        Eigen::VectorXd x;
        {
            std::cout << "solving" << std::endl;
            generic::tools::ProgressTimer t;

            //iterator
            // Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower|Eigen::Upper> solver;
            // solver.compute(spMat);
            // x = solver.solve(b);
            // std::cout << "#iterations:     " << solver.iterations() << std::endl;
            // std::cout << "estimated error: " << solver.error()      << std::endl;

            //direct
            Eigen::SparseLU<Eigen::SparseMatrix<double> > solver;
            // Eigen::SimplicialCholesky<Eigen::SparseMatrix<double> > solver;
            {
                std::cout << "analyzePattern and factorize" << std::endl;
                generic::tools::ProgressTimer t;
                solver.analyzePattern(spMat);
                solver.factorize(spMat);
            }

            {
                std::cout << "solve" << std::endl;
                generic::tools::ProgressTimer t; 
                x = solver.solve(b); 
            }
        }

        const auto & nodes = m_network.GetNodes();
        std::vector<num_type> results(m_network.Size());
        for(size_t i = 0; i < size; ++i) results[mnMap.at(i)] = x[i];
        for(size_t i = 0; i < m_network.Size(); ++i) {
            if(!nmMap.count(i)) results[i] = nodes[i].t;
        }
        return results;
    }
#endif//MFSOLVER_SUPPORT
private:
    const ThermalNetwork<num_type> & m_network;
};

}//namespace solver
}//namespace thermal
#endif//THERMAL_SOLVER_THERMALNETWORKSOLVER_HPP