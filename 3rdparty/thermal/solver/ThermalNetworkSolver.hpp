#pragma once
#include "thermal/model/ThermalNetwork.hpp"
#include "generic/math/MathUtility.hpp"
#include "generic/tools/Tools.hpp"

#include <boost/numeric/odeint.hpp>
#include <memory>

#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseCholesky>
#include <Eigen/SparseLU>

namespace thermal
{
    namespace solver
    {

        using namespace model;
        using namespace generic;
        using namespace generic::ckt;
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
                constexpr bool direct = true;
                Eigen::setNbThreads(std::max<size_t>(1, m_threads));

                auto m = makeMNA(m_network); 
                auto rhs = makeRhs(m_network, refT);
                Eigen::Matrix<num_type, Eigen::Dynamic, 1> x;
                if (direct) {
                    // Eigen::SparseLU<Eigen::SparseMatrix<num_type> > solver;
                    Eigen::SimplicialCholesky<Eigen::SparseMatrix<num_type> > solver;
                    solver.analyzePattern(m.G);
                    solver.factorize(m.G);
                    x = solver.solve(m.B * rhs);
                }
                else {
                    Eigen::ConjugateGradient<Eigen::SparseMatrix<num_type>, Eigen::Lower | Eigen::Upper> solver;
                    solver.compute(m.G);
                    x = m.L * solver.solve(m.B * rhs);
                    std::cout << "#iterations:     " << solver.iterations() << std::endl;
                    std::cout << "estimated error: " << solver.error() << std::endl;
                }
                auto & nodes = m_network.GetNodes();
                for (size_t i = 0; i < nodes.size(); ++i)
                    nodes[i].t = x[i];
            }

