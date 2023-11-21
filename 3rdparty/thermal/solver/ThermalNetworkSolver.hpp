#pragma once
#include "thermal/model/ThermalNetwork.hpp"
#include "generic/math/MathUtility.hpp"
#include "generic/tools/Tools.hpp"
#include "generic/circuit/MNA.hpp"
#include "generic/circuit/MOR.hpp"

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
            explicit ThermalNetworkSolver(ThermalNetwork<num_type> & network, size_t threads)
                : m_network(network)
            {
                Eigen::setNbThreads(std::max<size_t>(1, threads));
            }

            virtual ~ThermalNetworkSolver() = default;
            void SetVerbose(int verbose) { m_verbose = verbose; }

            void Solve(num_type refT) const
            {
                constexpr bool direct = false;
                auto m = makeMNA(m_network, true);
                auto rhs = makeRhs(m_network, true, refT);
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
                const ThermalNetworkTransientSolver & solver;
                Recorder(const ThermalNetworkTransientSolver & solver, std::ostream & os, num_type interval)
                 : interval(interval), os(os), solver(solver) {}
                virtual ~Recorder() = default;
                void operator()(const StateType & x, num_type t)
                {
                    if (count += t - prev; count > interval)
                    {
                        os << t;
                        for (auto p : solver.Probs())
                            os << ',' << x[p];
                        os << GENERIC_DEFAULT_EOL;
                        count = 0;
                    }
                    prev = t;
                }
            };
        
            struct Intermidiate
            {
                num_type refT = 25;
                DenseVector<num_type> hf;
                SparseMatrix<num_type> hfP;
                SparseMatrix<num_type> htcM;
                SparseMatrix<num_type> coeff;
                const ThermalNetwork<num_type> & network;
                std::unordered_map<size_t, size_t> rhs2Nodes;
                Intermidiate(const ThermalNetwork<num_type> & network, num_type refT)
                    : refT(refT), network(network)
                {
                    auto [invC, negG] = makeInvCandNegG(network);
                    coeff = invC * negG;

                    htcM = invC * makeBondsRhs(network, refT);
                    hfP = invC * makeSourceProjMatrix(network, rhs2Nodes);
                    hf = DenseVector<num_type>(rhs2Nodes.size());
                    for (auto [rhs, node] : rhs2Nodes)
                        hf[rhs] = network[node].hf;
                }
                virtual ~Intermidiate() = default;
                size_t StateSize() const { return coeff.cols(); }
            };

            struct NullExcitation
            {
                virtual ~NullExcitation() = default;
                num_type operator() ([[maybe_unused]] num_type t) const { return 1; }
            };

            template <typename Excitation>
            struct Solver
            {
                const Excitation & e;
                const Intermidiate & im;
                DenseVector<num_type> hf;
                explicit Solver(const Intermidiate & im, const Excitation & e) : e(e), im(im) { hf = DenseVector<num_type>(im.hf.size());}
                virtual ~Solver() = default;

                void operator() (const StateType & x, StateType & dxdt, num_type t)
                {
                    using VectorType = Eigen::Matrix<num_type, Eigen::Dynamic, 1>;
                    Eigen::Map<const VectorType> xM(x.data(), x.size());
                    Eigen::Map<VectorType> dxdtM(dxdt.data(), dxdt.size());
                    for (int i = 0; i < im.hf.size(); ++i) hf[i] = e(t) * im.hf[i];
                    dxdtM = im.coeff * xM + im.htcM + im.hfP * hf;
                }
            };
           
            ThermalNetworkTransientSolver(const ThermalNetwork<num_type> & network, num_type refT, std::set<size_t> probs = {})
             : m_refT(refT), m_probs(std::move(probs)), m_network(network)
            {
                m_im.reset(new Intermidiate(m_network, m_refT));
            }

            virtual ~ThermalNetworkTransientSolver() = default;
            size_t StateSize() const { return m_im->StateSize(); }

            template <typename Observer = Recorder, typename Excitation = NullExcitation>
            size_t Solve(StateType & initState, num_type t0, num_type duration, num_type dt, Observer observer, const Excitation & e = NullExcitation{})
            {
                if (initState.size() != StateSize()) return 0;
                using namespace boost::numeric::odeint;
                using ErrorStepperType = runge_kutta_cash_karp54<StateType>;
                return integrate_adaptive(make_controlled<ErrorStepperType>(num_type{1e-12}, num_type{1e-10}),
                                        Solver<Excitation>(*m_im, e), initState, num_type{t0}, num_type{t0 + duration}, num_type{dt}, std::move(observer));
            }

            const std::set<size_t> Probs() const { return m_probs; }
            const Intermidiate & Im() const { return *m_im; }
        private:
            num_type m_refT;
            std::set<size_t> m_probs;
            const ThermalNetwork<num_type> & m_network;
            std::unique_ptr<Intermidiate> m_im{nullptr};
        };

        template <typename num_type>
        class ThermalNetworkReducedTransientSolver
        {
        public:
            using StateType = std::vector<num_type>;    
            struct Intermidiate
            {
                num_type refT;
                const std::set<size_t> & probs;
                const ThermalNetwork<num_type> & network;

                bool includeBonds{true};
                DenseVector<num_type> uh;
                DenseVector<num_type> ub;
                DenseMatrix<num_type> rLT;
                ReducedModel<num_type> rom;
                DenseMatrix<num_type> coeff, input;
                Intermidiate(const ThermalNetwork<num_type> & network, num_type refT, const std::set<size_t> & probs, size_t order)
                    : refT(refT), probs(probs), network(network)
                {
                    const size_t source = network.Source(includeBonds);
                    auto m = makeMNA(network, includeBonds, probs);
                    {
                        tools::ProgressTimer t("reduce");
                        rom = Reduce(m, std::max(source, order));
                        std::cout << "mor: " << rom.x.rows() << "->" << rom.x.cols() << std::endl;
                    }
                    auto dcomp = rom.m.C.ldlt();
                    coeff = dcomp.solve(-1 * rom.m.G);
                    input = dcomp.solve(rom.m.B);
                    rLT = rom.m.L.transpose();
                    uh.resize(input.cols());
                    if (not includeBonds) {
                        auto bondsRhs = makeBondsRhs(network, refT);
                        ub = rom.xT * bondsRhs;
                    }
                }           

                virtual ~Intermidiate() = default;

                bool Input2State(const StateType & in, StateType & x) const
                {
                    if (in.size() != network.Size()) return false;
                    using VectorType = Eigen::Matrix<num_type, Eigen::Dynamic, 1>;
                    x.resize(rom.m.G.cols());
                    Eigen::Map<VectorType> xvec(x.data(), x.size());
                    Eigen::Map<const VectorType> ivec(in.data(), in.size());
                    xvec = rom.xT * ivec;
                    return true;
                }

                void State2Output(const StateType & x, StateType & out) const
                {
                    using VectorType = Eigen::Matrix<num_type, Eigen::Dynamic, 1>;
                    out.resize(rLT.rows());
                    Eigen::Map<VectorType> ovec(out.data(), out.size());
                    Eigen::Map<const VectorType> xvec(x.data(), x.size());
                    ovec = rLT * xvec;
                }

                size_t StateSize() const { return rom.m.G.cols(); }
            };

            struct Recorder
            {
                StateType out;
                num_type prev{0};
                num_type count{0};
                num_type interval;
                std::ostream & os;
                const ThermalNetworkReducedTransientSolver & solver;
                Recorder(const ThermalNetworkReducedTransientSolver & solver, std::ostream & os, num_type interval)
                 : interval(interval), os(os), solver(solver) {}

                virtual ~Recorder() = default;
    
                void operator()(const StateType & x, num_type t)
                {   
                    if (count += t - prev; count > interval)
                    {
                        os << t;
                        solver.Im().State2Output(x, out);
                        for (auto o : out)
                            os << ',' << o;
                        os << GENERIC_DEFAULT_EOL;
                        count = 0;
                    }
                    prev = t;
                }
            };

            struct NullExcitation
            {
                virtual ~NullExcitation() = default;
                num_type operator() ([[maybe_unused]] num_type t) const { return 1; }
            };

            template <typename Excitation>
            struct Solver
            {
                Intermidiate & im;
                const Excitation & e;
                explicit Solver(Intermidiate & im, const Excitation & e) : im(im), e(e) {}
                virtual ~Solver() = default;
                void operator() (const StateType & x, StateType & dxdt, [[maybe_unused]] num_type t)
                {
                    using VectorType = Eigen::Matrix<num_type, Eigen::Dynamic, 1>;
                    const size_t nodes = im.network.Size();
                    for (size_t i = 0, s = 0; i < nodes; ++i) {
                        const auto & node = im.network[i];
                        if (node.hf != 0 || (im.includeBonds && node.htc != 0))
                            im.uh[s++] = node.hf * e(t) + node.htc * im.refT;
                    }
                    Eigen::Map<VectorType> result(dxdt.data(), dxdt.size());
                    Eigen::Map<const VectorType> xvec(x.data(), x.size());
                    Eigen::Map<VectorType> dxdtM(dxdt.data(), dxdt.size());
                    result = im.coeff * xvec + im.input * im.uh;
                    if (not im.includeBonds) result += im.ub;
                }
            };

            ThermalNetworkReducedTransientSolver(const ThermalNetwork<num_type> & network, num_type refT, std::set<size_t> probs = {}, size_t order = 99)
             : m_refT(refT), m_probs(std::move(probs)), m_network(network)
            {
                m_im.reset(new Intermidiate(m_network, m_refT, m_probs, order));
            }

            virtual ~ThermalNetworkReducedTransientSolver() = default;

            template <typename Observer = Recorder, typename Excitation = NullExcitation>
            size_t Solve(StateType & initT, num_type t0, num_type duration, num_type dt, Observer observer, const Excitation & e = NullExcitation{})
            {
                StateType initState;
                if (not m_im->Input2State(initT, initState)) return 0;

                using namespace boost::numeric::odeint;
                using ErrorStepperType = runge_kutta_cash_karp54<StateType>;
                return integrate_adaptive(make_controlled<ErrorStepperType>(num_type{1e-12}, num_type{1e-10}),
                                        Solver<Excitation>(*m_im, e), initState, num_type{t0}, num_type{t0 + duration}, num_type{dt}, observer);
            }
            const std::set<size_t> Probs() const { return m_probs; }
            const Intermidiate & Im() const { return *m_im; }
        private:
            num_type m_refT;
            std::set<size_t> m_probs;
            const ThermalNetwork<num_type> & m_network;
            std::unique_ptr<Intermidiate> m_im{nullptr};
        };
    } // namespace solver
} // namespace thermal