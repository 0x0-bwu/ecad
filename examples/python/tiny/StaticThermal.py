import os
import sys
import SetupDesign
from SetupDesign import ecad

mgr = ecad.DataMgr

def get_die_monitors(layout, cell_insts, components) :
        monitors = []
        elevation = 0.0
        thickness = 0.0
        retriever = ecad.LayoutRetriever(layout)
        for cell_inst in cell_insts :
            for component in components :
                name = cell_inst + mgr.hier_sep() + component
                comp = layout.find_component_by_name(name)
                assert(comp)
                success, elevation, thickness = retriever.get_component_height_thickness(comp)
                assert(success)
                bond = comp.get_boundary()
                assert(bond)
                bounding_box = bond.get_bbox()
                center = bounding_box.center()
                location = layout.get_coord_units().to_unit(center)
                monitors.append(ecad.FPoint3D(location.x, location.y, elevation - 0.1 * thickness))
        return monitors

def get_extraction_setting(work_dir, force_rebuild) :
    htc = 5000
    prism_settings = ecad.PrismThermalModelExtractionSettings(work_dir, mgr.threads(), set())
    prism_settings.bot_uniform_bc.type = ecad.ThermalBoundaryConditionType.HTC
    prism_settings.bot_uniform_bc.value = htc
    prism_settings.mesh_settings.gen_mesh_by_layer = False
    prism_settings.mesh_settings.iteration = 1
    prism_settings.mesh_settings.min_alpha = 15
    prism_settings.mesh_settings.min_len = 1e-3
    prism_settings.mesh_settings.max_len = 1000
    prism_settings.mesh_settings.tolerance = 0
    prism_settings.mesh_settings.dump_mesh_file = True
    prism_settings.layer_cut_settings.layer_transition_ratio = 3
    prism_settings.layer_cut_settings.dump_sketch_img = True
    prism_settings.force_rebuild = force_rebuild

    return prism_settings

def get_simulation_setup(layout, work_dir, cell_insts, components, force_rebuild = False) :

    extraction_setting = get_extraction_setting(work_dir, force_rebuild)
    simulation_setup = ecad.ThermalStaticSimulationSetup(work_dir, mgr.threads(), set())
    simulation_setup.settings.iteration = 100
    simulation_setup.settings.dump_hotmaps = True
    simulation_setup.settings.dump_matrices = True
    simulation_setup.settings.env_temperature = ecad.Temperature(25, ecad.TemperatureUnit.CELSIUS)
    simulation_setup.set_extraction_settings(extraction_setting)
    simulation_setup.monitors = get_die_monitors(layout, cell_insts, components)

    return simulation_setup

def setup_power(layout, include_joule_heat) :
    def get_sic_die_temperature_and_power_config() :
        return [(25.0, 108.0), (125.0, 124.0), (150.0, 126.5)]
    
    cell_insts = ["BotBridge1"]
    sic_comps = ["Die1"]
    for cell_inst in cell_insts :
        for sic_comp in sic_comps :
            name = cell_inst + mgr.hier_sep() + sic_comp
            comp = layout.find_component_by_name(name)
            assert(comp)
            for t, p in get_sic_die_temperature_and_power_config() :
                comp.set_loss_power(t, p)

def static_thermal_flow(chip_locations, include_joule_heat = True, show_heat_map = True) :
    
    work_dir = os.path.dirname(__file__) + '/data/simulation/static'
    layout = SetupDesign.setup_design("Tiny", chip_locations, work_dir)
    if not layout :
        return 0, 0, []

    setup_power(layout, include_joule_heat)
    setup = get_simulation_setup(layout, work_dir, ["BotBridge1"], ["Die1"])
    results = layout.run_thermal_simulation(setup)

    if show_heat_map :
        try :
            import vtk
            hotmap_vtk = work_dir + '/hotmap.vtk'
            if 'vtk' in sys.modules and os.path.exists(hotmap_vtk) :
                sys.path.append(os.path.dirname(__file__) + '/../tools')
                import HotmapViewer
                HotmapViewer.view_hotmap(hotmap_vtk)
        except :
            pass

    return results
    
def main() :

    mgr.init(ecad.LogLevel.TRACE)
    static_thermal_flow(SetupDesign.chip_locations, include_joule_heat=True)
    mgr.shut_down()

if __name__ == '__main__' :
    main()