        private:
            int m_verbose = 0;
            size_t m_threads = 1;
            ThermalNetwork<num_type> & m_network;
        };

        template <typename num_type>
        class ThermalNetworkTransientSolver
        {
        public:
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
                void operator()(const StateType & x, num_type t)
                {
                    if (count += t - prev; count > interval)
                    {
                        os << t;
                        for (auto p : probs)
                            os << ',' << x[p];
                        os << GENERIC_DEFAULT_EOL;
                        count = 0;
                    }
                    prev = t;
                }
            };

            // struct Intermidiate
            // {
            //     num_type refT = 25;
            //     size_t threads = 1;
            //     DenseVector<num_type> rhs;
            //     MNA<SparseMatrix<num_type> > m;
            //     const ThermalNetwork<num_type> & network;
            //     Intermidiate(const ThermalNetwork<num_type> & network, num_type refT, size_t threads)
            //         : refT(refT), threads(threads), network(network)
            //     {
            //         Eigen::setNbThreads(std::max<size_t>(1, threads));
            //         m = makeMNA(network);
            //         rhs = m.B * makeRhs(network, refT);
            //     }
            //     num_type G(size_t i, size_t j) const { return m.G.coeff(i, j); }
            //     num_type C(size_t i) const { return m.C.coeff(i, i); }
            //     num_type Rhs(size_t i) const { return rhs[i]; }
            //     size_t StateSize() const { return m.G.cols(); }
            // };
            
            struct Intermidiate
            {
                num_type refT = 25;
                size_t threads = 1;
                DenseVector<num_type>  rhs;
                SparseMatrix<num_type> invC;
                SparseMatrix<num_type> coeff;
                const ThermalNetwork<num_type> & network;
                Intermidiate(const ThermalNetwork<num_type> & network, num_type refT, size_t threads)
                    : refT(refT), threads(threads), network(network)
                {
                    Eigen::setNbThreads(std::max<size_t>(1, threads));

                    SparseMatrix<num_type> negG;
                    std::tie(invC, negG) = makeInvCandNegG(network);
                    coeff = invC * negG;
                    rhs = makeFullRhs(network, refT);
                }
                size_t StateSize() const { return invC.cols(); }
            };
            
            struct Solver
            {
                Intermidiate & im;
                explicit Solver(Intermidiate & im) : im(im) {}
                // void operator() (const StateType & x, StateType & dxdt, num_type t)
                // {
                //     // #pragma omp parallel for
                //     for (size_t i = 0; i < dxdt.size(); ++i) {
                //         dxdt[i] = 0;
                //         // #pragma omp parallel for
                //         for (size_t j = 0; j < x.size(); ++j) {
                //             dxdt[i] += im.G(i, j) * x.at(j);
                //         }
                //         dxdt[i] *= -1 / im.C(i);
                //         dxdt[i] += im.Rhs(i) / im.C(i);
                //     }
                // }
                void operator() (const StateType & x, StateType & dxdt, [[maybe_unused ]] num_type t)
                {
                    using VectorType = Eigen::Matrix<num_type, Eigen::Dynamic, 1>;
                    Eigen::Map<const VectorType> xM(x.data(), x.size());
                    Eigen::Map<VectorType> dxdtM(dxdt.data(), dxdt.size());
                    dxdtM = im.coeff * xM + im.invC * im.rhs;
                }
            };
           
            ThermalNetworkTransientSolver(const ThermalNetwork<num_type> & network, num_type refT, size_t threads)
             : m_refT(refT), m_threads(threads), m_network(network)
            {
                Eigen::setNbThreads(std::max<size_t>(1, threads));
            }

            virtual ~ThermalNetworkTransientSolver() = default;

            template <typename Recorder>
            size_t Solve(StateType & initState, num_type t0, num_type duration, num_type dt, Recorder * recorder = nullptr)
            {
                Intermidiate im(m_network, m_refT, m_threads);
                Solver solver(im);

                size_t steps;
                if (initState.size() != im.StateSize())
                    initState.assign(im.StateSize(), m_refT);
                using namespace boost::numeric::odeint;
                using ErrorStepperType = runge_kutta_cash_karp54<StateType>;
                if (nullptr == recorder)               
                    steps = integrate_adaptive(make_controlled(1e-12, 1e-10, ErrorStepperType{}),
                                        solver, initState, t0, t0 + duration, dt);
                else steps = integrate_adaptive(make_controlled(1e-12, 1e-10, ErrorStepperType{}),
                                        solver, initState, t0, t0 + duration, dt, *recorder);
                return steps;
            }
        private:
            num_type m_refT;
            size_t m_threads;
            const ThermalNetwork<num_type> & m_network;
        };

        // template <typename num_type>
        // class ThermalNetworkReducedTransientSolver
        // {
        // public:
        //     using StateType = std::vector<num_type>;
        //     using ProjMatrix = Eigen::Matrix<num_type, Eigen::Dynamic, Eigen::Dynamic>;
        //     using ReducedMatrix = Eigen::Matrix<num_type, Eigen::Dynamic, Eigen::Dynamic>;

        //     struct Input
        //     {
        //         int verbose{0};
        //         num_type refT = 25;
        //         size_t threads = 1;

        //         ProjMatrix x;
        //         ReducedMatrix rG;
        //         ReducedMatrix rC;
        //         ReducedMatrix rB;
        //         ReducedMatrix coeff;
        //         ReducedMatrix input;
        //         std::vector<num_type> rhs;
        //         const ThermalNetwork<num_type> &network;
        //         std::unique_ptr<thread::ThreadPool> pool;
        //         size_t StateSize() const { return rG.cols(); }
        //         Input(const ThermalNetwork<num_type> &network, num_type refT, size_t q, size_t threads, int verbose = 0)
        //             : verbose(verbose), refT(refT), threads(threads), network(network)
        //         {
        //             pool.reset(new thread::ThreadPool(threads));
        //             ThermalMatrixBuilder<num_type> builder(network);
        //             auto [G, C, B] = builder.GetMatrices(refT, rhs);
        //             q = std::min(std::max(q, rhs.size()), network.Size());

        //             x = Prima(C, G, B, q);
        //             rG = x.transpose() * G * x;
        //             rC = x.transpose() * C * x;
        //             rB = x.transpose() * B;

        //             auto CredQR = rC.fullPivHouseholderQr();
        //             coeff = CredQR.solve(-1 * rG);
        //             input = CredQR.solve(rB);

        //             if (verbose > 0)
        //             {
        //                 std::cout << "x:\n"
        //                           << x << std::endl;
        //                 std::cout << "rG:\n"
        //                           << rG << std::endl;
        //                 std::cout << "rC:\n"
        //                           << rC << std::endl;
        //                 std::cout << "rB:\n"
        //                           << rB << std::endl;
        //                 std::cout << "rhs:[";
        //                 for (const auto &v : rhs)
        //                     std::cout << v << ',' << std::endl;
        //                 std::cout << ']' << std::endl;
        //                 if (verbose > 1)
        //                 {
        //                     std::cout << "G:\n"
        //                               << G << std::endl;
        //                     std::cout << "C:\n"
        //                               << C << std::endl;
        //                     std::cout << "B:\n"
        //                               << B << std::endl;
        //                 }
        //             }
        //         }
        //     };

        //     struct Recorder
        //     {
        //         const Input *in;
        //         num_type prev{0};
        //         num_type count{0};
        //         num_type interval;
        //         std::ostream &os;
        //         const std::vector<size_t> &probs;
        //         Recorder(std::ostream &os, const Input *in, const std::vector<size_t> &probs, num_type interval)
        //             : in(in), interval(interval), os(os), probs(probs) {}
        //         void operator()(const StateType &x, num_type t)
        //         {
        //             if (count += t - prev; count > interval)
        //             {
        //                 os << t;

        //                 Eigen::Map<const Eigen::Matrix<num_type, Eigen::Dynamic, 1>> state(x.data(), x.size(), 1);
        //                 auto result = in->x * state;
        //                 for (auto p : probs)
        //                     os << ',' << result(p, 1);
        //                 os << GENERIC_DEFAULT_EOL;
        //                 count = 0;
        //             }
        //             prev = t;
        //         }
        //     };

        //     const Input *in;
        //     ThermalNetworkReducedTransientSolver(const Input *in) : in(in) {}
        //     virtual ~ThermalNetworkReducedTransientSolver() = default;

        //     void operator()(const StateType &x, StateType &dxdt, num_type t)
        //     {
        //         Eigen::Map<const Eigen::Matrix<num_type, Eigen::Dynamic, 1>> xvec(x.data(), in->StateSize(), 1);
        //         Eigen::Map<const Eigen::Matrix<num_type, Eigen::Dynamic, 1>> u(in->rhs.data(), in->rhs.size(), 1);
        //         Eigen::Map<Eigen::Matrix<num_type, Eigen::Dynamic, 1>> result(dxdt.data(), in->StateSize(), 1);
        //         result = in->coeff * xvec + in->input * u;
        //     }
        // };

    } // namespace solver
} // namespace thermal