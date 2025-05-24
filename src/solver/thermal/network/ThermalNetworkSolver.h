#pragma once
#include "ThermalNetwork.h"
#include "generic/tools/Tools.hpp"
#include "generic/circuit/MNA.hpp"
#include "generic/circuit/MOR.hpp"

#include <boost/numeric/odeint.hpp>
#include <memory>
#include <list>

#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseCholesky>
#include <Eigen/SparseLU>

#ifdef ECAD_APPLE_ACCELERATE_SUPPORT
#include <Eigen/AccelerateSupport>
#endif //ECAD_APPLE_ACCELERATE_SUPPORT

namespace thermal::solver {
    using namespace model;
    using namespace generic;
    using namespace generic::ckt;
    template <typename Scalar>
    class ThermalNetworkSolver
    {
    public:
        explicit ThermalNetworkSolver(ThermalNetwork<Scalar> & network, int solverType = 2)
            : m_network(network), m_solverType(solverType)
        {
        }

        virtual ~ThermalNetworkSolver() = default;

        void Solve(Scalar refT, std::vector<Scalar> & result, std::string rptDir) const
        {
            using namespace generic::math::la;
            auto m = makeMNA(m_network, true);
            auto rhs = makeRhs(m_network, true, refT);
            if (not rptDir.empty()) {

                auto dumpSpMat = [](const auto & filename, auto & mat) {
                    std::ofstream out(filename);
                    for (int k = 0; k < mat.outerSize(); ++k)
                        for (typename SparseMatrix<Scalar>::InnerIterator it(mat, k); it; ++it)
                            out << it.row() << " " << it.col() << " " << it.value() << "\n";
                    out.close();
                };
                generic::fs::CreateDir(rptDir);
                dumpSpMat(rptDir + "/G.txt", m.G);
                dumpSpMat(rptDir + "/B.txt", m.B);
                dumpSpMat(rptDir + "/C.txt", m.C);
                dumpSpMat(rptDir + "/L.txt", m.L);
                std::ofstream osRhs(rptDir + "/rhs.txt");
                osRhs << rhs; osRhs.close();
            }
            result.resize(m_network.GetNodes().size(), refT);
            Eigen::Map<DenseVector<Scalar>> x(result.data(), result.size());
            switch (m_solverType) {
                case 0 : {
                    Eigen::SparseLU<Eigen::SparseMatrix<Scalar> > solver(m.G);
                    x = m.L * solver.solve(m.B * rhs);
                    break;
                }
                case 1 : {
                    Eigen::SimplicialCholesky<Eigen::SparseMatrix<Scalar> > solver(m.G);
                    x = m.L * solver.solve(m.B * rhs);
                    break;
                }
                case 2: {
#ifdef ECAD_APPLE_ACCELERATE_SUPPORT
                    Eigen::AccelerateLLT<Eigen::SparseMatrix<Scalar> > solver(m.G);
#else
                    Eigen::SimplicialLLT<Eigen::SparseMatrix<Scalar> > solver(m.G);
#endif //ECAD_APPLE_ACCELERATE_SUPPORT
                    x = m.L * solver.solve(m.B * rhs);
                    break;
                }
                case 3: {
#ifdef ECAD_APPLE_ACCELERATE_SUPPORT
                    Eigen::AccelerateLDLT<Eigen::SparseMatrix<Scalar>,0> solver(m.G);
#else
                    Eigen::SimplicialLDLT<Eigen::SparseMatrix<Scalar> > solver(m.G);
#endif //ECAD_APPLE_ACCELERATE_SUPPORT
                    x = m.L * solver.solve(m.B * rhs);
                    break;
                }
                case 10 : {
                    Eigen::ConjugateGradient<Eigen::SparseMatrix<Scalar>, Eigen::Lower | Eigen::Upper> solver(m.G);
                    x = m.L * solver.solve(m.B * rhs);
                    ECAD_TRACE("#iterations: %1%", solver.iterations());
                    ECAD_TRACE("estimated error: %1%", solver.error());
                    break;
                }
                default : {
                    ECAD_ASSERT(false)
                    break;
                }
            }
        }

    private:
        ThermalNetwork<Scalar> & m_network;
        int m_solverType{2};
    };

    template <typename Scalar>
    using Sample = std::vector<Scalar>;

    template <typename Scalar>
    using Samples = std::vector<Sample<Scalar>>;

