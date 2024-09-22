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

@print_test_info
def setup_design(name, parameters) :
    mgr = ecad.DataMgr
    mgr.init(ecad.LogLevel.TRACE)
    def get_sic_die_temperature_and_power_config() :
        return [(25.0, 108.0), (125.0, 124.0), (150.0, 126.5)]
    
    def get_diode_temperature_and_power_config() :
        return [(25.0, 20.4), (125.0, 21.7), (150.0, 21.8)]
    
    def setup_material(database) :
        mat_al = mgr.create_material_def(database, MAT_AL)
        mat_al.set_property(ecad.MaterialPropId.THERMAL_CONDUCTIVITY, mgr.create_simple_material_prop(238))
        mat_al.set_property(ecad.MaterialPropId.SPECIFIC_HEAT, mgr.create_simple_material_prop(880))
        mat_al.set_property(ecad.MaterialPropId.MASS_DENSITY, mgr.create_simple_material_prop(2700))
        mat_al.set_property(ecad.MaterialPropId.RESISTIVITY, mgr.create_simple_material_prop(2.82e-8))
    
        mat_cu = mgr.create_material_def(database, MAT_CU)
        mat_cu.set_property(ecad.MaterialPropId.THERMAL_CONDUCTIVITY, mgr.create_polynomial_material_prop([[437.6, -0.165, 1.825e-4, -1.427e-7, 3.979e-11]]))
        mat_cu.set_property(ecad.MaterialPropId.SPECIFIC_HEAT, mgr.create_polynomial_material_prop([[342.8, 0.134, 5.535e-5, -1.971e-7, 1.141e-10]]))
        mat_cu.set_property(ecad.MaterialPropId.MASS_DENSITY, mgr.create_simple_material_prop(8850))

        mat_air = mgr.create_material_def(database, MAT_AIR)
        mat_air.set_material_type(ecad.MaterialType.FLUID)
        mat_air.set_property(ecad.MaterialPropId.THERMAL_CONDUCTIVITY, mgr.create_simple_material_prop(0.026))
        mat_air.set_property(ecad.MaterialPropId.SPECIFIC_HEAT, mgr.create_simple_material_prop(1.003))
        mat_air.set_property(ecad.MaterialPropId.MASS_DENSITY, mgr.create_simple_material_prop(1.225))

        mat_sic = mgr.create_material_def(database, MAT_SIC)
        mat_sic.set_property(ecad.MaterialPropId.THERMAL_CONDUCTIVITY, mgr.create_polynomial_material_prop([[1860, -11.7, 0.03442, -4.869e-5, 2.675e-8]]))
        mat_sic.set_property(ecad.MaterialPropId.SPECIFIC_HEAT, mgr.create_polynomial_material_prop([[-3338, 33.12, -0.1037, 0.0001522, -8.553e-80]]))
        mat_sic.set_property(ecad.MaterialPropId.MASS_DENSITY, mgr.create_simple_material_prop(3210))

        mat_aln = mgr.create_material_def(database, MAT_ALN)
        mat_aln.set_property(ecad.MaterialPropId.THERMAL_CONDUCTIVITY, mgr.create_polynomial_material_prop([[421.7867, -1.1262, 0.001]]))
        mat_aln.set_property(ecad.MaterialPropId.SPECIFIC_HEAT, mgr.create_polynomial_material_prop([[170.2, -2.018, 0.032, -8.957e-5, 1.032e-7, -4.352e-11]]))
        mat_aln.set_property(ecad.MaterialPropId.MASS_DENSITY, mgr.create_simple_material_prop(3260))

        mat_solder = mgr.create_material_def(database, MAT_SAC305)
        mat_solder.set_property(ecad.MaterialPropId.THERMAL_CONDUCTIVITY, mgr.create_simple_material_prop(55))
        mat_solder.set_property(ecad.MaterialPropId.SPECIFIC_HEAT, mgr.create_simple_material_prop(218))
        mat_solder.set_property(ecad.MaterialPropId.MASS_DENSITY, mgr.create_simple_material_prop(7800))
        mat_solder.set_property(ecad.MaterialPropId.RESISTIVITY, mgr.create_simple_material_prop(11.4e-8))

    def create_bondwire_solder_joints(database, name, radius) :
        padstack_def = mgr.create_padstack_def(database, name)
        def_data = mgr.create_padstack_def_data()
        def_data.set_top_solder_bump_material(MAT_SAC305)
        def_data.set_bot_solder_ball_material(MAT_SAC305)

        bump_r = radius * 1.1
        bump = mgr.create_shape_rectangle(database.coord_units, ecad.FPoint2D(-bump_r, -bump_r), ecad.FPoint2D(bump_r, bump_r))
        def_data.set_top_solder_bump_parameters(bump, 0.05)
        def_data.set_bot_solder_ball_parameters(bump, 0.05)

        padstack_def.set_padstack_def_data(def_data)
        return padstack_def

    def create_sic_die_component_def(database) :
        sic_die = mgr.create_component_def(database, 'SicDie')
        sic_die.set_component_type(ecad.ComponentType.IC)
        sic_die.set_solder_ball_bump_height(0.1)
        sic_die.set_solder_filling_material(MAT_SAC305)
        sic_die.set_bonding_box(mgr.create_box(database.coord_units, ecad.FPoint2D(-2.545, -2.02), ecad.FPoint2D(2.545, 2.02)))        
        sic_die.set_material(MAT_SIC)
        sic_die.set_height(0.18)

        mgr.create_component_def_pin(sic_die, "G", ecad.FPoint2D(-1.25,  1.0), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(sic_die, "B", ecad.FPoint2D(-1.25,  0.0), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(sic_die, "D", ecad.FPoint2D(-1.25, -1.0), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(sic_die, "A", ecad.FPoint2D( 1.25,  1.0), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(sic_die, "C", ecad.FPoint2D( 1.25,  0.0), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(sic_die, "E", ecad.FPoint2D( 1.25, -1.0), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(sic_die, "K", ecad.FPoint2D(-2.00, -0.5), ecad.PinIOType.RECEIVER)
        return sic_die

    def create_diode_component_def(database) :
        diode = mgr.create_component_def(database, "Diode")
        diode.set_component_type(ecad.ComponentType.IC)
        diode.set_solder_ball_bump_height(0.1)
        diode.set_solder_filling_material(MAT_SAC305)
        diode.set_bonding_box(mgr.create_box(database.coord_units, ecad.FPoint2D(-2.25, -2.25), ecad.FPoint2D(2.25, 2.25)))
        diode.set_material(MAT_SIC)
        diode.set_height(0.18)

        mgr.create_component_def_pin(diode, "A", ecad.FPoint2D(-1.125,  1.50), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(diode, "B", ecad.FPoint2D(-1.125,  0.75), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(diode, "C", ecad.FPoint2D(-1.125,  0.00), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(diode, "D", ecad.FPoint2D(-1.125, -0.75), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(diode, "E", ecad.FPoint2D(-1.125, -1.50), ecad.PinIOType.RECEIVER)

        mgr.create_component_def_pin(diode, "F", ecad.FPoint2D( 1.125,  1.50), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(diode, "G", ecad.FPoint2D( 1.125,  0.75), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(diode, "H", ecad.FPoint2D( 1.125,  0.00), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(diode, "I", ecad.FPoint2D( 1.125, -0.75), ecad.PinIOType.RECEIVER)
        mgr.create_component_def_pin(diode, "J", ecad.FPoint2D( 1.125, -1.50), ecad.PinIOType.RECEIVER)
        return diode
    
    def create_gate_resistance_component_def(database) :
        r = mgr.create_component_def(database, RES_GATE)
        r.set_bonding_box(mgr.create_box(database.coord_units, ecad.FPoint2D(-1.05, -0.65), ecad.FPoint2D(1.05, 0.65)))
        r.set_material(MAT_SIC)
        r.set_height(0.5)
        return r

    def create_base_layout(database) :
        base_cell = mgr.create_circuit_cell(database, 'Base')
        base_layout = base_cell.get_layout_view()
        coord_units = database.coord_units
        pwh = ecad.PolygonWithHolesData()
        pwh.outline = mgr.create_shape_polygon(coord_units, [-52.2, -29.7, 52.2, -29.7, 52.5, 29.7, -52.2, 29.7], 5.3).get_contour()
        pwh.holes.append(mgr.create_shape_circle(coord_units, ecad.FPoint2D(-46.5, -24), 3.85).get_contour())
        pwh.holes.append(mgr.create_shape_circle(coord_units, ecad.FPoint2D( 46.5, -24), 3.85).get_contour())
        pwh.holes.append(mgr.create_shape_circle(coord_units, ecad.FPoint2D( 46.5,  24), 3.85).get_contour())
        pwh.holes.append(mgr.create_shape_circle(coord_units, ecad.FPoint2D(-46.5,  24), 3.85).get_contour())
        base_layout.set_boundary(mgr.create_shape_polygon_with_holes(pwh))
        
        mgr.create_net(base_layout, 'Gate')
        mgr.create_net(base_layout, 'Drain')
        mgr.create_net(base_layout, 'Source')
        mgr.create_net(base_layout, 'Kelvin')

        top_layer = base_layout.append_layer(mgr.create_stackup_layer('TopCuLayer', ecad.LayerType.CONDUCTING_LAYER, 0, 0.3, MAT_CU, MAT_AIR))
        base_layout.append_layer(mgr.create_stackup_layer('CeramicLayer', ecad.LayerType.DIELECTRIC_LAYER, -0.3, 0.38, MAT_ALN, MAT_AIR))
        base_layout.append_layer(mgr.create_stackup_layer('BotCuLayer', ecad.LayerType.CONDUCTING_LAYER, -0.68, 0.3, MAT_CU, MAT_AIR))
        base_layout.append_layer(mgr.create_stackup_layer('SolderLayer', ecad.LayerType.CONDUCTING_LAYER, -0.98, 0.1, MAT_SAC305, MAT_AIR))
        base_layout.append_layer(mgr.create_stackup_layer('BaseLayer', ecad.LayerType.CONDUCTING_LAYER, -1.08, 3, MAT_CU, MAT_CU))

        d_ploc = ecad.fcoord_to_fpoint2d([-3, 24, -3, 23.275, -3, 22.55, -4, 23.275, -4, 22.55, -3, 6.525, -3, 5.8, -3, 5.075, -4, 6.525, -4, 5.8])
        s_ploc = ecad.fcoord_to_fpoint2d([3, 24, 3, 23.275, 3, 22.55, 4, 21.825, 3, 21.825, 3, 6.525, 3, 5.8, 3, 5.075, 3, 7.25, 4, 7.25])
        for i in range(len(d_ploc)) :
            bw1 = mgr.create_bondwire(base_layout, f'DS1_{i + 1}', ecad.NetId.NO_NET, THICK_BONDWIRE_RADIUS)
            bw1.set_start_layer(top_layer, coord_units.to_coord(d_ploc[i]), False)
            bw1.set_end_layer(top_layer, coord_units.to_coord(s_ploc[i]), False)
            bw1.set_current(15)
            bw1.set_dynamic_power_scenario(0)

            d_ploc[i].y *= -1
            s_ploc[i].y *= -1
            bw2 = mgr.create_bondwire(base_layout, f'DS2_{i + 1}', ecad.NetId.NO_NET, THICK_BONDWIRE_RADIUS)
            bw2.set_start_layer(top_layer, coord_units.to_coord(d_ploc[i]), False)
            bw2.set_end_layer(top_layer, coord_units.to_coord(s_ploc[i]), False)
            bw2.set_current(15)
            bw2.set_dynamic_power_scenario(0)
        
        g_ploc = ecad.fcoord_to_fpoint2d([-3, 3, -3, 1.8])
        for i in range(len(g_ploc)) :
            p = g_ploc[i]
            bw1 = mgr.create_bondwire(base_layout, f'G1_{i + 1}', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
            bw1.set_start_layer(top_layer, coord_units.to_coord(p), False)
            bw2.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(-p.x, p.y)), False)

            bw2 = mgr.create_bondwire(base_layout, f'G2_{i + 1}', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
            bw2.set_start_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(p.x, -p.y)), False)
            bw2.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(-p.x, -p.y)), False)
        
        kelvin_bw0 = mgr.create_bondwire(base_layout, 'KelvinBw0', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
        kelvin_bw0.set_start_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(30.15, -5.95)), False)
        kelvin_bw0.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(40.05, -5.95)), False)

        kelvin_bw = mgr.create_bondwire(base_layout, 'KelvinBw', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
        kelvin_bw.set_start_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(30.15, 5)), False)
        kelvin_bw.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(30.15, -5)), False)

        gate_bw0 = mgr.create_bondwire(base_layout, 'GateBw0', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
        gate_bw0.set_start_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(32, -12.375)), False)
        gate_bw0.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(40.05, -12.375)), False)

        gate_bw = mgr.create_bondwire(base_layout, 'GateBw', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
        gate_bw.set_start_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(32, 5)), False)
        gate_bw.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(32, -5)), False)

        gate_bw1 = mgr.create_bondwire(base_layout, 'GateBw1', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
        gate_bw1.set_start_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(32.5, 3)), False)
        gate_bw1.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(41.3, 3.35)), False)

        gate_bw2 = mgr.create_bondwire(base_layout, 'GateBw2', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
        gate_bw2.set_start_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(32.5, 1.8)), False)
        gate_bw2.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(40.05, 1.0375)), False)

        gate_bw3 = mgr.create_bondwire(base_layout, 'GateBw3', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
        gate_bw3.set_start_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(32.5, -1.8)), False)
        gate_bw3.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(40.05, -0.3625)), False)

        gate_bw4 = mgr.create_bondwire(base_layout, 'GateBw4', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
        gate_bw4.set_start_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(32.5, -3)), False)
        gate_bw4.set_end_layer(top_layer, coord_units.to_coord(ecad.FPoint2D(40.05, -2.7)), False)

        return base_layout
    
    def create_driver_layout(database) :
        coord_units = database.coord_units
        driver_cell = mgr.create_circuit_cell(database, "Driver")
        driver_layout = driver_cell.get_layout_view()

        layer1 = driver_layout.append_layer(mgr.create_stackup_layer('DriverLayer1', ecad.LayerType.DIELECTRIC_LAYER, -0.3, 0.38, MAT_ALN, MAT_AIR))
        layer2 = driver_layout.append_layer(mgr.create_stackup_layer('DriverLayer2', ecad.LayerType.CONDUCTING_LAYER, -0.68, 0.3, MAT_CU, MAT_AIR))
        layer3 = driver_layout.append_layer(mgr.create_stackup_layer('DriverLayer3', ecad.LayerType.CONDUCTING_LAYER, -0.98, 0.1, MAT_SAC305, MAT_AIR))
        layer4 = driver_layout.append_layer(mgr.create_stackup_layer('DriverLayer4', ecad.LayerType.CONDUCTING_LAYER, -0.98, 0.1, MAT_SAC305, MAT_AIR))
        
        driver_layout.set_boundary(mgr.create_shape_polygon(coord_units, [-5.5, -14.725, 5.5, -14.725, 5.5, 14.725, -5.5, 14.725], 0))

        mgr.create_net(driver_layout, 'Gate')
        mgr.create_net(driver_layout, 'Drain')
        mgr.create_net(driver_layout, 'Source')

        mgr.create_geometry_2d(driver_layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [1.7, 9.625, 4.7, 9.625, 4.7, 13.925, 1.7, 13.925], 0.25))
        mgr.create_geometry_2d(driver_layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [1.7, 4.325, 4.7, 4.325, 4.7,  8.625, 1.7, 8.625], 0.25))
        mgr.create_geometry_2d(driver_layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [1.7, -13.925, 4.7, -13.925, 4.7, 1.075, 3.2, 1.075, 3.2, -1.775, 4.2, -1.775, 4.2, -4.925, 3.2, -4.925, 3.2, -7.025, 4.2, -7.025, 4.2, -11.425, 1.7, -11.425], 0.25))
        mgr.create_geometry_2d(driver_layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [1.7, -10.325, 3.2, -10.325, 3.2, -8.225, 2.2, -8.225, 2.2, -3.875, 3.2, -3.875, 3.2, -2.825, 2.2, -2.825, 2.2, 2.175, 4.7, 2.175, 4.7, 3.225, 1.7, 3.225], 0.25))

        mgr.create_geometry_2d(driver_layout, layer2, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [0.9, -14.725, 5.5, -14.725, 5.5, 14.725, 0.9, 14.725], 0.25))
        mgr.create_geometry_2d(driver_layout, layer3, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [1.4, -14.225, 5.0, -14.225, 5.0, 14.225, 1.4, 14.225], 0.25))
        mgr.create_geometry_2d(driver_layout, layer4, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [0.9, -14.725, 5.5, -14.725, 5.5, 14.725, 0.9, 14.725], 0.25))

        return driver_layout

    def create_default_layer_map(database, from_layout, to_layout, name) :
        layer_map = mgr.create_layer_map(database, name)
        from_iter = from_layout.get_layer_iter()
        to_iter = to_layout.get_layer_iter()
        from_layer = from_iter.next()
        to_layer = to_iter.next()
        while from_layer and to_layer :
            layer_map.set_mapping(from_layer.get_layer_id(), to_layer.get_layer_id())
            from_layer = from_iter.next()
            to_layer = to_iter.next()
        return layer_map

    def create_bot_bridge_layout(database, locations) :
        coord_units = database.coord_units
        cell = mgr.create_circuit_cell(database, 'BotBridgeCell')
        layout = cell.get_layout_view()

        layer1 = layout.append_layer(mgr.create_stackup_layer('Layer1', ecad.LayerType.DIELECTRIC_LAYER, -0.3, 0.38, MAT_ALN, MAT_AIR))
        layer2 = layout.append_layer(mgr.create_stackup_layer('Layer2', ecad.LayerType.CONDUCTING_LAYER, -0.68, 0.3, MAT_CU, MAT_AIR))
        layer3 = layout.append_layer(mgr.create_stackup_layer('Layer3', ecad.LayerType.CONDUCTING_LAYER, -0.98, 0.1, MAT_SAC305, MAT_AIR))
        layer4 = layout.append_layer(mgr.create_stackup_layer('Layer4', ecad.LayerType.CONDUCTING_LAYER, -0.98, 0.1, MAT_SAC305, MAT_AIR))

        layout.set_boundary(mgr.create_shape_polygon(coord_units, [-16.75, -12.5, 16.75, -12.5, 16.75, 12.5, -16.75, 12.5], 0))
        mgr.create_net(layout, 'Gate')
        mgr.create_net(layout, 'Drain')
        mgr.create_net(layout, 'Source')

        mgr.create_geometry_2d(layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [-15.45, -11.2, 13.35, -11.2, 13.95, -11.6, 15.45, -11.6, 15.45, -10.8, -15.05, -10.8, -15.05, -1.3, -14.7, -0.7, -14.7, 11.45, -15.45, 11.45], 0.25))
        mgr.create_geometry_2d(layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [-14.2, -10.4, 15.45, -10.4, 15.45, -9.6, 13.95, -9.6, 13.35, -10, -13.8, -10, -13.8, -2.55, -11.1, -2.55, -11.1, 11.45, -11.85, 11.45, -11.85, -2, -14.2, -2], 0.25))
        mgr.create_geometry_2d(layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [-12.6, -8.8, 12.35, -8.8, 12.95, -8.4, 15.45, -8.4, 15.45, -5.97, 7.95, -5.97, 7.95, 9.03, 15.45, 9.03, 15.45, 11.45, -9.75, 11.45, -9.75, -3.75, -12.6, -3.75], 0.25))

        mgr.create_geometry_2d(layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [-13.65, -0.7, -12.9, -0.7, -12.9, 2.6, -13.65, 2.6], 0.25))
        mgr.create_geometry_2d(layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [-13.65, 3.725, -12.9, 3.725, -12.9, 7.025, -13.65, 7.025], 0.25))
        mgr.create_geometry_2d(layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [-13.65, 8.15, -12.9, 8.15, -12.9, 11.45, -13.65, 11.45], 0.25))
       
        mgr.create_geometry_2d(layout, layer1, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [9.5, -4.77, 15.8, -4.77, 15.8, 7.83, 9.5, 7.83], 0.25))
       
        mgr.create_geometry_2d(layout, layer2, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [-16.75, -12.5, 16.75, -12.5, 16.75, 12.5, -16.75, 12.5], 0.25))
        mgr.create_geometry_2d(layout, layer3, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [-16.25, -12, 16.25, -12, 16.25, 12, -16.25, 12], 0.25))
        mgr.create_geometry_2d(layout, layer4, ecad.NetId.NO_NET, mgr.create_shape_polygon(coord_units, [-16.75, -12.5, 16.75, -12.5, 16.75, 12.5, -16.75, 12.5], 0.25))

        sic_die = mgr.find_component_def_by_name(database, 'SicDie')
        die_comps = [mgr.create_component(layout, f'Die{i + 1}', sic_die, layer1, mgr.create_transform_2d(coord_units, 1, 0, locations[i], ecad.Mirror2D.NO), False) for i in range(3)]
        for t, p in get_sic_die_temperature_and_power_config() :
            for die_comp in die_comps :
                die_comp.set_loss_power(t, p)
                die_comp.set_dynamic_power_scenario(2)

        diode = mgr.find_component_def_by_name(database, "Diode")
        diode_comps = [mgr.create_component(layout, f'Diode{i + 1}', diode, layer1, mgr.create_transform_2d(coord_units, 1, 0, locations[i + 3], ecad.Mirror2D.NO), False) for i in range(3)]
        for t, p in get_diode_temperature_and_power_config() :
            for diode_comp in diode_comps :
                diode_comp.set_loss_power(t, p)
                diode_comp.set_dynamic_power_scenario(2)
        res = mgr.find_component_def_by_name(database, RES_GATE)
        res_loc = [ecad.FPoint2D(-14.17, y) for y in [10.5, 6.075, 1.65]]
        res_comps = [mgr.create_component(layout, f'R{i + 1}', res, layer1, mgr.create_transform_2d(coord_units, 1, 0, res_loc[i], ecad.Mirror2D.NO), False) for i in range(3)]

        bw_loc = [ecad.FPoint2D(-13.275, y) for y in [8.6, 4.05, -0.27]]
        for i in range(3) :
            gate_bw = mgr.create_bondwire(layout, f'GateBw{i + 1}', ecad.NetId.NO_NET, THIN_BONDWIRE_RADIUS)
            gate_bw.set_start_component(die_comps[i], 'G')
            gate_bw.set_end_layer(layer1, coord_units.to_coord(bw_loc[i]), False)

        # todo
        return layout

    mgr.remove_database(name)
    database = mgr.create_database(name)

    database.coord_units = ecad.CoordUnits(ecad.CoordUnit.MILLIMETER)

    setup_material(database)

    thin_bw_solder_def = create_bondwire_solder_joints(database, 'Thin Solder Joints', THIN_BONDWIRE_RADIUS)
    thick_bw_solder_def = create_bondwire_solder_joints(database, 'Thick Solder Joints', THICK_BONDWIRE_RADIUS)
    
    create_sic_die_component_def(database)
    create_diode_component_def(database)
    create_gate_resistance_component_def(database)

    base_layout = create_base_layout(database)
    driver_layout = create_driver_layout(database)
    driver_layer_map = create_default_layer_map(database, driver_layout, base_layout, 'DriverLayerMap')

    driver = mgr.create_cell_inst(base_layout, 'Driver', driver_layout, mgr.create_transform_2d(database.coord_units, 1, 0, ecad.FVector2D(44, 0), ecad.Mirror2D.XY))
        
    bot_comp_loc = [ecad.FPoint2D(parameters[i * 2], parameters[i * 2 + 1]) for i in range(6)]
    bot_bridge_layout = create_bot_bridge_layout(database, bot_comp_loc)
    bot_bridge_layer_map = create_default_layer_map(database, bot_bridge_layout, base_layout, 'BotBridgeLayerMap')

    bot_bridge1 = mgr.create_cell_inst(base_layout, 'BotBridge1', bot_bridge_layout, mgr.create_transform_2d(database.coord_units, 1, 0, ecad.FPoint2D(-17.75, 13), ecad.Mirror2D.NO))
    bot_bridge2 = mgr.create_cell_inst(base_layout, 'BotBridge2', bot_bridge_layout, mgr.create_transform_2d(database.coord_units, 1, 0, ecad.FPoint2D(-17.75, -13), ecad.Mirror2D.X))

    base_layout.flatten(ecad.FlattenOption.NO)
    prim_iter = base_layout.get_primitive_iter()
    while prim := prim_iter.next() :
        if bw := prim.get_bondwire_from_primitive() :
            if bw.get_radius() > THIN_BONDWIRE_RADIUS :
                bw.set_solder_joints(thick_bw_solder_def)
            else :
                bw.set_solder_joints(thin_bw_solder_def)
            bw.set_material(MAT_AL)
            if not (bw.get_height() > 0) :
                bw.set_height(0.5)
    
    mgr.shut_down()

@print_test_info
def static_thermal_flow() :
    def get_die_monitors(layout) :
        monitors = []
        elevation = 0.0
        thickness = 0.0
        retriever = ecad.LayoutRetriever(layout)
        # cell_insts = ["TopBridge1", "TopBridge2", "BotBridge1", "BotBridge2"]
        cell_insts = ["BotBridge1", "BotBridge2"]
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
    work_dir = os.path.dirname(__file__) + '/../wolfspeed/simulation/static'

    mgr = ecad.DataMgr
    mgr.init(ecad.LogLevel.TRACE)
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
    mgr.shut_down()

def main() :
    print('ecad version: ' + ecad.__version__)

    chip_locations = [
        -5.23, 8.93, -5.23, 3.86, -5.23, -1.21, 3.71, 8.08, 3.71, 1.33, 3.71, -5.42,
        5.23, 8.08, 5.23, 1.33, 5.23, -5.42, -3.7, 8.08, -3.7, 1.33, -3.7, -5.42,]
    
    # setup_design("CAS300M12BM2_Test", chip_locations)
    static_thermal_flow()

    print('every thing is fine')

if __name__ == '__main__' :
    main()