from gc import collect
import os
import sys

current_dir = os.path.dirname(__file__)
ecad_lib_path = os.path.abspath(current_dir + "/../build/")
sys.path.append(ecad_lib_path)

import PyEcad as ecad
from PyEcad import EPoint2D

def print_test_info(func) :
    def wrapper(*args, **kwargs):
        print(f"running: {func.__qualname__}")
        return func(*args, **kwargs)

    return wrapper

###EDataMgr
@print_test_info
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
    layer_m = mgr.create_stackup_layer_with_default_materials("m", ecad.ELayerType.CONDUCTINGLAYER, 0, 10)
    assert(layer_m.thickness == 10)
    layer_v = mgr.create_stackup_layer_with_default_materials("v", ecad.ELayerType.DIELECTRICLAYER, 0, 20)
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
    geom = mgr.create_geometry_2d(layout, ecad.ELayerId.NOLAYER, ecad.ENetId.NONET, eshape3)
    assert(geom != None)

    #create text
    s = "hello world"
    text = mgr.create_text(layout, ecad.ELayerId.NOLAYER, ecad.ETransform2D(), s)
    assert(text.text == s)

    #shut down
    mgr.shutdown(False)
    mgr.shutdown()

###EDatabase
@print_test_info
def test_database() :

    #create database
    db_name = "test"
    database = ecad.EDatabase(db_name)
    
    #clone
    database_copy = database.clone()
    assert(database_copy.suuid != database.suuid)
    assert(database_copy.name == database.name)

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
    next_def_name = database.get_next_def_name("def", ecad.EDefinitionType.PADSTACKDEF)
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
    circuit_cells = database.get_circuit_cells()
    assert(len(circuit_cells) == 1)
    assert(circuit_cells[0].name == cell_name)

    #get_top_cells
    top_cells = database.get_top_cells()
    assert(len(top_cells) == 1)
    
    #flatten
    assert(database.flatten(top_cells[0]))

    #get layer map collection
    layer_map_collection = database.get_layer_map_collection()

    #create layer map
    layer_map_name = "layer_map"
    layer_map = database.create_layer_map(layer_map_name)
    assert(layer_map != None)
    
    #add layer map
    assert(database.add_layer_map(layer_map) == False)

    #get padstack def collection
    padstack_def_collection = database.get_padstack_def_collection()

    #create padstack def
    padstack_def_name = "padstack"
    padstack_def = database.create_padstack_def(padstack_def_name)
    assert(padstack_def != None)
    assert(padstack_def.name == padstack_def_name)

    #get cell iter
    iterator = database.get_cell_iter()
    next = iterator.next()
    assert(next.name == cell_name)
    next = iterator.next()
    assert(next == None)

    #get layer map iter
    iterator = database.get_layer_map_iter()
    next = iterator.next()
    assert(next.name == layer_map_name)
    next = iterator.next()
    assert(next == None)

    #get padstack def iter
    iterator = database.get_padstack_def_iter()
    next = iterator.next()
    assert(next.name == padstack_def_name)
    next = iterator.next()
    assert(next == None)

    #name
    assert(database.name == db_name)

    #suuid
    suuid = database.suuid

    #clear
    database.clear()
    assert(cell_collection.size() == 0)
    assert(layer_map_collection.size() == 0)
    assert(padstack_def_collection.size() == 0)

###ECellCollection
@print_test_info
def test_cell_collection() :
    #create database
    db_name = "test"
    database = ecad.EDatabase(db_name)

    #create circuit cell
    r = range(0, 1)
    for i in r :
        database.create_circuit_cell('cell_{}'.format(i + 1))
    
    cell_collection = database.get_cell_collection()
    #__len__/size
    assert(len(cell_collection) == 1)
    assert(cell_collection.size() == 1)

    #__getitem__/get cell iter
    assert(cell_collection['cell_1'].suuid == cell_collection.get_cell_iter().next().suuid)

    #__contains__
    assert(('cell_0' in cell_collection) == False)

    #clear
    cell_collection.clear()
    assert(len(cell_collection) == 0)

