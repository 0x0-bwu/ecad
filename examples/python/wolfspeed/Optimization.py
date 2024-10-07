import os
import sys
import scipy
from scipy import stats
from scipy import optimize

import SetupDesign
import StaticThermal
from SetupDesign import ecad

mgr = ecad.DataMgr

def cost_function(parameters) :
    try :
        cost_function.counter += 1
    except AttributeError :
        cost_function.counter = 0
    
    def score(chipT) :
        if not chipT :
            return 999999999

        return stats.variation(chipT)

    chip_locations = ecad.coord_to_fpoint2d(parameters)
    work_dir = os.path.dirname(__file__) + '/data/optimization'
    layout = SetupDesign.setup_design("CAS300M12BM2", chip_locations, work_dir, False)
    if not layout :
        return score([])
    
    power_config = StaticThermal.setup_power(layout, include_joule_heat=True)
    setup = StaticThermal.get_simulation_setup(layout, work_dir, ["TopBridge1", "TopBridge2", "BotBridge1", "BotBridge2"], ["Die1", "Die2", "Die3"], True)
    extraction_settings = setup.get_prism_thermal_model_extraction_settings()
    assert(extraction_settings)
    extraction_settings.mesh_settings.max_len = 2
    extraction_settings.mesh_settings.dump_mesh_file = False
    extraction_settings.layer_cut_settings.layer_transition_ratio = 0
    minT, maxT, chipT = layout.run_thermal_simulation(setup)

    print(f'run: {cost_function.counter}, score: {score(chipT)}')
    print(parameters)
    print(chipT)
    return score(chipT)

def main() :

    mgr.init(ecad.LogLevel.INFO)

    parameters = [-5.23, 8.93, -5.23, 3.86, -5.23, -1.21, 3.71, 8.08, 3.71, 1.33, 3.71, -5.42,
                   5.23, 8.08, 5.23, 1.33, 5.23, -5.42, -3.7, 8.08, -3.7, 1.33, -3.7, -5.42,]

    res = optimize.minimize(cost_function, parameters, method='BFGS', options={'disp': True})
    print(res.x)
    
    mgr.shut_down()

if __name__ == '__main__' :
    main()