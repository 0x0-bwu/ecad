#pragma once
#include "ThermalNetwork.h"
#include "generic/math/MathUtility.hpp"
#include "generic/tools/Tools.hpp"
#include "generic/circuit/MNA.hpp"
#include "generic/circuit/MOR.hpp"
#include "generic/tools/Log.hpp"

#include <boost/numeric/odeint.hpp>
#include <memory>
#include <list>

#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseCholesky>
#include <Eigen/SparseLU>

namespace thermal::solver {
    using namespace model;
    using namespace generic;
    using namespace generic::ckt;
    template <typename num_type>
    class ThermalNetworkSolver
    {
    public:
        explicit ThermalNetworkSolver(ThermalNetwork<num_type> & network, int solverType = 2)
            : m_network(network), m_solverType(solverType)
        {
        }

        virtual ~ThermalNetworkSolver() = default;

        void Solve(num_type refT, std::vector<num_type> & result) const
        {
            using namespace generic::math::la;
            auto m = makeMNA(m_network, true);
            auto rhs = makeRhs(m_network, true, refT);
            result.resize(m_network.GetNodes().size(), refT);
            Eigen::Map<DenseVector<num_type>> x(result.data(), result.size());
            switch (m_solverType) {
                case 0 : {
                    Eigen::SparseLU<Eigen::SparseMatrix<num_type> > solver;
                    solver.analyzePattern(m.G);
                    solver.factorize(m.G);
                    x = solver.solve(m.B * rhs);
                    break;
                }
                case 1 : {
                    Eigen::SimplicialCholesky<Eigen::SparseMatrix<num_type> > solver;
                    solver.analyzePattern(m.G);
                    solver.factorize(m.G);
                    x = solver.solve(m.B * rhs);
                    break;
                }
                case 2 : {
                    Eigen::ConjugateGradient<Eigen::SparseMatrix<num_type>, Eigen::Lower | Eigen::Upper> solver;
                    solver.compute(m.G);
                    x = m.L * solver.solve(m.B * rhs);

                    ECAD_TRACE("#iterations: %1%", solver.iterations())
                    ECAD_TRACE("estimated error: %1%", solver.error())
                    break;
                }
                default : {
                    ECAD_ASSERT(false)
                    break;
                }
            }
        }

    private:
        ThermalNetwork<num_type> & m_network;
        int m_solverType{2};
    };

    template <typename num_type>
    using Sample = std::vector<num_type>;

    template <typename num_type>
    using Samples = std::vector<Sample<num_type>>;

    template <typename num_type>
    struct TimeWindow
    {
        num_type start{0};
        num_type end{0};
        num_type interval{std::numeric_limits<num_type>::max()};
        TimeWindow(num_type start, num_type end, num_type interval)
            : start(start), end(end), interval(interval)
        {
            assert(start < end);
        }

        bool isInside(num_type t) const
        {
            return math::Within<math::Close>(t, start, end);
        }

        size_t EsitimateSamples() const
        {
            return (end - start) / interval;
        }
    };

    template <typename num_type>
    class ThermalNetworkTransientSolver
    {
    public:
        using StateType = std::vector<num_type>;
        struct Sampler
        {
            num_type endT{0};
            num_type prev{0};
            num_type count{0};
            bool verbose{false};
            StateType & lastState;
            Samples<num_type> & samples;
            TimeWindow<num_type> window;
            const ThermalNetworkTransientSolver & solver;
            Sampler(const ThermalNetworkTransientSolver & solver, Samples<num_type> & samples, StateType & lastState, TimeWindow<num_type> window, num_type endT, bool verbose)
             : endT(endT), verbose(verbose), lastState(lastState), samples(samples), window(std::move(window)), solver(solver)
            {
                if (not samples.empty())
                    prev = samples.back().front();
                samples.reserve(window.EsitimateSamples());
            }
            virtual ~Sampler() = default;
            void operator() (const StateType & x, num_type t)
            {
                if (window.isInside(t)) {
                    if (count += t - prev; count > window.interval) {
                        const auto & probs = solver.Probs();
                        Sample<num_type> sample; sample.reserve(probs.size() + 1);
                        sample.emplace_back(t);
                        for (auto p : probs) sample.emplace_back(x[p]);
                        if (verbose) {
                            auto res = sample;
                            auto begin = res.begin(); begin++;
                            std::for_each(begin, res.end(), [](auto & t){ t = generic::unit::Kelvins2Celsius(t); });
                            ECAD_TRACE(generic::fmt::Fmt2Str(res, ","))
                        }
                        samples.emplace_back(std::move(sample));
                        count = 0;
                    }
                    prev = t;
                }
                if (math::GE<num_type>(t, endT)) lastState = x;
            }
        };
    