###ECell
@print_test_info
def test_cell() :
    #create database
    db_name = "test"
    database = ecad.EDatabase(db_name)

    #create cell
    cell_name = "cell"
    cell = database.create_circuit_cell(cell_name)

    #create layout
    layout_name = "layout"
    layout = ecad.ELayoutView(layout_name, None)

    #set layout view
    assert(cell.set_layout_view(layout))

    #get cell type
    assert(cell.get_cell_type() == ecad.ECellType.CIRCUITCELL)

    #get database
    assert(cell.get_database().suuid == database.suuid)

    #get layout view
    layout = cell.get_layout_view() 
    assert(layout != None)
    assert(layout.name == layout_name)
    assert(layout.get_cell().suuid == cell.suuid)

    #get flattened layout view
    flattened = cell.get_flattened_layout_view()
    assert(flattened != None)

    #get definition type
    assert(ecad.EDefinitionType.CELL == cell.get_definition_type())

    #name/suuid
    assert(cell_name == cell.name)
    assert(cell.suuid)

###ELayoutView
@print_test_info
def test_layout_view() :

     #create database
    db_name = "test"
    database = ecad.EDatabase(db_name)

    #create circuit cell
    cell_name = "cell"
    cell = database.create_circuit_cell(cell_name)
    
    #get layout view
    layout = cell.get_layout_view()

    #name/suuid
    assert(layout.name == cell_name)
    assert(layout.suuid)

    #get net iter
    net_iter = layout.get_net_iter()

    #get layer iter
    layer_iter = layout.get_layer_iter()

    #get conn obj iter
    conn_obj_iter = layout.get_conn_obj_iter()

    #get primitive iter
    primitve_iter = layout.get_primitive_iter()

    #get hierarchy obj iter
    hierarchy_obj_iter = layout.get_hierarchy_obj_iter()

    #get padstask inst iter
    padstack_inst_iter = layout.get_padstack_inst_iter()

    #append layer
    layer = ecad.EStackupLayer("layer0", ecad.ELayerType.CONDUCTINGLAYER)
    assert(ecad.ELayerId(0) == layout.append_layer(layer))

    #append layers
    layers = [ ecad.EStackupLayer("layer" + str(i), ecad.ELayerType.CONDUCTINGLAYER) for i in range(1, 3)]
    assert(layout.append_layers(layers) == [1, 2])

    #get stackup layers
    layers = layout.get_stackup_layers()
    assert(len(layers) == 3)

    #add default dielectric layers
    layer_map = layout.add_default_dielectric_layers()
    assert(layer_map.get_mapping_forward(ecad.ELayerId(1)) == 2)
    assert(layer_map.get_mapping_backward(ecad.ELayerId(4)) == 2)

    #create net
    net_name = "net"
    net = layout.create_net(net_name)
    net_iter = layout.get_net_iter()
    assert(net.suuid == net_iter.next().suuid)

    #create padstack inst
    padstack_name = "padstack"
    padstack_def = ecad.EPadstackDef("padstackdef")
    padstack_inst = layout.create_padstack_inst(padstack_name, padstack_def, ecad.ENetId.NONET, ecad.ELayerId(0), ecad.ELayerId(1), database.create_layer_map("layer_map"), ecad.ETransform2D())
    padstack_inst_iter = layout.get_padstack_inst_iter()
    assert(padstack_inst.suuid == padstack_inst_iter.next().suuid)

    #create cell inst
    cell_name = "cell"
    cell = ecad.ECircuitCell(cell_name, None)
    cell_inst = layout.create_cell_inst(cell_name, cell.get_layout_view(), ecad.ETransform2D())
    cell_inst_iter = layout.get_cell_inst_iter()
    assert(cell_inst.suuid == cell_inst_iter.next().suuid)

    #create geometry 2d
    polygon_data = ecad.EPolygonData()
    polygon_data.set_points([EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)])
    shape_polygon = ecad.EPolygon()
    shape_polygon.shape = polygon_data
    geom = layout.create_geometry_2d(ecad.ELayerId(0), ecad.ENetId.NONET, shape_polygon)
    assert(geom != None)
    geom_iter = layout.get_primitive_iter()
    assert(geom.suuid == geom_iter.next().suuid)

    #create text
    text = layout.create_text(ecad.ELayerId.NOLAYER, ecad.ETransform2D(), "text")
    assert(text != None)

    prim_iter = layout.get_primitive_iter()
    prim = prim_iter.next()
    while prim :
        t = prim.get_text_from_primitive()
        if t :
            assert(t.suuid == text.suuid)
        g = prim.get_geometry_2d_from_primitive()
        if g :
            assert(g.suuid == geom.suuid)
        prim = prim_iter.next()

    #get net collection
    net_collection = layout.get_net_collection()
    net = net_collection[net_name]
    assert(net.name == net_name)

    #get layer collection
    layer_collection = layout.get_layer_collection()
    assert(len(layer_collection) == 5)

    #get conn obj collection
    conn_obj_collection = layout.get_conn_obj_collection()
    assert(len(conn_obj_collection) == 3)

    #get cell inst collection
    cell_inst_collection = layout.get_cell_inst_collection()
    assert(len(cell_inst_collection) == 1)

    #get primitive collection
    primitive_collection = layout.get_primitive_collection()
    assert(len(primitive_collection) == 2)

    #get hierarchy obj collection
    hierarchy_obj_collection = layout.get_hierarchy_obj_collection()
    assert(len(hierarchy_obj_collection) == 1)

    #get padstack inst collection
    padstack_inst_collection = layout.get_padstack_inst_collection()
    assert(len(padstack_inst_collection) == 1)

    #set boundary
    boundary = ecad.EPolygon()
    boundary.set_points([EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)])
    layout.set_boundary(boundary)

    #get boundary
    boundary = layout.get_boundary()

    #generate metal fraction mapping
    mf_settings = ecad.EMetalFractionMappingSettings()
    assert(layout.generate_metal_fraction_mapping(mf_settings))

    #connectivity extraction
    layout.connectivity_extraction()

    #flatten
    layout.flatten(ecad.EFlattenOption())

    #merge
    layout.merge(layout, ecad.ETransform2D())

    #map
    layout.map(layer_map)

