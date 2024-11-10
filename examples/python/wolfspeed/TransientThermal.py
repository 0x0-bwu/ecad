import os
import sys
import SetupDesign
import StaticThermal
from SetupDesign import ecad

mgr = ecad.DataMgr

def get_simulation_setup(layout, work_dir, cell_insts, components, force_rebuild = False) :

    extraction_setting = StaticThermal.get_extraction_setting(work_dir, force_rebuild)
    extraction_setting.layer_cut_settings.layer_transition_ratio = 5
    simulation_setup = ecad.ThermalTransientSimulationSetup(work_dir, mgr.threads(), set())
    simulation_setup.set_extraction_settings(extraction_setting)
    simulation_setup.settings.env_temperature = ecad.Temperature(25, ecad.TemperatureUnit.CELSIUS)
    simulation_setup.settings.mor.order = 0
    simulation_setup.settings.verbose = True
    simulation_setup.settings.dump_results = True
    simulation_setup.settings.duration = 100
    simulation_setup.settings.step = 10
    simulation_setup.settings.temperature_depend = False
    simulation_setup.settings.sampling_window = 100
    simulation_setup.settings.min_sampling_interval = 0.01
    simulation_setup.settings.absolute_error = 1e-2
    simulation_setup.settings.relative_error = 1e-3
    simulation_setup.settings.adaptive = True
    simulation_setup.monitors = StaticThermal.get_die_monitors(layout, cell_insts, components)
    return simulation_setup

def transient_thermal_flow(chip_locations, include_joule_heat = True) :
    
    work_dir = os.path.dirname(__file__) + '/data/simulation/transient'
    layout = SetupDesign.setup_design("CAS300M12BM2", chip_locations, work_dir)
    if not layout :
        return 0, 0, []

    StaticThermal.setup_power(layout, include_joule_heat)
    setup = get_simulation_setup(layout, work_dir, ["TopBridge1", "TopBridge2", "BotBridge1", "BotBridge2"], ["Die1", "Die2", "Die3"])
    
    def excitation(t, scen) :
        return 1 #const power
    results = layout.run_thermal_simulation(setup, excitation)
    return results
    
def main() :

    mgr.init(ecad.LogLevel.TRACE)
    transient_thermal_flow(SetupDesign.chip_locations, include_joule_heat=True)
    mgr.shut_down()

if __name__ == '__main__' :
    main()