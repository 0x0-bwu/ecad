import os
import sys

py_ecad = os.path.abspath(os.path.dirname(__file__) + '/../../build.release/lib')
sys.path.append(py_ecad)
import PyEcad as ecad

#constant variables
THIN_BONDWIRE_RADIUS = 0.0635
THICK_BONDWIRE_RADIUS = 0.15
RES_GATE = "Rg"
MAT_AL = "Al"
MAT_CU = "Cu"
MAT_AIR = "Air"
MAT_ALN = "AlN"
MAT_SIC = "SiC"
MAT_SAC305 = "SAC305"

def print_test_info(func) :
    def wrapper(*args, **kwargs):
        print(f'running: {func.__qualname__}')
        return func(*args, **kwargs)

    return wrapper

###EDataMgr
@print_test_info
def test_data_mgr() :
    #instance
    mgr = ecad.DataMgr
    mgr.init(ecad.LogLevel.INFO)

@print_test_info
def setup_design(name, parameters) :
    mgr = ecad.DataMgr
    def setup_material(database) :
        mat_al = mgr.create_material_def(database, MAT_AL)
        mat_al.set_property(ecad.MaterialPropId.THERMAL_CONDUCTIVITY, mgr.create_simple_material_prop(238))
        mat_al.set_property(ecad.MaterialPropId.SPECIFIC_HEAT, mgr.create_simple_material_prop(880))
        mat_al.set_property(ecad.MaterialPropId.MASS_DENSITY, mgr.create_simple_material_prop(2700))
        mat_al.set_property(ecad.MaterialPropId.RESISTIVITY, mgr.create_simple_material_prop(2.82e-8))
    
        mat_al = mgr.create_material_def(database, MAT_AL)
        mat_al.set_property(ecad.MaterialPropId.THERMAL_CONDUCTIVITY, mgr.create_simple_material_prop(238))
        mat_al.set_property(ecad.MaterialPropId.SPECIFIC_HEAT, mgr.create_simple_material_prop(880))
        mat_al.set_property(ecad.MaterialPropId.MASS_DENSITY, mgr.create_simple_material_prop(2700))
        mat_al.set_property(ecad.MaterialPropId.RESISTIVITY, mgr.create_simple_material_prop(2.82e-8))

    
    mgr.remove_database(name)
    database = mgr.create_database(name)

    setup_material(database)
        
@print_test_info
def static_thermal_flow() :
    def get_die_monitors(layout) :
        monitors = []
        elevation = 0.0
        thickness = 0.0
        retriever = ecad.LayoutRetriever(layout)
        cell_insts = ["TopBridge1", "TopBridge2", "BotBridge1", "BotBridge2"]
        components = ["Die1", "Die2", "Die3"]
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
        prism_settings.mesh_settings.max_len = 3
        prism_settings.mesh_settings.tolerance = 0
        prism_settings.mesh_settings.dump_mesh_file = True
        prism_settings.layer_cut_settings.layer_transition_ratio = 0
        prism_settings.layer_cut_settings.dump_sketch_img = True

        top_htc = htc
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(-29.35, 4.7, -20.35, 8.7), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(-29.35, -8.7, -20.35, -4.7), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(2.75, 11.5, 9.75, 17), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(2.75, -17, 9.75, -11.5), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(-7.75, 11.5, -2.55, 17), ecad.ThermalBondaryConditionType.HTC, top_htc)
        prism_settings.add_block_bc(ecad.Orientation.TOP, ecad.FBox2D(-7.75, -17, -2.55, -11.5), ecad.ThermalBondaryConditionType.HTC, top_htc)
        return prism_settings

    design_filename = os.path.dirname(__file__) + '/../wolfspeed/data/design/CAS300M12BM2.ecad'
    work_dir = os.path.dirname(__file__) + '../wolfspeed/simulation/static'

    mgr = ecad.DataMgr
    database = mgr.load_database(design_filename)
    print(database.get_name())
    cell = database.find_cell_by_name("Base")
    layout = cell.get_flattened_layout_view()    

    extraction_setting = get_extraction_setting(work_dir)
    simulation_setup = ecad.ThermalStaticSimulationSetup(work_dir, mgr.threads(), set())
    simulation_setup.settings.iteration = 100
    simulation_setup.settings.dump_hotmaps = True
    simulation_setup.settings.env_temperature = ecad.Temperature(25, ecad.TemperatureUnit.CELSIUS)
    simulation_setup.set_extraction_settings(extraction_setting)
    simulation_setup.monitors = get_die_monitors(layout)
    results = layout.run_thermal_simulation(simulation_setup)
    print(results)

def main() :
    print('ecad version: ' + ecad.__version__)

    mgr = ecad.DataMgr
    mgr.init(ecad.LogLevel.TRACE)

    chip_locations = [
        -5.23, 8.93, -5.23, 3.86, -5.23, -1.21, 3.71, 8.08, 3.71, 1.33, 3.71, -5.42,
        5.23, 8.08, 5.23, 1.33, 5.23, -5.42, -3.7, 8.08, -3.7, 1.33, -3.7, -5.42,]
    
    setup_design("CAS300M12BM2", chip_locations)
    static_thermal_flow()

    mgr.shut_down()
    print('every thing is fine')

if __name__ == '__main__' :
    main()