###EPrimitive Collection
@print_test_info
def test_primitive_collection() :
    
    #create primitive collection
    collection = ecad.EPrimitiveCollection()

    #create text
    text = collection.create_text(ecad.ELayerId(-1), ecad.ETransform2D(), "text")
    assert(text.text == "text")

    #create geometry 2d
    shape_polygon = ecad.EPolygon()
    shape_polygon.set_points([EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)])
    geom = collection.create_geometry_2d(ecad.ELayerId(-1), ecad.ENetId(-1), shape_polygon)

    #add primitive
    copy = geom.clone()
    prim = collection.add_primitive(copy)

    #__len__/size
    assert(len(collection) == collection.size() == 3)

    #get primitive iter
    iter = collection.get_primitive_iter()
    curr = iter.next()
    assert(curr.get_text_from_primitive() != None)

    #map
    database = ecad.EDatabase("database")
    layer_map = database.create_layer_map("layer_map")
    collection.map(layer_map)

    #clear
    collection.clear()
    assert(len(collection) == 0)

###EPrimitve
@print_test_info
def test_primitive() :
        
    #create primitive collection
    collection = ecad.EPrimitiveCollection()

    #create text
    text = collection.create_text(ecad.ELayerId(-1), ecad.ETransform2D(), "text")
    assert(text.text == "text")

    #create geometry 2d
    shape_polygon = ecad.EPolygon()
    shape_polygon.set_points([EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)])
    geom = collection.create_geometry_2d(ecad.ELayerId(-1), ecad.ENetId(-1), shape_polygon)

    #get text from primitve
    assert(text.get_text_from_primitive())

    #get conn obj from primitive
    assert(geom.get_conn_obj_from_primitive().net == ecad.ENetId(-1))
    
    #get geometry 2d from primitive
    assert(geom.get_geometry_2d_from_primitive())

    #net
    assert(geom.net == ecad.ENetId(-1))

    #layer
    geom.layer = ecad.ELayerId(1)
    assert(ecad.ELayerId(1) == geom.layer)

    #get primitive type
    assert(ecad.EPrimitiveType.TEXT == text.get_primitive_type())

