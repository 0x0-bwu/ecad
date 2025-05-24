import os
import sys
sys.path.append(os.path.dirname(__file__) + '/../../../build.release/lib')
import PyEcad as ecad

#constant variables
MAT_CU = "Cu"
mgr = ecad.DataMgr
chip_locations = ecad.coord_to_fpoint2d([-5.23, 8.93])

def setup_design(name, chip_locations, work_dir, save = True) :
    
    def setup_material(database) :
        mat_cu = mgr.create_material_def(database, MAT_CU)
        mat_cu.set_property(ecad.MaterialPropId.THERMAL_CONDUCTIVITY, mgr.create_polynomial_material_prop([[437.6, -0.165, 1.825e-4, -1.427e-7, 3.979e-11]]))
        mat_cu.set_property(ecad.MaterialPropId.SPECIFIC_HEAT, mgr.create_polynomial_material_prop([[342.8, 0.134, 5.535e-5, -1.971e-7, 1.141e-10]]))
        mat_cu.set_property(ecad.MaterialPropId.MASS_DENSITY, mgr.create_simple_material_prop(8850))

    def create_sic_die_component_def(database) :
        sic_die = mgr.create_component_def(database, 'SicDie')
        sic_die.set_component_type(ecad.ComponentType.IC)
        sic_die.set_solder_ball_bump_height(0.1)
        sic_die.set_solder_filling_material(MAT_CU)
        sic_die.set_boundary(mgr.create_shape_rectangle(database.coord_units, ecad.FPoint2D(-2.545, -2.02), ecad.FPoint2D(2.545, 2.02)))        
        sic_die.set_material(MAT_CU)
        sic_die.set_height(0.18)
        return sic_die

    def create_base_layout(database) :
        base_cell = mgr.create_circuit_cell(database, 'Base')
        base_layout = base_cell.get_layout_view()
        coord_units = database.coord_units
        outline = mgr.create_shape_polygon(coord_units, [-52.2, -29.7, 52.2, -29.7, 52.5, 29.7, -52.2, 29.7],0)
        base_layout.set_boundary(outline)
        
        mgr.create_net(base_layout, 'Source')
        top_layer = base_layout.append_layer(mgr.create_stackup_layer('TopCuLayer', ecad.LayerType.CONDUCTING_LAYER, 0, 0.3, MAT_CU, MAT_CU))

        return base_layout
    
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
        assert(len(locations) == 1)
        coord_units = database.coord_units
        cell = mgr.create_circuit_cell(database, 'BotBridgeCell')
        layout = cell.get_layout_view()

        layer1 = layout.append_layer(mgr.create_stackup_layer('Layer1', ecad.LayerType.DIELECTRIC_LAYER, 0, 0.3, MAT_CU, MAT_CU))
        layout.set_boundary(mgr.create_shape_polygon(coord_units, [-16.75, -12.5, 16.75, -12.5, 16.75, 12.5, -16.75, 12.5], 0))

        sic_die = mgr.find_component_def_by_name(database, 'SicDie')
        die_comps = [mgr.create_component(layout, f'Die{i + 1}', sic_die, layer1, mgr.create_transform_2d(coord_units, 1, 0, locations[i], ecad.Mirror2D.NO), False) for i in range(1)]

        return layout

    
    
    mgr.remove_database(name)
    database = mgr.create_database(name)
    database.coord_units = ecad.CoordUnits(ecad.CoordUnit.MILLIMETER)

    setup_material(database)

    create_sic_die_component_def(database)

    base_layout = create_base_layout(database)


    bot_bridge_layout = create_bot_bridge_layout(database, chip_locations[0:6])
    bot_bridge_layer_map = create_default_layer_map(database, bot_bridge_layout, base_layout, 'BotBridgeLayerMap')

    bot_bridge1 = mgr.create_cell_inst(base_layout, 'BotBridge1', bot_bridge_layout, mgr.create_transform_2d(database.coord_units, 1, 0, ecad.FVector2D(-17.75, 13), ecad.Mirror2D.NO))
    bot_bridge1.set_layer_map(bot_bridge_layer_map)

    base_layout.flatten(ecad.FlattenOption.NO)

    if save :
        mgr.save_database(database, work_dir + '/Tiny.ecad')
    return base_layout

def main() :
    print('ecad version: ' + ecad.__version__)

    mgr.init(ecad.LogLevel.TRACE)
    work_dir = os.path.dirname(__file__) + '/data/design'
    layout = setup_design('Tiny', chip_locations, work_dir, True)
    assert(layout and layout.get_database().get_name() == 'Tiny')
    assert(os.path.exists(work_dir + '/Tiny.ecad'))
    mgr.shut_down()

if __name__ == '__main__' :
    main()