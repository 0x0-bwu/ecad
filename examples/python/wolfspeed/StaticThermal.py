import os
import sys
import SetupDesign
from SetupDesign import ecad

mgr = ecad.DataMgr

def get_simulation_setup(layout, work_dir, cell_insts, components, force_rebuild = False) :
    
    def get_extraction_setting(work_dir) :
        htc = 5000
        prism_settings = ecad.PrismThermalModelExtractionSettings(work_dir, mgr.threads(), set())
        prism_settings.bot_uniform_bc.type = ecad.ThermalBondaryConditionType.HTC
        prism_settings.bot_uniform_bc.value = htc
        prism_settings.mesh_settings.gen_mesh_by_layer = True
        if prism_settings.mesh_settings.gen_mesh_by_layer :
            prism_settings.mesh_settings.imprint_upper_layer = False
        prism_settings.mesh_settings.iteration = int(1e5)
        prism_settings.mesh_settings.min_alpha = 15
        prism_settings.mesh_settings.min_len = 1e-3
        prism_settings.mesh_settings.max_len = 1
        prism_settings.mesh_settings.tolerance = 0
        prism_settings.mesh_settings.dump_mesh_file = True
        prism_settings.layer_cut_settings.layer_transition_ratio = 3
        prism_settings.layer_cut_settings.dump_sketch_img = True
        prism_settings.force_rebuild = force_rebuild

        top_htc = htc
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(-29.35, 4.7, -20.35, 8.7), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(-29.35, -8.7, -20.35, -4.7), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(2.75, 11.5, 9.75, 17), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(2.75, -17, 9.75, -11.5), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(-7.75, 11.5, -2.55, 17), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(-7.75, -17, -2.55, -11.5), ecad.ThermalBondaryConditionType.HTC, top_htc)
        return prism_settings

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
                    bounding_box = comp.get_bounding_box()
                    center = bounding_box.center()
                    location = layout.get_coord_units().to_unit(center)
                    monitors.append(ecad.FPoint3D(location.x, location.y, elevation - 0.1 * thickness))
            return monitors

    extraction_setting = get_extraction_setting(work_dir)
    simulation_setup = ecad.ThermalStaticSimulationSetup(work_dir, mgr.threads(), set())
    simulation_setup.settings.iteration = 100
    simulation_setup.settings.dump_hotmaps = True
    simulation_setup.settings.env_temperature = ecad.Temperature(25, ecad.TemperatureUnit.CELSIUS)
    simulation_setup.set_extraction_settings(extraction_setting)
    simulation_setup.monitors = get_die_monitors(layout, cell_insts, components)

    return simulation_setup

def setup_power(layout, include_joule_heat) :
    def get_sic_die_temperature_and_power_config() :
        return [(25.0, 108.0), (125.0, 124.0), (150.0, 126.5)]

    def get_diode_temperature_and_power_config() :
        return [(25.0, 20.4), (125.0, 21.7), (150.0, 21.8)]
    
    cell_insts = ["TopBridge1", "TopBridge2", "BotBridge1", "BotBridge2"]
    diode_comps = ["Diode1", "Diode2", "Diode3"]
    sic_comps = ["Die1", "Die2", "Die3"]
    for cell_inst in cell_insts :
        for sic_comp in sic_comps :
            name = cell_inst + mgr.hier_sep() + sic_comp
            comp = layout.find_component_by_name(name)
            assert(comp)
            for t, p in get_sic_die_temperature_and_power_config() :
                comp.set_loss_power(t, p)
        for diode_comp in diode_comps :
            name = cell_inst + mgr.hier_sep() + diode_comp
            comp = layout.find_component_by_name(name)
            assert(comp)
            for t, p in get_diode_temperature_and_power_config() :
                comp.set_loss_power(t, p)
        if include_joule_heat :
            prim_iter = layout.get_primitive_iter()
            while prim := prim_iter.next() :
                if bw := prim.get_bondwire_from_primitive() :
                    name = bw.get_name()
                    if '10A' in name :
                        bw.set_current(10)
                    if '15A' in name :
                        bw.set_current(15)

def static_thermal_flow(chip_locations, include_joule_heat = True, show_heat_map = True) :
    
    work_dir = os.path.dirname(__file__) + '/data/simulation/static'
    layout = SetupDesign.setup_design("CAS300M12BM2", chip_locations, work_dir)
    if not layout :
        return 0, 0, []

    setup_power(layout, include_joule_heat)
    setup = get_simulation_setup(layout, work_dir, ["TopBridge1", "TopBridge2", "BotBridge1", "BotBridge2"], ["Die1", "Die2", "Die3"])
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