###EGeometry2D
@print_test_info
def test_geometry_2d() :
    #create geometry 2d
    geom = ecad.EGeometry2D(ecad.ELayerId.NOLAYER, ecad.ENetId.NONET)

    #set shape
    shape_polygon = ecad.EPolygon()
    shape_polygon.set_points([EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)])
    geom.set_shape(shape_polygon)

    #get shape
    shape = geom.get_shape()
    assert(shape.get_bbox().ll == EPoint2D(0, 0))

    #transform
    trans = ecad.make_transform_2d(1.0, 0.0, EPoint2D(10, 10))
    geom.transform(trans)
    shape = geom.get_shape()
    assert(shape.get_bbox().ll == EPoint2D(10, 10))

###EText
@print_test_info
def test_text() :
    #create text
    text = ecad.EText("text", ecad.ELayerId.NOLAYER, ecad.ENetId.NONET)
    
    #text
    assert(text.text == "text")

    #get position
    assert(text.get_position() == ecad.EPoint2D(0, 0))
    
###ECellInst Collection
@print_test_info
def test_cell_inst_collection() :
    #create cell inst collection
    collection = ecad.ECellInstCollection()

    #create cell inst
    inst = collection.create_cell_inst(None, "inst", None, ecad.ETransform2D())

    #add cell inst
    copy = inst.clone()
    collection.add_cell_inst(copy)

    #__len__/size
    assert(len(collection) == collection.size() == 2)

    #get cell inst iter
    iter = collection.get_cell_inst_iter()
    curr = iter.next()
    assert(curr.name == "inst")

    #clear
    collection.clear()
    assert(len(collection) == 0)

###ECellInst
@print_test_info
def test_cell_inst() :
    #create cell inst
    top_cell = ecad.ECircuitCell("top_cell", None)
    sub_cell = ecad.ECircuitCell("sub_cell", None)

    collection = top_cell.get_layout_view().get_cell_inst_collection()
    inst = collection.create_cell_inst(None, "inst", None, ecad.ETransform2D())

    #ref layout
    inst.ref_layout = top_cell.get_layout_view()
    assert(inst.ref_layout.suuid == top_cell.get_layout_view().suuid)

    #def layout
    inst.def_layout = sub_cell.get_layout_view()
    assert(inst.def_layout.suuid == sub_cell.get_layout_view().suuid)

    #get flattened layout view
    assert(inst.get_flattened_layout_view().suuid == sub_cell.get_flattened_layout_view().suuid)


###EHierarchyObjCollection
@print_test_info
def test_hierarchy_obj_collection() :
    #create hierarchy obj collection
    collection = ecad.EHierarchyObjCollection()
    
    #get cell inst collection
    cell_inst_collection = collection.get_cell_inst_collection()
    assert(len(cell_inst_collection) == 0)

    #get hierarchy obj iter
    iter = collection.get_hierarchy_obj_iter()
    assert(iter.current() == None)

    #__len__/size
    assert(len(collection) == collection.size() == 0)

###EPadstackInstCollection
@print_test_info
def test_padstack_inst_collection() :
    #create padstack inst collection
    collection = ecad.EPadstackInstCollection()

    #create padstack inst
    database = ecad.EDatabase("database")
    layer_map = database.create_layer_map("layer_map")
    padstack_inst = collection.create_padstack_inst("padstack", None, ecad.ENetId.NONET, ecad.ELayerId.NOLAYER, ecad.ELayerId.NOLAYER, layer_map, ecad.ETransform2D())

    #add padstack inst
    copy = padstack_inst.clone()
    padstack_copy = collection.add_padstack_inst(copy)

    #map
    collection.map(layer_map)

    #get padstack inst iter
    iter = collection.get_padstack_inst_iter()
    curr = iter.next()
    curr = iter.next()
    assert(iter.next() == None)

    #__len__/size
    assert(len(collection) == collection.size() == 2)

