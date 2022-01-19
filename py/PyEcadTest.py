import os
import sys

current_dir = os.path.dirname(__file__)
ecad_lib_path = os.path.abspath(current_dir + "/../build/")
sys.path.append(ecad_lib_path)

import libEcad as ecad
from libEcad import EPoint2D

###EDataMgr
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
txt_file = current_dir + '/' + db_name + '.txt'
if os.path.isfile(txt_file) :
    os.remove(txt_file)
assert(mgr.save_database(database, txt_file))
assert(os.path.isfile(txt_file) == True)
xml_file = current_dir + '/' + db_name + '.xml'
if os.path.isfile(xml_file) :
    os.remove(xml_file)
assert(mgr.save_database(database, xml_file, ecad.EArchiveFormat.XML))
assert(os.path.isfile(xml_file) == True)

#load database
assert(mgr.load_database(database, txt_file))
assert(mgr.load_database(database, xml_file, ecad.EArchiveFormat.XML))
os.remove(txt_file)
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


###ECell


###EPoint
p = EPoint2D(2, 3)
assert(p.x == 2 and p.y == 3)
p.x = 3
p.y = 2
assert(p.x == 3 and p.y == 2)

###ETransform
trans = ecad.make_transform_2d(0.5, 0, EPoint2D(3, 5), True)
m = trans.get_transform()
assert(m.a11 == -.5 and m.a13 == 3.0 and m.a22 == 0.5 and m.a23 == 5.0)


###EPolygonData
points = [EPoint2D(0, 0), EPoint2D(10, 0), EPoint2D(10, 10), EPoint2D(0, 10)]
polygon = ecad.EPolygonData()
polygon.set_points(points)
assert(polygon.size() == 4)

print("every thing is fine")

