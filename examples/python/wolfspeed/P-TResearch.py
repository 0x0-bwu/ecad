import os
import sys
import random
import SetupDesign
import StaticThermal
from SetupDesign import ecad

mgr = ecad.DataMgr

def static_p_t_try_run(chip_locations, work_dir) :
    def setup_power(layout) :
        def get_random_power_config(mean, sigma) :
            return random.gauss(mean, sigma)
        power_config = []
        cell_insts = ["TopBridge1", "TopBridge2"]
        diode_comps = ["Diode1", "Diode2", "Diode3"]
        sic_comps = ["Die1", "Die2", "Die3"]
        for cell_inst in cell_insts :
            for sic_comp in sic_comps :
                name = cell_inst + mgr.hier_sep() + sic_comp
                comp = layout.find_component_by_name(name)
                assert(comp)
                power_config.append(get_random_power_config(124, 10))
                comp.set_loss_power(25, power_config[-1])

        for cell_inst in cell_insts :
            for diode_comp in diode_comps :
                name = cell_inst + mgr.hier_sep() + diode_comp
                comp = layout.find_component_by_name(name)
                assert(comp)
                power_config.append(get_random_power_config(21, 0))
                comp.set_loss_power(25, power_config[-1])
        return power_config
        
    layout = SetupDesign.setup_design("CAS300M12BM2", chip_locations, work_dir, False)
    power_config = setup_power(layout)
    setup = StaticThermal.get_simulation_setup(layout, work_dir, ["TopBridge1", "TopBridge2"], ["Die1", "Die2", "Die3"], True)
    temperatures = layout.run_thermal_simulation(setup)

    return power_config[0:6] + temperatures[2]

def static_p_t_research(index, work_dir) :
    mgr.set_threads(1)
    sub_work_dir = work_dir + '/' + str(int(index) + 1)
    results = static_p_t_try_run(SetupDesign.chip_locations, sub_work_dir)
    with open(sub_work_dir + '/results.txt', 'w') as f :
        values = [str(value) for value in results]
        f.write(','.join(values))
        f.write('\n')
    f.close()

def main() :
    
    work_dir = os.path.dirname(__file__) + '/data/research'
    def collect_results(total) :

        sum_f = open(work_dir + '/summary.txt', 'w')
        for i in range(total) :
            filename = work_dir + f'/{i + 1}/results.txt'
            with open(filename) as f :
                for line in f :
                    line = line.strip()
                    if line :
                        sum_f.write(f'{line}\n')
        sum_f.close()

    total = 10
    core_num = min(total, 20)
    from multiprocessing import Pool
    with Pool(core_num) as pool:
        pool.starmap(static_p_t_research, [(i, work_dir) for i in range(total)])
    collect_results(total)

if __name__ == '__main__' :
    main()