###EPadstackInst
@print_test_info
def test_padstack_inst() :

    #create padstack inst
    padstack_inst = ecad.EPadstackInst("padstack", None, ecad.ENetId.NONET)

    #net
    padstack_inst.net = ecad.ENetId(0)
    assert(padstack_inst.net == ecad.ENetId(0))

    #set layer range
    padstack_inst.set_layer_range(ecad.ELayerId(0), ecad.ELayerId(1))

    #get layer range
    top, bot = padstack_inst.get_layer_range()
    assert(top == ecad.ELayerId(0) and bot == ecad.ELayerId(1))

    #set/get layer map
    database = ecad.EDatabase("database")
    layer_map = database.create_layer_map("layer_map")
    padstack_inst.layer_map = layer_map
    assert(layer_map.suuid == padstack_inst.layer_map.suuid)

    #get padstack def
    assert(padstack_inst.get_padstack_def() == None)

    #get layer shape
    assert(padstack_inst.get_layer_shape(ecad.ELayerId(0)) == None)

###EPadstackDefData
@print_test_info
def test_padstack_def_data() :
    
    #create padstack def data
    def_data = ecad.EPadstackDefData()

    #material
    def_data.material = "copper"

    #clone
    copy = def_data.clone()
    assert(copy.material == def_data.material)
    
    #set layers
    def_data.set_layers(["layer1", "layer2"])

    #set pad parameters
    shape_polygon = ecad.EPolygon()
    shape_polygon.set_points([EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)])
    assert(def_data.set_pad_parameters(ecad.ELayerId(0), shape_polygon, ecad.EPoint2D(1, 2), 0.5) == True)
    assert(def_data.set_pad_parameters("layer3", shape_polygon, ecad.EPoint2D(0, 0), 0) == False)

    # get pad parameters
    offset, rotation = def_data.get_pad_parameters(ecad.ELayerId(0))
    assert(offset == EPoint2D(1, 2))
    assert(rotation == 0.5)
    assert(def_data.get_pad_shape("layer3") == None)

    #set via parameter
    def_data.set_via_parameters(shape_polygon, ecad.EPoint2D(0, 0), 0.5)

    #get via parameter
    offset, rotation = def_data.get_via_parameters()
    assert(offset == EPoint2D(0, 0))
    assert(rotation == 0.5)
    assert(def_data.get_via_shape() != None)

###EPadstackDef Collection
@print_test_info
def test_padstack_def_collection() :
    #create padstack def collection
    collection = ecad.EPadstackDefCollection()

    #__len__/size
    assert(len(collection) == collection.size() == 0)

    #get padstack def iter
    iter = collection.get_padstack_def_iter()
    assert(iter.next() == None)

    #clear
    collection.clear()

###EPadstackDef
@print_test_info
def test_padstack_def() :
    #create padstack def
    padstack_def = ecad.EPadstackDef("padstack_def")

    #set padstack def data
    def_data = ecad.EPadstackDefData()
    padstack_def.set_padstack_def_data(def_data)

    #get padstack def data
    assert(padstack_def.get_padstack_def_data())

###ConnObj Collection
@print_test_info
def test_conn_obj_collection() :
    #create conn obj collection
    collection = ecad.EConnObjCollection()

    #__len__/size
    assert(len(collection) == collection.size() == 0)

    #get primitive collection
    assert(collection.get_primitive_collection() != None)

    #get padstack inst collection
    assert(collection.get_padstack_inst_collection() != None)

    #get conn obj iter
    iter = collection.get_conn_obj_iter()
    assert(iter.next() == None)
    
###ELayerMapCollection
@print_test_info
def test_conn_obj_collection() :
    #create conn obj collection
    collection = ecad.ELayerMapCollection()

    #__len__/size
    assert(len(collection) == collection.size() == 0)

    #get layer map iter
    iter = collection.get_layer_map_iter()
    assert(iter.next() == None)