    template <typename Scalar>
    struct TimeWindow
    {
        Scalar start{0};
        Scalar end{0};
        Scalar interval{std::numeric_limits<Scalar>::max()};
        TimeWindow(Scalar start, Scalar end, Scalar interval)
            : start(start), end(end), interval(interval)
        {
            assert(start < end);
        }

        bool isInside(Scalar t) const
        {
            return math::Within<math::Close>(t, start, end);
        }

        size_t EsitimateSamples() const
        {
            return (end - start) / interval;
        }
    };

    template <typename Scalar>
    class ThermalNetworkTransientSolver
    {
    public:
        using StateType = std::vector<Scalar>;
        struct Sampler
        {
            Scalar endT{0};
            Scalar prev{0};
            Scalar count{0};
            bool verbose{false};
            StateType & lastState;
            Samples<Scalar> & samples;
            TimeWindow<Scalar> window;
            const ThermalNetworkTransientSolver & solver;
            Sampler(const ThermalNetworkTransientSolver & solver, Samples<Scalar> & samples, StateType & lastState, TimeWindow<Scalar> window, Scalar endT, bool verbose)
             : endT(endT), verbose(verbose), lastState(lastState), samples(samples), window(std::move(window)), solver(solver)
            {
                if (not samples.empty())
                    prev = samples.back().front();
                samples.reserve(window.EsitimateSamples());
            }
            virtual ~Sampler() = default;
            void operator() (const StateType & x, Scalar t)
            {
                if (window.isInside(t)) {
                    if (count += t - prev; count > window.interval) {
                        const auto & probs = solver.Probs();
                        Sample<Scalar> sample; sample.reserve(probs.size() + 1);
                        sample.emplace_back(t);
                        for (auto p : probs) sample.emplace_back(x[p]);
                        if (verbose) {
                            auto res = sample;
                            auto begin = res.begin(); begin++;
                            std::for_each(begin, res.end(), [](auto & t){ t = generic::unit::Kelvins2Celsius(t); });
                            ECAD_TRACE(generic::fmt::Fmt2Str(res, ","));
                        }
                        samples.emplace_back(std::move(sample));
                        count = 0;
                    }
                    prev = t;
                }
                if (math::GE<Scalar>(t, endT)) lastState = x;
            }
        };
    
        struct Intermidiate
        {
            Scalar refT = 25;
            DenseVector<Scalar> hf;
            DenseVector<Scalar> scen;
            SparseMatrix<Scalar> hfP;
            SparseMatrix<Scalar> htcM;
            SparseMatrix<Scalar> coeff;
            const ThermalNetwork<Scalar> & network;
            std::unordered_map<size_t, size_t> rhs2Nodes;
            Intermidiate(const ThermalNetwork<Scalar> & network, Scalar refT)
                : refT(refT), network(network)
            {
                auto [invC, negG] = makeInvCandNegG(network);
                coeff = invC * negG;

                htcM = invC * makeBondsRhs(network, refT);
                hfP = invC * makeSourceProjMatrix(network, rhs2Nodes);
                scen = DenseVector<Scalar>(rhs2Nodes.size());
                hf = DenseVector<Scalar>(rhs2Nodes.size());
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
            DenseVector<Scalar> hf;
            const Excitation * e{nullptr};
            explicit Solver(const Intermidiate & im, const Excitation * e) : im(im), e(e) { hf = DenseVector<Scalar>(im.hf.size());}
            virtual ~Solver() = default;

            void operator() (const StateType & x, StateType & dxdt, Scalar t)
            {
                using VectorType = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
                Eigen::Map<const VectorType> xM(x.data(), x.size());
                Eigen::Map<VectorType> dxdtM(dxdt.data(), dxdt.size());
                for (int i = 0; i < im.hf.size(); ++i) {
                    Scalar excitation = e ? (*e)(t, im.scen[i]) : 1;
                    hf[i] = im.hf[i] * excitation;
                }
                dxdtM = im.coeff * xM + im.htcM + im.hfP * hf;
            }
        };
        
        ThermalNetworkTransientSolver(const ThermalNetwork<Scalar> & network, Scalar refT, std::vector<size_t> probs)
            : m_refT(refT), m_probs(std::move(probs)), m_network(network)
        {
            m_im.reset(new Intermidiate(m_network, m_refT));
        }

        virtual ~ThermalNetworkTransientSolver() = default;
        size_t StateSize() const { return m_im->StateSize(); }

