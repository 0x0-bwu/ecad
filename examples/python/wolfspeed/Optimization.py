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
    setup.settings.dump_hotmaps = False
    minT, maxT, chipT = layout.run_thermal_simulation(setup)

    print(f'run: {cost_function.counter}, score: {score(chipT)}')
    print(parameters)
    print(chipT)
    return score(chipT)

def main() :

    mgr.init(ecad.LogLevel.INFO)

    parameters = [-5.23, 8.93, -5.23, 3.86, -5.23, -1.21, 3.71, 8.08, 3.71, 1.33, 3.71, -5.42,
                   5.23, 8.08, 5.23, 1.33, 5.23, -5.42, -3.7, 8.08, -3.7, 1.33, -3.7, -5.42,]
    
    bonds = ((-6.95, 0.65), (1.50, 9.50), (-6.95, 0.65), (-2.50, 5.50), (-6.95, 0.65), (-6.50, 1.50),
             (-2.20, 5.40), (2.75, 9.25), (-2.20, 5.40), (-1.75, 4.75), (-2.20, 5.40), (-6.25, 0.25),
             (-0.65, 6.95), (1.50, 9.50), (-0.65, 6.95), (-2.50, 5.50), (-0.65, 6.95), (-6.50, 1.50),
             (-5.40, 2.20), (2.75, 9.25), (-5.40, 2.20), (-1.75, 4.75), (-5.40, 2.20), (-6.25, 0.25),            
            )
    
    conds = ({'type': 'ineq', 'fun': lambda x: x[ 6] - x[ 0] - 5.0},
             {'type': 'ineq', 'fun': lambda x: x[ 8] - x[ 2] - 5.0},
             {'type': 'ineq', 'fun': lambda x: x[10] - x[ 4] - 5.0},
             {'type': 'ineq', 'fun': lambda x: x[ 1] - x[ 3] - 4.0},
             {'type': 'ineq', 'fun': lambda x: x[ 3] - x[ 5] - 4.0},
             {'type': 'ineq', 'fun': lambda x: x[ 7] - x[ 9] - 4.5},
             {'type': 'ineq', 'fun': lambda x: x[ 9] - x[11] - 4.5},
             {'type': 'ineq', 'fun': lambda x: x[12] - x[18] - 5.0},
             {'type': 'ineq', 'fun': lambda x: x[14] - x[20] - 5.0},
             {'type': 'ineq', 'fun': lambda x: x[16] - x[22] - 5.0},
             {'type': 'ineq', 'fun': lambda x: x[13] - x[15] - 4.0},
             {'type': 'ineq', 'fun': lambda x: x[15] - x[17] - 4.0},
             {'type': 'ineq', 'fun': lambda x: x[19] - x[21] - 4.5},
             {'type': 'ineq', 'fun': lambda x: x[21] - x[23] - 4.5},
            )

    res = optimize.minimize(cost_function, parameters, method='SLSQP', bounds=bonds, constraints=conds, options={'disp': True, 'eps': 0.1})
    print(res.x)
    
    mgr.shut_down()

if __name__ == '__main__' :
    main()