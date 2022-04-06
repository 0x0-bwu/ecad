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
#endif//MFSOLVER_SUPPORT

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

    void Solve(num_type refT) const
    {
#ifdef MFSOLVER_SUPPORT
        SolveMF(refT);
#else
        SolveEigen(refT);
#endif//MFSOLVER_SUPPORT
    }

#ifdef MFSOLVER_SUPPORT
    void SolveMF(num_type refT) const
    {
    
        ThermalNetworkMatrixBuilder<num_type> builder(m_network);
        auto size = builder.GetMatrixSize();
        const auto & mnMap = builder.GetMatrixNodeIndicesMap();
        const auto & nmMap = builder.GetNodeMatrixIndicesMap();

        mfs_options myOptions;
        // myOptions.SetReduceMatrix(2);//specifyareductiontype2.
        myOptions.SetFullPrecision(true);//Setfullprecision.
        myOptions.SetMetisReordering(true);//RequestreorderingusingMETIS.
        myOptions.SetMetis51(true);//UseMETIS5.1
        myOptions.SetCheckSolution(false);//Donotchecksolution.
        myOptions.SetMultithread(true);//Toenablemultithreading.
        myOptions.SetNcpu(std::max<size_t>(1, m_threads));//Enable4threads.
        myOptions.SetNcpuFbs(std::max<size_t>(1, m_threads));//Solverfor4RHS.
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
        
        auto & nodes = m_network.GetNodes();
        for(size_t i = 0; i < dimb; ++i)
            nodes[mnMap.at(i)].t = b[i];
    }
#endif//MFSOLVER_SUPPORT
    void SolveEigen(num_type refT) const
    {

        ThermalNetworkMatrixBuilder<num_type> builder(m_network);
        size_t size = builder.GetMatrixSize();
        const auto & mnMap = builder.GetMatrixNodeIndicesMap();
        const auto & nmMap = builder.GetNodeMatrixIndicesMap();

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

        Eigen::setNbThreads(std::max<size_t>(1, m_threads));
        
        Eigen::VectorXd x;
        //iterator
        // Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower|Eigen::Upper> solver;
        // solver.compute(spMat);
        // x = solver.solve(b);
        // std::cout << "#iterations:     " << solver.iterations() << std::endl;
        // std::cout << "estimated error: " << solver.error()      << std::endl;

        //direct
        Eigen::SparseLU<Eigen::SparseMatrix<double> > solver;
        // Eigen::SimplicialCholesky<Eigen::SparseMatrix<double> > solver;
        solver.analyzePattern(spMat);
        solver.factorize(spMat);
        x = solver.solve(b); 

        auto & nodes = m_network.GetNodes();
        for(size_t i = 0; i < size; ++i)
            nodes[mnMap.at(i)].t = x[i];
    }
private:
    size_t m_threads = 1;
    ThermalNetwork<num_type> & m_network;
};

}//namespace solver
}//namespace thermal
#endif//THERMAL_SOLVER_THERMALNETWORKSOLVER_HPP