        template <typename Observer = Sampler, typename Excitation>
        size_t SolveAdaptive(StateType & initState, Scalar t0, Scalar duration, Scalar dt, Scalar absErr, Scalar relErr, Observer observer, const Excitation * e = nullptr)
        {
            if (initState.size() != StateSize()) return 0;
            using namespace boost::numeric::odeint;
            using ErrorStepperType = runge_kutta_dopri5<StateType, Scalar>;
            return integrate_adaptive(make_controlled<ErrorStepperType>(absErr, relErr),
                                    Solver<Excitation>(*m_im, e), initState, Scalar{t0}, Scalar{t0 + duration}, Scalar{dt}, std::move(observer));
        }

        template <typename Observer = Sampler, typename Excitation>
        size_t Solve(StateType & initState, Scalar t0, Scalar duration, Scalar dt, Scalar absErr, Scalar relErr, Observer observer, const Excitation * e = nullptr)
        {
            if (initState.size() != StateSize()) return 0;
            using namespace boost::numeric::odeint;
            using Stepper = modified_midpoint<StateType>;
            return integrate_const(Stepper{}, Solver<Excitation>(*m_im, e), initState, Scalar{t0}, Scalar{t0 + duration}, Scalar{dt}, std::move(observer));
        }

        const std::vector<size_t> Probs() const { return m_probs; }
        const Intermidiate & Im() const { return *m_im; }
    private:
        Scalar m_refT;
        std::vector<size_t> m_probs;
        const ThermalNetwork<Scalar> & m_network;
        std::unique_ptr<Intermidiate> m_im{nullptr};
    };

    template <typename Scalar>
    class ThermalNetworkReducedTransientSolver
    {
    public:
        using StateType = std::vector<Scalar>;    
        struct Intermidiate
        {
            Scalar refT;
            const std::vector<size_t> & probs;
            const ThermalNetwork<Scalar> & network;

