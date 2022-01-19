import os
import sys

current_dir = os.path.dirname(__file__)
ecad_lib_path = os.path.abspath(current_dir + "/../build/")
sys.path.append(ecad_lib_path)

import libEcad as ecad
from libEcad import EPoint2D

###EDataMgr
def test_data_mgr() :
    #instance
    mgr = ecad.EDataMgr.instance()

    db_name = "test"
    #create database
    mgr.create_database(db_name)

    #open database
    database = mgr.open_database(db_name)
    assert(database != None)

    #remove database
    assert(mgr.remove_database(db_name))
    database = mgr.open_database(db_name)
    assert(database == None)

    #save database
    database = mgr.create_database(db_name)
    bin_file = current_dir + '/' + db_name + '.bin'
    if os.path.isfile(bin_file) :
        os.remove(bin_file)
    assert(mgr.save_database(database, bin_file))
    assert(os.path.isfile(bin_file) == True)
    xml_file = current_dir + '/' + db_name + '.xml'
    if os.path.isfile(xml_file) :
        os.remove(xml_file)
    assert(mgr.save_database(database, xml_file, ecad.EArchiveFormat.XML))
    assert(os.path.isfile(xml_file) == True)

    #load database
    assert(mgr.load_database(database, bin_file))
    assert(mgr.load_database(database, xml_file, ecad.EArchiveFormat.XML))
    os.remove(bin_file)
    os.remove(xml_file)

    #create cell
    cell_name = "cell"
    cell = mgr.create_circuit_cell(database, cell_name)
    assert(cell != None)

    #find cell
    found_cell = mgr.find_cell_by_name(database, cell_name)
    assert(found_cell != None)
    assert(found_cell.suuid == cell.suuid)

    #create net
    net_name = "net"
    layout = cell.get_layout_view()
    net = mgr.create_net(layout, net_name)
    assert(net != None)

    net_iter = layout.get_net_iter()
    next = net_iter.next()
    assert(next.name == net_name)
    next = net_iter.next()
    assert(next == None)

    #create stackup layer
    layer_m = mgr.create_stackup_layer("m", ecad.ELayerType.ConductingLayer, 0, 10)
    assert(layer_m.thickness == 10)
    layer_v = mgr.create_stackup_layer("v", ecad.ELayerType.DielectricLayer, 0, 20)
    layer_v.elevation = -10
    assert(layer_v.elevation == -10)

    #create layer map
    layer_map_name = "layer_map"
    layer_map = mgr.create_layer_map(database, layer_map_name)
    assert(layer_map.get_database().suuid == database.suuid)

    #create padstack def
    padstack_def_name = "padstack_def"
    padstack_def = mgr.create_padstack_def(database, padstack_def_name)

    #create padstack def data
    padstackdef_data = mgr.create_padstack_def_data()

    #create padstack inst
    padstack_inst_name = "padstack_inst"
    padstack_inst_tran = ecad.ETransform2D()
    padstack_inst = mgr.create_padstack_inst(layout, padstack_inst_name, padstack_def, ecad.ENetId(-1), ecad.ELayerId(-1), ecad.ELayerId(-1), layer_map, padstack_inst_tran)
    assert(padstack_inst != None)

    #create cell inst
    sub_cell_name = "sub_cell"
    sub_cell = mgr.create_circuit_cell(database, sub_cell_name)
    sub_layout = sub_cell.get_layout_view()
    cell_inst_name = "cell_inst"
    cell_inst_tran = ecad.ETransform2D()
    cell_inst = mgr.create_cell_inst(layout, cell_inst_name, sub_layout, cell_inst_tran)
    assert(cell_inst != None)

    #create shape polygon
    points = [EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)]
    eshape = mgr.create_shape_polygon(points)
    assert(eshape.shape.size() == 4)
    eshape2 = mgr.create_shape_polygon(ecad.EPolygonData())
    assert(eshape2.shape.size() == 0)

    #create shape polygon with holes
    eshape3 = mgr.create_shape_polygon_with_holes(ecad.EPolygonWithHolesData())
    assert(eshape3 != None)

    #create geometry 2d
    geom = mgr.create_geometry_2d(layout, ecad.ELayerId.noLayer, ecad.ENetId.noNet, eshape3)
    assert(geom != None)

    #create text
    s = "hello world"
    text = mgr.create_text(layout, ecad.ELayerId.noLayer, ecad.ETransform2D(), s)
    assert(text.text == s)

    #shut down
    mgr.shutdown(False)
    mgr.shutdown()

###EDatabase
def test_database() :

    #create database
    db_name = "test"
    database = ecad.EDatabase(db_name)

    #save
    bin_file = current_dir + '/' + db_name + '.bin'
    xml_file = current_dir + '/' + db_name + '.xml'
    if os.path.isfile(bin_file) :
        os.remove(bin_file)
    assert(database.save(bin_file))
    assert(os.path.isfile(bin_file) == True)
    if os.path.isfile(xml_file) :
        os.remove(xml_file)
    assert(database.save(xml_file, ecad.EArchiveFormat.XML))
    assert(os.path.isfile(xml_file) == True)

    #load
    assert(database.load(bin_file))
    assert(database.load(xml_file, ecad.EArchiveFormat.XML))
    os.remove(bin_file)
    os.remove(xml_file)

    #coord units
    coord_units = database.coord_units
    database.coord_units = coord_units

    #get next def name
    next_def_name = database.get_next_def_name("def", ecad.EDefinitionType.PadstackDef)
    assert(next_def_name == "def")

    #get cell collection
    cell_collection = database.get_cell_collection()

    #create cell
    cell_name = "cell"
    cell = database.create_circuit_cell(cell_name)
    assert(cell != None)

    #find cell
    found_cell = database.find_cell_by_name(cell_name)
    assert(found_cell != None)
    assert(found_cell.suuid == cell.suuid)

    #get_circuit_cells
    circuit_cells = []
    # res = database.get_circuit_cells(circuit_cells)
    # assert(len(circuit_cells) == 1)

###EPoint
def test_point() :
    p = EPoint2D(2, 3)
    assert(p.x == 2 and p.y == 3)
    p.x = 3
    p.y = 2
    assert(p.x == 3 and p.y == 2)

###ETransform
def test_transform() :
    trans = ecad.make_transform_2d(0.5, 0, EPoint2D(3, 5), True)
    m = trans.get_transform()
    assert(m.a11 == -.5 and m.a13 == 3.0 and m.a22 == 0.5 and m.a23 == 5.0)

###EPolygonData
def test_polygon_data() :
    points = [EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)]
    polygon = ecad.EPolygonData()
    polygon.set_points(points)
    assert(polygon.size() == 4)

def main() :
    test_data_mgr()
    test_database()
    test_point()
    test_transform()
    test_polygon_data()
    print("every thing is fine")

if __name__ == '__main__' :
    main()