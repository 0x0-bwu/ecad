#include "solvers/EThermalNetworkSolver.h"
#include "models/thermal/utilities/EThermalModelReduction.h"
#include "thermal/utilities/ThermalNetlistWriter.hpp"
#include "thermal/solver/ThermalNetworkSolver.hpp"
#include "utilities/EThermalNetworkBuilder.h"
#include "generic/thread/ThreadPool.hpp"
#include "EDataMgr.h"
namespace ecad {
namespace esolver {

using namespace emodel;

ECAD_INLINE ESimVal CalculateResidual(const std::vector<ESimVal> & v1, const std::vector<ESimVal> & v2)
{
    ECAD_ASSERT(v1.size() == v2.size());
    ESimVal residual = 0;
    size_t size = v1.size();
    for(size_t i = 0; i < size; ++i) {
        residual += std::fabs(v1[i] - v2[i]) / size;
    }
    return residual;
}

ECAD_INLINE EThermalNetworkSolver::~EThermalNetworkSolver()
{
}

ECAD_INLINE void EThermalNetworkSolver::SetSolveSettings(const EThermalNetworkSolveSettings & settings)
{
    m_settings = settings;
}

ECAD_INLINE EGridThermalNetworkDirectSolver::EGridThermalNetworkDirectSolver(const EGridThermalModel & model)
 : m_model(model)
{
}

ECAD_INLINE EGridThermalNetworkDirectSolver::~EGridThermalNetworkDirectSolver()
{
}

ECAD_INLINE bool EGridThermalNetworkDirectSolver::Solve(ESimVal refT, std::vector<ESimVal> & results)
{
    using namespace thermal::utils;
    using namespace thermal::solver;
    ECAD_EFFICIENCY_TRACK("thermal network solve")

    results.assign(m_model.TotalGrids(), refT);
    bool needIteration = m_model.NeedIteration();
    if(m_settings.iteration > 0 && math::GT<EValue>(m_settings.residual, 0)) {

        EValue residual = m_settings.residual;
        size_t iteration = m_settings.iteration;
        EGridThermalNetworkBuilder builder(m_model);
        do {
            std::vector<ESimVal> lastRes(results);
            auto network = builder.Build(lastRes);
            if (nullptr == network) return false;

            if (not m_settings.spiceFile.empty()) {
                auto spiceWriter = ThermalNetlistWriter(*network);
                spiceWriter.SetReferenceTemperature(refT);
                auto res = spiceWriter.WriteSpiceNetlist(m_settings.spiceFile);
                if (res) std::cout << "write out spice file " << m_settings.spiceFile << " successfully!" << std::endl;
            }
            std::cout << "total nodes: " << network->Size() << std::endl;
            std::cout << "intake  heat flow: " << builder.summary.iHeatFlow << "w" << std::endl;
            std::cout << "outtake heat flow: " << builder.summary.oHeatFlow << "w" << std::endl;
            
            ThermalNetworkSolver<ESimVal> solver(*network, EDataMgr::Instance().DefaultThreads());
            solver.Solve(refT);

            const auto & nodes = network->GetNodes();
            for (size_t n = 0; n < nodes.size(); ++n)
                results[n] = nodes[n].t;

            size_t maxId = std::distance(results.begin(), std::max_element(results.begin(), results.end()));
            std::cout << "hotspot: " << maxId << std::endl;
            if (false) {
                ECAD_EFFICIENCY_TRACK("transient mor")
                using TransSolver = ThermalNetworkReducedTransientSolver<ESimVal>;
                using StateType = typename TransSolver::StateType;
                size_t threads = 1;//EDataMgr::Instance().DefaultThreads();
                size_t sourceSize = network->SourceSize(refT);
                std::cout << "source size: " << sourceSize << std::endl;
                auto in = typename TransSolver::Input(*network, ESimVal{refT}, sourceSize, threads);
                
                std::vector<size_t> probs{maxId};
                std::ofstream out(std::filesystem::path(m_settings.spiceFile).parent_path().string() + "/trans2.out");
                auto recorder = typename TransSolver::Recorder(out, &in, probs, 0.0001);
                StateType state(in.StateSize());
                StateType initT(network->Size(), refT);
                Eigen::Map<const Eigen::Matrix<ESimVal, Eigen::Dynamic, 1>> init(initT.data(), initT.size(), 1);
                Eigen::Map<Eigen::Matrix<ESimVal, Eigen::Dynamic, 1>> result(state.data(), state.size(), 1);
                result = in.x.colPivHouseholderQr().solve(init);
                // result = in.x.transpose() * init;
                // std::cout << "init State: \n" << result << std::endl;//wbtest
                using namespace boost::numeric::odeint;
                using ErrorStepperType = runge_kutta_cash_karp54<StateType>;
                size_t totalSteps = integrate_adaptive(make_controlled(1e-12, 1e-10,ErrorStepperType{}),
                                        TransSolver(&in), initT, 0.0, 10.0, 0.01, recorder);
                
                std::cout << "integrate step: " << totalSteps << std::endl;
                out.close();
            }

            if (false) {
                ECAD_EFFICIENCY_TRACK("transient origin")
                using TransSolver = ThermalNetworkTransientSolver<ESimVal>;
                using StateType = typename TransSolver::StateType;
                using Jacobi = typename TransSolver::Jacobi;
                size_t threads = 1;//EDataMgr::Instance().DefaultThreads();
                auto in = typename TransSolver::Input(*network, refT, threads);

                std::vector<size_t> probs{maxId};
                std::ofstream out(std::filesystem::path(m_settings.spiceFile).parent_path().string() + "/trans.out");
                auto recorder = typename TransSolver::Recorder(out, probs, 0.0001);
                
                using namespace boost::numeric::odeint;
                StateType initT(nodes.size(), refT);
                // size_t totalSteps = integrate(TransSolver(&in), initT, 0.0, 10.0, 0.01, recorder);
                using ErrorStepperType = runge_kutta_cash_karp54<StateType>;
                size_t totalSteps = integrate_adaptive(make_controlled(1e-12, 1e-10, ErrorStepperType{}),
                                        TransSolver(&in), initT, 0.0, 10.0, 0.1, recorder);


                // size_t totalSteps = integrate_const(runge_kutta4<StateType>(), TransSolver(&in), initT, 0.0, 10.0, 0.01, recorder);
                // size_t totalSteps = integrate_const(make_dense_output<rosenbrock4<ESimVal>>(1.0e-6 , 1.0e-6),
                //                         std::make_pair(TransSolver(&in), Jacobi(&in)), initT, 0.0, 10.0, 0.01, recorder);

                std::cout << "integrate step: " << totalSteps << std::endl;
                out.close();
            }

            iteration -= 1;
            if(!needIteration || iteration == 0) break;

            residual = CalculateResidual(results, lastRes);
            needIteration = math::GE(residual, m_settings.residual);
            std::cout << "Residual: " << residual << ", Remain Iteration: " << (needIteration ? iteration : 0) << std::endl;
    
        } while (needIteration);
    }

    return true;
}

ECAD_INLINE EGridThermalNetworkIterativeSolver::EGridThermalNetworkIterativeSolver(const EGridThermalModel & model, std::vector<ESimVal> iniT)
 : m_model(model), m_iniT(std::move(iniT))
{
}

ECAD_INLINE EGridThermalNetworkIterativeSolver::~EGridThermalNetworkIterativeSolver()
{
}

ECAD_INLINE bool EGridThermalNetworkIterativeSolver::Solve(ESimVal refT, std::vector<ESimVal> & results)
{
    if(m_model.TotalGrids() != m_iniT.size()) return false;

    if(m_settings.iteration > 0 && math::GT<EValue>(m_settings.residual, 0)) {
        EValue residual = m_settings.residual;
        // size_t iteration = m_settings.iteration;
        size_t iteration = std::numeric_limits<size_t>::max();
        do {
            SolveOneIteration(refT, results);            
            residual = CalculateResidual(m_iniT, results);
            iteration -= 1;

            std::cout << "Iterative Residual: " << residual << ", Remain Iteration: ";
            std::cout << (math::GE(residual, m_settings.residual) ? iteration : 0) << std::endl;

            m_iniT = results;
        } while(iteration != 0 && math::GE(residual, m_settings.residual));
    }
    return true;
}

ECAD_INLINE void EGridThermalNetworkIterativeSolver::SolveOneIteration(ESimVal refT, std::vector<ESimVal> & results)
{
    bool multiThreads = true;
    results.resize(m_iniT.size());
    auto lyrs = m_model.TotalLayers();
    size_t threads = EDataMgr::Instance().DefaultThreads();
    if(!multiThreads || threads == 1) {
        for(size_t z = 0; z < lyrs; ++z)
            SolveOneLayer(z, refT, results);
    }
    else {
        generic::thread::ThreadPool pool(threads);
        for(size_t z = 0; z < lyrs; ++z)
            pool.Submit(std::bind(&EGridThermalNetworkIterativeSolver::SolveOneLayer, this, z, refT, std::ref(results)));
    }
}

ECAD_INLINE void EGridThermalNetworkIterativeSolver::SolveOneLayer(size_t z, ESimVal refT, std::vector<ESimVal> & results)
{
    EGridThermalNetworkBuilder builder(m_model);

    //bc
    SPtr<EGridBCModel> topBC = nullptr, botBC = nullptr;
    m_model.GetTopBotBCModel(topBC, botBC);

    EGridThermalModel::BCType topT, botT;
    m_model.GetTopBotBCType(topT, botT);

    using namespace thermal::model;
    using namespace thermal::solver;
    using Orient = typename EGridThermalNetworkBuilder::Orientation;

    auto getT = [this](const ESize3D & grid) {
        size_t i = m_model.GetFlattenIndex(grid);
        return m_iniT[i];
    };

    size_t node;
    size_t index;
    bool success;
    ESimVal temperature;
    auto pwrModel = m_model.GetPowerModel(z);
    pwrModel = nullptr;//wbtest
    topBC = nullptr;//wbtest
    botBC = nullptr;//wbtest
    auto xGridArea = builder.GetXGridArea(z);
    auto yGridArea = builder.GetYGridArea(z);
    auto halfXGridLen = 0.5 * builder.GetXGridLength();
    auto halfYGridLen = 0.5 * builder.GetYGridLength();
    auto size = utils::detail::Reduce(m_model.GridSize(), utils::ReduceIndexMethod::Ceil);
    for(size_t x = 0; x < size.x; ++x) {
        for(size_t y = 0; y < size.y; ++y) {
            ESize2D ll = EGridThermalNetworkReductionSolver::LowLeftIndexFromReducedModelIndex(ESize2D(x, y));
            ESize2D lr = ESize2D(ll.x + 1, ll.y);
            ESize2D ul = ESize2D(ll.x, ll.y + 1);
            ESize2D ur = ESize2D(ll.x + 1, ll.y + 1);
            if(!m_model.isValid(lr) && !m_model.isValid(ul)) {
                ESize3D grid(ll, z);
                temperature = getT(grid);
                auto k = builder.GetCompositeMatK(grid, temperature);

                ThermalNetwork<ESimVal> network(1);
                
                //power
                if(pwrModel) {
                    auto p = pwrModel->Query(temperature, grid.x, grid.y, &success);
                    if(success) network.SetHF(0, p);
                }

                //bc
                if(z == 0 && topBC) {
                    auto val = topBC->Query(temperature, grid.x, grid.y, &success);
                    if(success) {
                        switch(topT) {
                            case EGridThermalModel::BCType::HTC : {
                                network.SetHTC(0, builder.GetZGridArea() * val);
                                break;
                            }
                            case EGridThermalModel::BCType::HeatFlow : {
                                network.SetHF(0, val);
                                break;
                            }
                            case EGridThermalModel::BCType::Temperature : {
                                network.SetT(0, val);
                                break;
                            }
                        }
                    }
                }

                if(z == (m_model.TotalLayers() - 1) && botBC) {
                    auto val = botBC->Query(temperature, grid.x, grid.y, &success);
                    if(success) {
                        switch(botT) {
                            case EGridThermalModel::BCType::HTC : {
                                network.SetHTC(0, builder.GetZGridArea() * val);
                                break;
                            }
                            case EGridThermalModel::BCType::HeatFlow : {
                                network.SetHF(0, val);
                                break;
                            }
                            case EGridThermalModel::BCType::Temperature : {
                                network.SetT(0, val);
                                break;
                            }
                        }
                    }
                }
                //left
                auto left = builder.GetNeighbor(grid, Orient::Left);
                if(m_model.isValid(left)) {
                    //0
                    temperature = getT(left);
                    auto kL0 = builder.GetCompositeMatK(left, temperature);
                    auto r0L = builder.GetRes(k[0], halfXGridLen, kL0[0], halfXGridLen, xGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(0, node, r0L);
                }

                //front
                auto front = builder.GetNeighbor(grid, Orient::Front);
                if(m_model.isValid(front)) {
                    //0
                    temperature = getT(front);
                    auto kF0 = builder.GetCompositeMatK(front, temperature);
                    auto r0F = builder.GetRes(k[1], halfYGridLen, kF0[1], halfYGridLen, yGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(0, node, r0F);
                }

                // //top
                // auto top = builder.GetNeighbor(grid, Orient::Top);
                // if(m_model.isValid(top)) {
                //     temperature = getT(top);
                //     auto zN = builder.GetZGridLength(z);
                //     auto zT = builder.GetZGridLength(top.z);
                //     auto kTN = builder.GetConductingMatK(top, temperature);
                //     auto rNT = builder.GetR(k[2], zN, kTN[2], zT, builder.GetZGridArea());
                //     node = network.AppendNode(temperature);
                //     network.SetR(0, node, rNT);
                // }

                // //bot
                // auto bot = builder.GetNeighbor(grid, Orient::Bot);
                // if(m_model.isValid(bot)) {
                //     temperature = getT(bot);
                //     auto zN = builder.GetZGridLength(z);
                //     auto zB = builder.GetZGridLength(bot.z);
                //     auto kBN = builder.GetConductingMatK(bot, temperature);
                //     auto rNB = builder.GetR(k[2], zN, kBN[2], zB, builder.GetZGridArea());
                //     node = network.AppendNode(temperature);
                //     network.SetR(0, node, rNB);
                // }
                ThermalNetworkSolver solver(network);
                solver.SolveEigen(refT);

                const auto & nodes = network.GetNodes();
                index = m_model.GetFlattenIndex(grid);
                results[index] = nodes.front().t;
            }
            else if(m_model.isValid(lr) && !m_model.isValid(ul)) {
                std::array<ESize2D, 2> tiles {ll, lr};
                std::array<std::array<ESimVal, 3>, 2> k;
                for(size_t n = 0; n < k.size(); ++n) {
                    ESize3D grid(tiles[n], z);
                    temperature = getT(grid);
                    k[n] = builder.GetCompositeMatK(grid, temperature);
                }

                ThermalNetwork<ESimVal> network(2);

                //power
                if(pwrModel) {
                    for(size_t n = 0; n < tiles.size(); ++n) {
                        const auto & curr = tiles[n];
                        temperature = getT(ESize3D(curr, z));
                        auto p = pwrModel->Query(temperature, curr.x, curr.y, &success);
                        if(success) network.SetHF(n, p);
                    }
                }

                //bc
                if(z == 0 && topBC) {
                    for(size_t n = 0; n < tiles.size(); ++n) {
                        const auto & curr = tiles[n];
                        temperature = getT(ESize3D(curr, z));
                        auto val = topBC->Query(temperature, curr.x, curr.y, &success);
                        if(!success) continue;
                        switch(topT) {
                            case EGridThermalModel::BCType::HTC : {
                                network.SetHTC(n, builder.GetZGridArea() * val);
                                break;
                            }
                            case EGridThermalModel::BCType::HeatFlow : {
                                network.SetHF(n, val);
                                break;
                            }
                            case EGridThermalModel::BCType::Temperature : {
                                network.SetT(n, val);
                                break;
                            }
                        }
                    }
                }

                if(z == (m_model.TotalLayers() - 1) && botBC) {
                    for(size_t n = 0; n < tiles.size(); ++n) {
                        const auto & curr = tiles[n];
                        temperature = getT(ESize3D(curr, z));
                        auto val = botBC->Query(temperature, curr.x, curr.y, &success);
                        if(!success) continue;
                        switch(botT) {
                            case EGridThermalModel::BCType::HTC : {
                                network.SetHTC(n, builder.GetZGridArea() * val);
                                break;
                            }
                            case EGridThermalModel::BCType::HeatFlow : {
                                network.SetHF(n, val);
                                break;
                            }
                            case EGridThermalModel::BCType::Temperature : {
                                network.SetT(n, val);
                                break;
                            }
                        }
                    }
                }

                //r
                auto r01 = builder.GetRes(k[0][0], halfXGridLen, k[1][0], halfXGridLen, xGridArea);
                network.SetR(0, 1, r01);

                //left
                auto left = builder.GetNeighbor(ESize3D(ll, z), Orient::Left);
                if(m_model.isValid(left)) {
                    //0
                    temperature = getT(left);
                    auto kL0 = builder.GetCompositeMatK(left, temperature);
                    auto r0L = builder.GetRes(k[0][0], halfXGridLen, kL0[0], halfXGridLen, xGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(0, node, r0L);
                }

                //front
                auto front = builder.GetNeighbor(ESize3D(ll, z), Orient::Front);
                if(m_model.isValid(front)) {
                    //0
                    temperature = getT(front);
                    auto kF0 = builder.GetCompositeMatK(front, temperature);
                    auto r0F = builder.GetRes(k[0][1], halfYGridLen, kF0[1], halfYGridLen, yGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(0, node, r0F);

                    //1
                    front = builder.GetNeighbor(ESize3D(lr, z), Orient::Front);
                    temperature = getT(front);
                    auto kF1 = builder.GetCompositeMatK(front, temperature);
                    auto r1F = builder.GetRes(k[1][1], halfYGridLen, kF1[1], halfYGridLen, yGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(1, node, r1F);
                }

                //right
                auto right = builder.GetNeighbor(ESize3D(lr, z), Orient::Right);
                if(m_model.isValid(right)) {
                    //1
                    temperature = getT(right);
                    auto kR1 = builder.GetCompositeMatK(right, temperature);
                    auto r1R = builder.GetRes(k[1][0], halfXGridLen, kR1[0], halfXGridLen, xGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(1, node, r1R);
                }

                // //top
                // auto top = builder.GetNeighbor(ESize3D(ll, z), Orient::Top);
                // if(m_model.isValid(top)) {
                //     for(size_t n = 0; n < tiles.size(); ++n) {
                //         const auto & curr = tiles[n];
                //         top = builder.GetNeighbor(ESize3D(curr, z), Orient::Top);
                //         temperature = getT(top);
                //         auto zN = builder.GetZGridLength(z);
                //         auto zT = builder.GetZGridLength(top.z);
                //         auto kTN = builder.GetConductingMatK(top, temperature);
                //         auto rNT = builder.GetR(k[n][2], zN, kTN[2], zT, builder.GetZGridArea());
                //         node = network.AppendNode(temperature);
                //         network.SetR(n, node, rNT);
                //     }
                // }

                // //bot
                // auto bot = builder.GetNeighbor(ESize3D(ll, z), Orient::Bot);
                // if(m_model.isValid(bot)) {
                //     for(size_t n = 0; n < tiles.size(); ++n) {
                //         const auto & curr = tiles[n];
                //         bot = builder.GetNeighbor(ESize3D(curr, z), Orient::Bot);
                //         temperature = getT(bot);
                //         auto zN = builder.GetZGridLength(z);
                //         auto zB = builder.GetZGridLength(bot.z);
                //         auto kBN = builder.GetConductingMatK(bot, temperature);
                //         auto rNB = builder.GetR(k[n][2], zN, kBN[2], zB, builder.GetZGridArea());
                //         node = network.AppendNode(temperature);
                //         network.SetR(n, node, rNB);
                //     }
                // }
                ThermalNetworkSolver solver(network);
                solver.SolveEigen(refT);

                const auto & nodes = network.GetNodes();
                for(size_t n = 0; n < tiles.size(); ++n) {
                    size_t index = m_model.GetFlattenIndex(ESize3D(tiles[n], z));
                    results[index] = nodes[n].t;
                }
            }
            else if(!m_model.isValid(lr) && m_model.isValid(ul)) {
                std::array<ESize2D, 2> tiles {ll, ul};
                std::array<std::array<ESimVal, 3>, 2> k;
                for(size_t n = 0; n < k.size(); ++n) {
                    ESize3D grid(tiles[n], z);
                    temperature = getT(grid);
                    k[n] = builder.GetCompositeMatK(grid, temperature);
                }

                ThermalNetwork<ESimVal> network(2);
                //power
                if(pwrModel) {
                    for(size_t n = 0; n < tiles.size(); ++n) {
                        const auto & curr = tiles[n];
                        temperature = getT(ESize3D(curr, z));
                        auto p = pwrModel->Query(temperature, curr.x, curr.y, &success);
                        if(success) network.SetHF(n, p);
                    }
                }

                //bc
                if(z == 0 && topBC) {
                    for(size_t n = 0; n < tiles.size(); ++n) {
                        const auto & curr = tiles[n];
                        temperature = getT(ESize3D(curr, z));
                        auto val = topBC->Query(temperature, curr.x, curr.y, &success);
                        if(!success) continue;
                        switch(topT) {
                            case EGridThermalModel::BCType::HTC : {
                                network.SetHTC(n, builder.GetZGridArea() * val);
                                break;
                            }
                            case EGridThermalModel::BCType::HeatFlow : {
                                network.SetHF(n, val);
                                break;
                            }
                            case EGridThermalModel::BCType::Temperature : {
                                network.SetT(n, val);
                                break;
                            }
                        }
                    }
                }

                if(z == (m_model.TotalLayers() - 1) && botBC) {
                    for(size_t n = 0; n < tiles.size(); ++n) {
                        const auto & curr = tiles[n];
                        temperature = getT(ESize3D(curr, z));
                        auto val = botBC->Query(temperature, curr.x, curr.y, &success);
                        if(!success) continue;
                        switch(botT) {
                            case EGridThermalModel::BCType::HTC : {
                                network.SetHTC(n, builder.GetZGridArea() * val);
                                break;
                            }
                            case EGridThermalModel::BCType::HeatFlow : {
                                network.SetHF(n, val);
                                break;
                            }
                            case EGridThermalModel::BCType::Temperature : {
                                network.SetT(n, val);
                                break;
                            }
                        }
                    }
                }

                //r
                auto r01 = builder.GetRes(k[0][1], halfYGridLen, k[1][1], halfYGridLen, yGridArea);
                network.SetR(0, 1, r01);
                
                //left
                auto left = builder.GetNeighbor(ESize3D(ll, z), Orient::Left);
                if(m_model.isValid(left)) {
                    //0
                    temperature = getT(left);
                    auto kL0 = builder.GetCompositeMatK(left, temperature);
                    auto r0L = builder.GetRes(k[0][0], halfXGridLen, kL0[0], halfXGridLen, xGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(0, node, r0L);
                    //1
                    left = builder.GetNeighbor(ESize3D(ul, z), Orient::Left);
                    temperature = getT(left);
                    auto kL1 = builder.GetCompositeMatK(left, temperature);
                    auto r1L = builder.GetRes(k[1][0], halfXGridLen, kL1[0], halfXGridLen, xGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(1, node, r1L);
                }

                //front
                auto front = builder.GetNeighbor(ESize3D(ll, z), Orient::Front);
                if(m_model.isValid(front)) {
                    //0
                    temperature = getT(front);
                    auto kF0 = builder.GetCompositeMatK(front, temperature);
                    auto r0F = builder.GetRes(k[0][1], halfYGridLen, kF0[1], halfYGridLen, yGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(0, node, r0F);
                }
                //end
                auto end = builder.GetNeighbor(ESize3D(ul, z), Orient::End);
                if(m_model.isValid(end)) {
                    //1
                    temperature = getT(end);
                    auto kE1 = builder.GetCompositeMatK(end, temperature);
                    auto r1E = builder.GetRes(k[1][1], halfYGridLen, kE1[1], halfYGridLen, yGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(2, node, r1E);
                }

                // //top
                // auto top = builder.GetNeighbor(ESize3D(ll, z), Orient::Top);
                // if(m_model.isValid(top)) {
                //     for(size_t n = 0; n < tiles.size(); ++n) {
                //         const auto & curr = tiles[n];
                //         top = builder.GetNeighbor(ESize3D(curr, z), Orient::Top);
                //         temperature = getT(top);
                //         auto zN = builder.GetZGridLength(z);
                //         auto zT = builder.GetZGridLength(top.z);
                //         auto kTN = builder.GetConductingMatK(top, temperature);
                //         auto rNT = builder.GetR(k[n][2], zN, kTN[2], zT, builder.GetZGridArea());
                //         node = network.AppendNode(temperature);
                //         network.SetR(n, node, rNT);
                //     }
                // }

                // //bot
                // auto bot = builder.GetNeighbor(ESize3D(ll, z), Orient::Bot);
                // if(m_model.isValid(bot)) {
                //     for(size_t n = 0; n < tiles.size(); ++n) {
                //         const auto & curr = tiles[n];
                //         bot = builder.GetNeighbor(ESize3D(curr, z), Orient::Bot);
                //         temperature = getT(bot);
                //         auto zN = builder.GetZGridLength(z);
                //         auto zB = builder.GetZGridLength(bot.z);
                //         auto kBN = builder.GetConductingMatK(bot, temperature);
                //         auto rNB = builder.GetR(k[n][2], zN, kBN[2], zB, builder.GetZGridArea());
                //         node = network.AppendNode(temperature);
                //         network.SetR(n, node, rNB);
                //     }
                // }
                ThermalNetworkSolver solver(network);
                solver.SolveEigen(refT);

                const auto & nodes = network.GetNodes();
                for(size_t n = 0; n < tiles.size(); ++n) {
                    size_t index = m_model.GetFlattenIndex(ESize3D(tiles[n], z));
                    results[index] = nodes[n].t;
                }
            }
            else {
                std::array<ESize2D, 4> tiles {ll, lr, ul, ur};
                std::array<std::array<ESimVal, 3>, 4> k;
                for(size_t n = 0; n < k.size(); ++n) {
                    ESize3D grid(tiles[n], z);
                    temperature = getT(grid);
                    k[n] = builder.GetCompositeMatK(grid, temperature);
                }

                ThermalNetwork<ESimVal> network(4);
                
                //power
                if(pwrModel) {
                    for(size_t n = 0; n < tiles.size(); ++n) {
                        const auto & curr = tiles[n];
                        temperature = getT(ESize3D(curr, z));
                        auto p = pwrModel->Query(temperature, curr.x, curr.y, &success);
                        if(success) network.SetHF(n, p);
                    }
                }

                //bc
                if(z == 0 && topBC) {
                    for(size_t n = 0; n < tiles.size(); ++n) {
                        const auto & curr = tiles[n];
                        temperature = getT(ESize3D(curr, z));
                        auto val = topBC->Query(temperature, curr.x, curr.y, &success);
                        if(!success) continue;
                        switch(topT) {
                            case EGridThermalModel::BCType::HTC : {
                                network.SetHTC(n, builder.GetZGridArea() * val);
                                break;
                            }
                            case EGridThermalModel::BCType::HeatFlow : {
                                network.SetHF(n, val);
                                break;
                            }
                            case EGridThermalModel::BCType::Temperature : {
                                network.SetT(n, val);
                                break;
                            }
                        }
                    }
                }

                if(z == (m_model.TotalLayers() - 1) && botBC) {
                    for(size_t n = 0; n < tiles.size(); ++n) {
                        const auto & curr = tiles[n];
                        temperature = getT(ESize3D(curr, z));
                        auto val = botBC->Query(temperature, curr.x, curr.y, &success);
                        if(!success) continue;
                        switch(botT) {
                            case EGridThermalModel::BCType::HTC : {
                                network.SetHTC(n, builder.GetZGridArea() * val);
                                break;
                            }
                            case EGridThermalModel::BCType::HeatFlow : {
                                network.SetHF(n, val);
                                break;
                            }
                            case EGridThermalModel::BCType::Temperature : {
                                network.SetT(n, val);
                                break;
                            }
                        }
                    }
                }

                //r
                auto r01 = builder.GetRes(k[0][0], halfXGridLen, k[1][0], halfXGridLen, xGridArea);
                network.SetR(0, 1, r01);

                auto r02 = builder.GetRes(k[0][1], halfYGridLen, k[2][1], halfYGridLen, yGridArea);
                network.SetR(0, 2, r02);

                auto r13 = builder.GetRes(k[1][1], halfYGridLen, k[3][1], halfYGridLen, yGridArea);
                network.SetR(1, 3, r13);

                auto r23 = builder.GetRes(k[2][0], halfXGridLen, k[3][0], halfXGridLen, xGridArea);
                network.SetR(2, 3, r23);
                
                //left
                auto left = builder.GetNeighbor(ESize3D(ll, z), Orient::Left);
                if(m_model.isValid(left)) {
                    //0
                    temperature = getT(left);
                    auto kL0 = builder.GetCompositeMatK(left, temperature);
                    auto r0L = builder.GetRes(k[0][0], halfXGridLen, kL0[0], halfXGridLen, xGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(0, node, r0L);
                    //2
                    left = builder.GetNeighbor(ESize3D(ul, z), Orient::Left);
                    temperature = getT(left);
                    auto kL2 = builder.GetCompositeMatK(left, temperature);
                    auto r2L = builder.GetRes(k[2][0], halfXGridLen, kL2[0], halfXGridLen, xGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(2, node, r2L);
                }

                //front
                auto front = builder.GetNeighbor(ESize3D(ll, z), Orient::Front);
                if(m_model.isValid(front)) {
                    //0
                    temperature = getT(front);
                    auto kF0 = builder.GetCompositeMatK(front, temperature);
                    auto r0F = builder.GetRes(k[0][1], halfYGridLen, kF0[1], halfYGridLen, yGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(0, node, r0F);

                    //1
                    front = builder.GetNeighbor(ESize3D(lr, z), Orient::Front);
                    temperature = getT(front);
                    auto kF1 = builder.GetCompositeMatK(front, temperature);
                    auto r1F = builder.GetRes(k[1][1], halfYGridLen, kF1[1], halfYGridLen, yGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(1, node, r1F);
                }

                //right
                auto right = builder.GetNeighbor(ESize3D(lr, z), Orient::Right);
                if(m_model.isValid(right)) {
                    //1
                    temperature = getT(right);
                    auto kR1 = builder.GetCompositeMatK(right, temperature);
                    auto r1R = builder.GetRes(k[1][0], halfXGridLen, kR1[0], halfXGridLen, xGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(1, node, r1R);
                    
                    //3
                    right = builder.GetNeighbor(ESize3D(ur, z), Orient::Right);
                    temperature = getT(right);
                    auto kR3 = builder.GetCompositeMatK(right, temperature);
                    auto r3R = builder.GetRes(k[3][0], halfXGridLen, kR3[0], halfXGridLen, xGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(3, node, r3R);
                }

                //end
                auto end = builder.GetNeighbor(ESize3D(ul, z), Orient::End);
                if(m_model.isValid(end)) {
                    //2
                    temperature = getT(end);
                    auto kE2 = builder.GetCompositeMatK(end, temperature);
                    auto r2E = builder.GetRes(k[2][1], halfYGridLen, kE2[1], halfYGridLen, yGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(2, node, r2E);

                    //3
                    end = builder.GetNeighbor(ESize3D(ur, z), Orient::End);
                    temperature = getT(end);
                    auto kE3 = builder.GetCompositeMatK(end, temperature);
                    auto r3E = builder.GetRes(k[3][1], halfYGridLen, kE3[1], halfYGridLen, yGridArea);
                    node = network.AppendNode(temperature);
                    network.SetR(3, node, r3E);
                }

                // //top
                // auto top = builder.GetNeighbor(ESize3D(ll, z), Orient::Top);
                // if(m_model.isValid(top)) {
                //     for(size_t n = 0; n < tiles.size(); ++n) {
                //         const auto & curr = tiles[n];
                //         top = builder.GetNeighbor(ESize3D(curr, z), Orient::Top);
                //         temperature = getT(top);
                //         auto zN = builder.GetZGridLength(z);
                //         auto zT = builder.GetZGridLength(top.z);
                //         auto kTN = builder.GetConductingMatK(top, temperature);
                //         auto rNT = builder.GetR(k[n][2], zN, kTN[2], zT, builder.GetZGridArea());
                //         node = network.AppendNode(temperature);
                //         network.SetR(n, node, rNT);
                //     }
                // }

                // //bot
                // auto bot = builder.GetNeighbor(ESize3D(ll, z), Orient::Bot);
                // if(m_model.isValid(bot)) {
                //     for(size_t n = 0; n < tiles.size(); ++n) {
                //         const auto & curr = tiles[n];
                //         bot = builder.GetNeighbor(ESize3D(curr, z), Orient::Bot);
                //         temperature = getT(bot);
                //         auto zN = builder.GetZGridLength(z);
                //         auto zB = builder.GetZGridLength(bot.z);
                //         auto kBN = builder.GetConductingMatK(bot, temperature);
                //         auto rNB = builder.GetR(k[n][2], zN, kBN[2], zB, builder.GetZGridArea());
                //         node = network.AppendNode(temperature);
                //         network.SetR(n, node, rNB);
                //     }
                // }
                ThermalNetworkSolver solver(network);
                solver.SolveEigen(refT);

                const auto & nodes = network.GetNodes();
                for(size_t n = 0; n < tiles.size(); ++n) {
                    size_t index = m_model.GetFlattenIndex(ESize3D(tiles[n], z));
                    results[index] = nodes[n].t;
                }
            }
        }
    }
}

ECAD_INLINE EGridThermalNetworkReductionSolver::EGridThermalNetworkReductionSolver(const EGridThermalModel & model, size_t reduceOrder)
 : m_model(model), m_reduceOrder(reduceOrder)
{
}

ECAD_INLINE EGridThermalNetworkReductionSolver::~EGridThermalNetworkReductionSolver()
{
}

ECAD_INLINE bool EGridThermalNetworkReductionSolver::Solve(ESimVal refT, std::vector<ESimVal> & results)
{
    return SolveRecursively(m_model, refT, results, m_reduceOrder);
}

ECAD_INLINE bool EGridThermalNetworkReductionSolver::SolveRecursively(const EGridThermalModel & model, ESimVal refT, std::vector<ESimVal> & results, size_t reduceOrder)
{
    if(reduceOrder == 0)
        return SolveDirectly(model, refT, results);

    reduceOrder--;
    auto reduced = etherm::utils::makeReductionModel(model);
    if(nullptr == reduced)
        return SolveDirectly(model, refT, results);

    if(!SolveRecursively(*reduced, refT, results, reduceOrder)) return false;

    auto size = model.ModelSize();
    std::cout << "reduction model size: (" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
    std::cout << "reduction model total nodes: " << model.TotalGrids() << std::endl;

    std::vector<ESimVal> iniT(model.TotalGrids());
    for(size_t i = 0; i < iniT.size(); ++i) {
        auto grid = model.GetGridIndex(i);
        auto rd2d = ReducedModelIndexFromModelIndex(ESize2D(grid.x, grid.y));
        iniT[i] = results[reduced->GetFlattenIndex(ESize3D(rd2d, grid.z))];
    }

    EGridThermalNetworkIterativeSolver solver(model, std::move(iniT));
    solver.SetSolveSettings(m_settings);
    return solver.Solve(refT, results);
}

ECAD_INLINE bool EGridThermalNetworkReductionSolver::SolveDirectly(const EGridThermalModel & model, ESimVal refT, std::vector<ESimVal> & results)
{
    auto size = model.ModelSize();
    std::cout << "direct model size: (" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
    std::cout << "direct model total nodes: " << model.TotalGrids() << std::endl;

    EGridThermalNetworkDirectSolver solver(model);
    solver.SetSolveSettings(m_settings);
    return solver.Solve(refT, results);
}

ECAD_INLINE ESize2D EGridThermalNetworkReductionSolver::LowLeftIndexFromReducedModelIndex(const ESize2D & index)
{
    return ESize2D(index.x * 2, index.y * 2);
}

ECAD_INLINE ESize2D EGridThermalNetworkReductionSolver::ReducedModelIndexFromModelIndex(const ESize2D & index)
{
    return ESize2D(index.x / 2, index.y / 2);
}



}//namespace esolver
}//namespace ecad