        struct Intermidiate
        {
            num_type refT = 25;
            DenseVector<num_type> hf;
            DenseVector<num_type> scen;
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
                scen = DenseVector<num_type>(rhs2Nodes.size());
                hf = DenseVector<num_type>(rhs2Nodes.size());
                for (auto [rhs, node] : rhs2Nodes) {
                    scen[rhs] = network[node].scen;
                    hf[rhs] = network[node].hf;
                }
            }
            virtual ~Intermidiate() = default;
            size_t StateSize() const { return coeff.cols(); }
        };

        template <typename Excitation>
        struct Solver
        {
            const Intermidiate & im;
            DenseVector<num_type> hf;
            const Excitation * e{nullptr};
            explicit Solver(const Intermidiate & im, const Excitation * e) : im(im), e(e) { hf = DenseVector<num_type>(im.hf.size());}
            virtual ~Solver() = default;

            void operator() (const StateType & x, StateType & dxdt, num_type t)
            {
                using VectorType = Eigen::Matrix<num_type, Eigen::Dynamic, 1>;
                Eigen::Map<const VectorType> xM(x.data(), x.size());
                Eigen::Map<VectorType> dxdtM(dxdt.data(), dxdt.size());
                for (int i = 0; i < im.hf.size(); ++i) {
                    num_type excitation = e ? (*e)(t, im.scen[i]) : 1;
                    hf[i] = im.hf[i] * excitation;
                }
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

        template <typename Observer = Sampler, typename Excitation>
        size_t SolveAdaptive(StateType & initState, num_type t0, num_type duration, num_type dt, num_type absErr, num_type relErr, Observer observer, const Excitation * e = nullptr)
        {
            if (initState.size() != StateSize()) return 0;
            using namespace boost::numeric::odeint;
            using ErrorStepperType = runge_kutta_dopri5<StateType>;
            return integrate_adaptive(make_controlled<ErrorStepperType>(absErr, relErr),
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
                    rom = Reduce(m, std::max(order, source));
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

        struct Sampler
        {
            StateType out;
            num_type endT{0};
            num_type prev{0};
            num_type count{0};
            bool verbose{false};
            StateType & lastState;
            TimeWindow<num_type> window;
            Samples<num_type> & samples;
            const ThermalNetworkReducedTransientSolver & solver;
            Sampler(const ThermalNetworkReducedTransientSolver & solver, Samples<num_type> & samples, StateType & lastState, TimeWindow<num_type> window, num_type endT, bool verbose)
             : endT(endT), verbose(verbose), lastState(lastState), window(std::move(window)), samples(samples), solver(solver)
            {
                if (not samples.empty())
                    prev = samples.back().front();
                samples.reserve(window.EsitimateSamples());
            }
            virtual ~Sampler() = default;
            void operator() (const StateType & x, num_type t)
            {
                if (window.isInside(t)) {
                    if (count += t - prev; count > window.interval) {
                        solver.Im().State2Output(x, out);
                        Sample<num_type> sample; sample.reserve(out.size() + 1);
                        sample.emplace_back(t);
                        for (const auto & o : out)
                            sample.emplace_back(o);
                        if (verbose) {
                            auto res = sample;
                            auto begin = res.begin(); begin++;
                            std::for_each(begin, res.end(), [](auto & t){ t = generic::unit::Kelvins2Celsius(t); });
                            ECAD_TRACE(generic::fmt::Fmt2Str(res, ","))
                        }
                        samples.emplace_back(std::move(sample));
                        count = 0;
                    }
                    prev = t;
                }
                if (math::GE<num_type>(t, endT)) lastState = x;
            }
        };

        template <typename Excitation>
        struct Solver
        {
            Intermidiate & im;
            const Excitation * e{nullptr};
            explicit Solver(Intermidiate & im, const Excitation * e) : im(im), e(e) {}
            virtual ~Solver() = default;
            void operator() (const StateType & x, StateType & dxdt, num_type t)
            {
                const size_t nodes = im.network.Size();
                for (size_t i = 0, s = 0; i < nodes; ++i) {
                    const auto & node = im.network[i];
                    num_type excitation = e ? (*e)(t, node.scen) : 1;
                    if (node.hf != 0 || (im.includeBonds && node.htc != 0))
                        im.uh[s++] = node.hf * excitation + node.htc * im.refT;
                }
                Eigen::Map<DenseVector<num_type>> result(dxdt.data(), dxdt.size());
                Eigen::Map<const DenseVector<num_type>> xvec(x.data(), x.size());
                Eigen::Map<DenseVector<num_type>> dxdtM(dxdt.data(), dxdt.size());
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

        template <typename Observer = Sampler, typename Excitation>
        size_t SolveAdaptive(StateType & initState, num_type t0, num_type duration, num_type dt, num_type absErr, num_type relErr, Observer observer, const Excitation * e = nullptr)
        {
            using namespace boost::numeric::odeint;
            using ErrorStepperType = runge_kutta_cash_karp54<StateType>;
            return integrate_adaptive(make_controlled<ErrorStepperType>(absErr, relErr),
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
} // namespace thermal::solver