            bool includeBonds{true};
            DenseVector<Scalar> uh;
            DenseVector<Scalar> ub;
            DenseMatrix<Scalar> rLT;
            ReducedModel<Scalar> rom;
            DenseMatrix<Scalar> coeff, input;
            Intermidiate(const ThermalNetwork<Scalar> & network, Scalar refT, const std::vector<size_t> & probs, size_t order, const std::string & romLoadFile, const std::string & romSaveFile)
                : refT(refT), probs(probs), network(network)
            {
                bool loadFromFile{false};
                if (generic::fs::FileExists(romLoadFile)) {
                    if (std::ifstream ifs(romLoadFile); ifs.is_open()) {
                        boost::archive::binary_iarchive ia(ifs);
                        boost::serialization::serialize(ia, rom, ecad::toInt(ecad::CURRENT_VERSION));
                        ECAD_TRACE("load rom from file %1%", romLoadFile);
                        loadFromFile = true;
                    }
                }
                if (not loadFromFile) {
                    auto m = makeMNA(network, includeBonds, probs);
                    {
                        tools::ProgressTimer t("reduce");
                        const size_t source = network.Source(includeBonds);
                        rom = Reduce(m, std::max(order, source));
                        ECAD_TRACE("mor %1% -> %2%", rom.x.rows(), rom.x.cols());
                    }
                    if (not romSaveFile.empty()) {
                        generic::fs::CreateDir(generic::fs::DirName(romSaveFile));
                        if (std::ofstream ofs(romSaveFile); ofs.is_open()) {
                            boost::archive::binary_oarchive oa(ofs);
                            boost::serialization::serialize(oa, rom, ecad::toInt(ecad::CURRENT_VERSION));
                            ECAD_TRACE("save rom to file %1%", romSaveFile);
                        }
                    }
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
                using VectorType = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
                x.resize(rom.m.G.cols());
                Eigen::Map<VectorType> xvec(x.data(), x.size());
                Eigen::Map<const VectorType> ivec(in.data(), in.size());
                xvec = rom.xT * ivec;
                return true;
            }

            void State2Output(const StateType & x, StateType & out) const
            {
                using VectorType = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
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
            Scalar endT{0};
            Scalar prev{0};
            Scalar count{0};
            bool verbose{false};
            StateType & lastState;
            TimeWindow<Scalar> window;
            Samples<Scalar> & samples;
            const ThermalNetworkReducedTransientSolver & solver;
            Sampler(const ThermalNetworkReducedTransientSolver & solver, Samples<Scalar> & samples, StateType & lastState, TimeWindow<Scalar> window, Scalar endT, bool verbose)
             : endT(endT), verbose(verbose), lastState(lastState), window(std::move(window)), samples(samples), solver(solver)
            {
                if (not samples.empty())
                    prev = samples.back().front();
                samples.reserve(window.EsitimateSamples());
            }
            virtual ~Sampler() = default;
            void operator() (const StateType & x, Scalar t)
            {
                if (window.isInside(t)) {
                    if (count += t - prev; count > window.interval) {
                        solver.Im().State2Output(x, out);
                        Sample<Scalar> sample; sample.reserve(out.size() + 1);
                        sample.emplace_back(t);
                        for (const auto & o : out)
                            sample.emplace_back(o);
                        if (verbose) {
                            auto res = sample;
                            auto begin = res.begin(); begin++;
                            std::for_each(begin, res.end(), [](auto & t){ t = generic::unit::Kelvins2Celsius(t); });
                            ECAD_TRACE(generic::fmt::Fmt2Str(res, ","));
                        }
                        samples.emplace_back(std::move(sample));
                        count = 0;
                    }
                    prev = t;
                }
                if (math::GE<Scalar>(t, endT)) lastState = x;
            }
        };

        template <typename Excitation>
        struct Solver
        {
            Intermidiate & im;
            const Excitation * e{nullptr};
            explicit Solver(Intermidiate & im, const Excitation * e) : im(im), e(e) {}
            virtual ~Solver() = default;
            void operator() (const StateType & x, StateType & dxdt, Scalar t)
            {
                const size_t nodes = im.network.Size();
                for (size_t i = 0, s = 0; i < nodes; ++i) {
                    const auto & node = im.network[i];
                    Scalar excitation = e ? (*e)(t, node.scen) : 1;
                    if (node.hf != 0 || (im.includeBonds && node.htc != 0))
                        im.uh[s++] = node.hf * excitation + node.htc * im.refT;
                }
                Eigen::Map<DenseVector<Scalar>> result(dxdt.data(), dxdt.size());
                Eigen::Map<const DenseVector<Scalar>> xvec(x.data(), x.size());
                Eigen::Map<DenseVector<Scalar>> dxdtM(dxdt.data(), dxdt.size());
                result = im.coeff * xvec + im.input * im.uh;
                if (not im.includeBonds) result += im.ub;
            }
        };

        ThermalNetworkReducedTransientSolver(const ThermalNetwork<Scalar> & network, Scalar refT, std::vector<size_t> probs, size_t order, const std::string & romLoadFile = {}, const std::string & romSaveFile = {})
            : m_refT(refT), m_probs(std::move(probs)), m_network(network)
        {
            m_im.reset(new Intermidiate(m_network, m_refT, m_probs, order, romLoadFile, romSaveFile));
        }

        virtual ~ThermalNetworkReducedTransientSolver() = default;

        template <typename Observer = Sampler, typename Excitation>
        size_t SolveAdaptive(StateType & initState, Scalar t0, Scalar duration, Scalar dt, Scalar absErr, Scalar relErr, Observer observer, const Excitation * e = nullptr)
        {
            using namespace boost::numeric::odeint;
            using ErrorStepperType = runge_kutta_cash_karp54<StateType, Scalar>;
            return integrate_adaptive(make_controlled<ErrorStepperType>(absErr, relErr),
                                    Solver<Excitation>(*m_im, e), initState, Scalar{t0}, Scalar{t0 + duration}, Scalar{dt}, observer);
        }

        template <typename Observer = Sampler, typename Excitation>
        size_t Solve(StateType & initState, Scalar t0, Scalar duration, Scalar dt, Scalar absErr, Scalar relErr, Observer observer, const Excitation * e = nullptr)
        {
            using namespace boost::numeric::odeint;
            using Stepper = modified_midpoint<StateType>;
            return integrate_const(Stepper{}, Solver<Excitation>(*m_im, e), initState, Scalar{t0}, Scalar{t0 + duration}, Scalar{dt}, std::move(observer));
        }

        const std::vector<size_t> Probs() const { return m_probs; }
        const Intermidiate & Im() const { return *m_im; }
    private:
        Scalar m_refT;
        std::vector<size_t> m_probs;
        const ThermalNetwork<Scalar> & m_network;
        std::unique_ptr<Intermidiate> m_im{nullptr};
    };
} // namespace thermal::solver