###ELayerMap
@print_test_info
def test_layer_map() :
    #create layer map
    layer_map = ecad.ELayerMap("layer_map", None)

    #clone
    copy = layer_map.clone()

    #get database
    assert(layer_map.get_database() == None)

    #set mapping
    layer_map.set_mapping(ecad.ELayerId(0), ecad.ELayerId(1))

    #get mapping forward/backward
    assert(layer_map.get_mapping_forward(ecad.ELayerId(0)) == ecad.ELayerId(1))
    assert(layer_map.get_mapping_backward(ecad.ELayerId(1)) == ecad.ELayerId(0))

    #mapping left/right
    copy.set_mapping(ecad.ELayerId(0), ecad.ELayerId(1))
    copy.set_mapping(ecad.ELayerId(1), ecad.ELayerId(0))

    layer_map.mapping_left(copy)
    assert(layer_map.get_mapping_backward(ecad.ELayerId(1)) == ecad.ELayerId(1))
    layer_map.mapping_right(copy)
    assert(layer_map.get_mapping_forward(ecad.ELayerId(1)) == ecad.ELayerId(0))

###ELayerCollection
@print_test_info
def test_layer_collection() :
    #create layer collection
    collection = ecad.ELayerCollection()

    #append layer
    layer = ecad.EStackupLayer("layer1", ecad.ELayerType.CONDUCTINGLAYER)
    assert(collection.append_layer(layer) == ecad.ELayerId(0))

    #append layers
    layers = [ecad.EStackupLayer("layer2", ecad.ELayerType.CONDUCTINGLAYER)]
    assert(collection.append_layers(layers) == [1])

    #add default die lectric layers
    layer_map = collection.add_default_dielectric_layers()
    assert(layer_map.get_mapping_forward(ecad.ELayerId(1)) == ecad.ELayerId(2))

    #get default layer map
    layer_map = collection.get_default_layer_map()
    assert(layer_map.get_mapping_forward(ecad.ELayerId(1)) == ecad.ELayerId(1))

    #get stackup layers
    layers = collection.get_stackup_layers()
    assert(len(layers) == 3)

    #get layer iter
    iter = collection.get_layer_iter()
    curr = iter.next()
    assert(curr.name == "layer1")

    #__len__/size
    assert(len(collection) == collection.size() == 3)

    #find layer by name
    assert(collection.find_layer_by_name("layer1").suuid == layers[0].suuid)

    #get next layer name
    assert(collection.get_next_layer_name("layer1") == "layer11")

###ELayer
@print_test_info
def test_layer() :
    #create stackup layer
    layer = ecad.EDataMgr.instance().create_stackup_layer_with_default_materials("layer", ecad.ELayerType.CONDUCTINGLAYER, 0, 10)

    #id
    assert(layer.layer_id == ecad.ELayerId.NOLAYER)

    #type
    assert(layer.get_layer_type() == ecad.ELayerType.CONDUCTINGLAYER)

    #get stackup layer from layer
    assert(layer.get_stackup_layer_from_layer() != None)

    #get via layer from layer
    assert(layer.get_via_layer_from_layer() == None)

    #elevation
    layer.elevation = -0.5
    assert(layer.elevation == -0.5)

    #thickness
    assert(layer.thickness == 10)

###ENetCollection
@print_test_info
def test_net_collection() :
    #create net collection
    collection = ecad.ENetCollection()

    #create net
    net = collection.create_net("net")
    
    #find net by name
    assert(collection.find_net_by_name("net").suuid == net.suuid)

    #clone
    copy = net.clone()

    #add net
    assert(collection.add_net(copy).name == "net1")

    #next net name
    assert(collection.next_net_name("net") == "net2")

    #get net iter
    iter = collection.get_net_iter()
    iter.next()
    iter.next()
    assert(iter.next() == None)

    #__len__/size
    assert(len(collection) == collection.size() == 2)

    #contains
    assert("net" in collection)

    #clear
    collection.clear()
    assert(len(collection) == 0)

