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
        return stats.variation(chipT[:5]) + stats.variation(chipT[6:]) 
        # return max(chipT) - min(chipT)

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
    minimizer_kwargs = {'method': 'Nelder-Mead', 'bounds': bonds, 'constraints': conds}
    res = optimize.dual_annealing(cost_function, x0=parameters, bounds=bonds, maxiter=100, maxfun=50, minimizer_kwargs=minimizer_kwargs)
    mgr.shut_down()

    # solution1 = [-5.47430291, 9.10514553, -4.5768218 ,  3.47671531, -5.30450942, -1.49377357,
    #                 3.52563662, 8.40086493, 3.74418299, 1.4601866 , 3.57019182, -5.59078083,
    #                 5.05825811, 7.95055065, 5.00023084, 0.93179762, 4.87108773, -5.80438403,
    #                 -3.92175326, 8.33647521, -3.64471131, 0.94203844, -3.76396559, -5.05881545]
    # solution2 = [-6.61367575, 8.27202511, -6.32373081, 1.47321837, -0.88582709, -0.89723667,
    #                 4.62950951, 5.88185049, -1.06509744, 4.5586134, 4.82157607, -1.51623676,
    #                 0.83176562, 8.57141646, 6.73294934, 1.99509159, 4.0975681, -4.69498692,
    #                 -5.17233796, 4.93276395, -0.25947581, 0.0506895, -1.34856115, -5.09672135,]

if __name__ == '__main__' :
    main()