 if (not settings.mor) {
        ECAD_EFFICIENCY_TRACK("impedence origin")
        using TransSolver = ThermalNetworkTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;        
        struct Excitation
        {
            EFloat cycle, duty;
            Excitation(EFloat cycle, EFloat duty = 0.5) : cycle(cycle), duty(duty * cycle) {}
            EFloat operator() (EFloat t) const
            {
                return std::fmod(t, cycle) < duty ? 1 : 0.1;
            }
        };

        std::vector<EFloat> cycles;
        for (size_t i = 0; i < 12; ++i)
            cycles.emplace_back(std::pow(10, 0.5 * i - 2.5));
        std::vector<EFloat> dutys {0.01, 0.02, 0.05, 0.1, 0.3, 0.5};
        TransSolver solver(*network, settings.iniT, probs);
        generic::thread::ThreadPool pool(settings.threads);
        std::cout << "available threads: " << pool.Threads() << std::endl;
            
        for (size_t i = 0, index = 0; i < cycles.size(); ++i) {
            for (size_t j = 0; j < dutys.size(); ++j) {
                auto cycle = cycles.at(i);
                auto duty = dutys.at(j);
                pool.Submit(
                    [&] {
                        Excitation excitation(cycle, duty);
                        StateType initState(network->Size(), settings.iniT);
                        EFloat t0 = 0, t1 = std::min<EFloat>(std::max<EFloat>(10, cycle * 50), 20), dt = std::min<EFloat>(0.5 * cycle * duty, 0.1);
                        std::ofstream ofs(fmt::Fmt2Str("./origin_%1%.txt", index));
                        Recorder recorder(solver, ofs, dt * 0.05);
                        solver.Solve(initState, t0, t1, dt, recorder, excitation);
                        ofs.close();
                        std::cout << "done with c: " << cycle << ", d: " << duty << std::endl;
                    }
                );
                index++;
            }
        }
    }  
    else {
        ECAD_EFFICIENCY_TRACK("impedence reduce")
        using TransSolver = ThermalNetworkReducedTransientSolver<EFloat>;
        using StateType = typename TransSolver::StateType;
        using Recorder = typename TransSolver::Recorder;        
        struct Excitation
        {
            EFloat cycle, duty;
            Excitation(EFloat cycle, EFloat duty = 0.5) : cycle(cycle), duty(duty * cycle) {}
            EFloat operator() (EFloat t) const
            {
                return std::fmod(t, cycle) < duty ? 1 : 0.1;
            }
        };

        std::vector<EFloat> cycles;
        for (size_t i = 0; i < 12; ++i)
            cycles.emplace_back(std::pow(10, 0.5 * i - 2.5));
        std::vector<EFloat> dutys {0.01, 0.02, 0.05, 0.1, 0.3, 0.5};
        TransSolver solver(*network, settings.iniT, probs);
        generic::thread::ThreadPool pool(settings.threads);
        std::cout << "available threads: " << pool.Threads() << std::endl;            
        for (size_t i = 0, index = 0; i < cycles.size(); ++i) {
            for (size_t j = 0; j < dutys.size(); ++j) {
                auto cycle = cycles.at(i);
                auto duty = dutys.at(j);
                pool.Submit(
                    [&] {
                        Excitation excitation(cycle, duty);
                        StateType initState(network->Size(), settings.iniT);
                        EFloat t0 = 0, t1 = std::min<EFloat>(std::max<EFloat>(10, cycle * 50), 20), dt = std::min<EFloat>(0.5 * cycle * duty, 0.1);
                        std::ofstream ofs(fmt::Fmt2Str("./reduce_%1%.txt", index));
                        Recorder recorder(solver, ofs, dt * 0.05);
                        solver.Solve(initState, t0, t1, dt, recorder, excitation);
                        ofs.close();
                        std::cout << "done with c: " << cycle << ", d: " << duty << std::endl;
                    }
                );
                index++;
            }
        }
    }  