###ENet
@print_test_info
def test_net() :
    #create net
    net = ecad.ENet("net")
    
    #net id
    net.net_id = ecad.ENetId(1)
    assert(net.net_id == ecad.ENetId(1))

###EShape
@print_test_info
def test_shape() :
    #create polygon with holes
    pwh = ecad.EPolygonWithHoles()
    assert(pwh.has_hole() == False)

###EPoint
@print_test_info
def test_point() :
    p = EPoint2D(2, 3)
    assert(p.x == 2 and p.y == 3)
    p.x = 3
    p.y = 2
    assert(p.x == 3 and p.y == 2)

###ETransform
@print_test_info
def test_transform() :
    trans = ecad.make_transform_2d(0.5, 0, EPoint2D(3, 5), ecad.EMirror2D.Y)
    m = trans.get_transform()
    assert(m.a11 == -.5 and m.a13 == 3.0 and m.a22 == 0.5 and m.a23 == 5.0)

###EPolygonData
@print_test_info
def test_polygon_data() :
    points = [EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)]
    polygon = ecad.EPolygonData()
    polygon.set_points(points)
    assert(polygon.size() == 4)

###Extension
@print_test_info
def test_extension() :
    #instance
    mgr = ecad.EDataMgr.instance()

    #gds
    gds_file = current_dir + '/../test/data/extension/gdsii/4004.gds'
    gds_database = mgr.create_database_from_gds('4004', gds_file)
    assert(gds_database)

    #xfl
    xfl_file = current_dir + '/../test/data/extension/xfl/fccsp.xfl'
    xfl_database = mgr.create_database_from_xfl('fccsp', xfl_file)
    assert(xfl_database)

    mgr.shutdown()

###Utilities
@print_test_info
def test_utilities() :
    #instance
    mgr = ecad.EDataMgr.instance()

    #xfl
    xfl_file = current_dir + '/../test/data/extension/xfl/qcom.xfl'
    xfl_database = mgr.create_database_from_xfl('fccsp', xfl_file)
    assert(xfl_database)

    top_cells = xfl_database.get_top_cells()
    assert(len(top_cells) == 1)

    layout = top_cells[0].get_layout_view()
    assert(layout)

    settings = ecad.ELayoutPolygonMergeSettings()
    settings.threads = 4
    settings.out_file = current_dir + '/../test/data/simulation/xfl'
    layout.merge_layer_polygons(settings)
    
    #gds
    gds_file = current_dir + '/../test/data/extension/gdsii/4004.gds'
    lyr_file = ''#current_dir + '/../test/data/extension/gdsii/ringo.elm'
    gds_database = mgr.create_database_from_gds('ringo', gds_file, lyr_file)
    assert(gds_database)

    top_cells = gds_database.get_top_cells()
    assert(len(top_cells) == 1)

    flattened_layout = top_cells[0].get_flattened_layout_view()
    assert(flattened_layout)

    settings = ecad.ELayoutPolygonMergeSettings()
    settings.threads = 4
    settings.out_file = current_dir + '/../test/data/simulation/gds'
    flattened_layout.merge_layer_polygons(settings)

    mgr.shutdown()

###Simulation
@print_test_info
def test_simulation() :
    pass

@print_test_info
def test_objects() :
    
    test_database()
    test_cell()
    test_layout_view()
    test_primitive()
    test_geometry_2d()
    test_text()
    test_cell_inst()
    test_padstack_inst()
    test_padstack_def_data()
    test_padstack_def()
    test_layer_map()
    test_layer()
    test_net()
    test_shape()
    test_point()
    test_transform()
    test_polygon_data()

@print_test_info  
def test_collections() :
    
    test_cell_collection()
    test_primitive_collection()
    test_cell_inst_collection()
    test_hierarchy_obj_collection()
    test_padstack_inst_collection()
    test_padstack_def_collection()
    test_conn_obj_collection()
    test_layer_collection()
    test_net_collection()
    
def main() :
    test_data_mgr()
    test_objects()
    test_collections()
    test_extension()
    test_utilities()
    test_simulation()

    print("every thing is fine")

if __name__ == '__main__' :
    main()