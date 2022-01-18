import os
import sys

current_dir = os.path.dirname(__file__)
ecad_lib_path = os.path.abspath(current_dir + "/../build/")
sys.path.append(ecad_lib_path)

import libEcad

###EDataMgr
#instance
mgr = libEcad.EDataMgr.instance()

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
assert(mgr.save_database(database, xml_file, libEcad.EArchiveFormat.XML))
assert(os.path.isfile(xml_file) == True)

#load database
assert(mgr.load_database(database, txt_file))
assert(mgr.load_database(database, xml_file, libEcad.EArchiveFormat.XML))
os.remove(txt_file)
os.remove(xml_file)

#create cell
cell_name = "cell"
cell = mgr.create_circuit_cell(database, cell_name)
assert(cell != None)

#find cell
found_cell = mgr.find_cell_by_name(database, cell_name)
assert(found_cell != None)
assert(found_cell.suuid() == cell.suuid())

#create net
net_name = "net"
layout = cell.get_layout_view()
net = mgr.create_net(layout, net_name)
assert(net != None)

net_iter = layout.get_net_iter()
next = net_iter.next()
assert(next.get_name() == net_name)
next = net_iter.next()
assert(next == None)

#create stackup layer
layer_m = mgr.create_stackup_layer("m", libEcad.ELayerType.ConductingLayer, 0, 10)
assert(layer_m.thickness == 10)
layer_v = mgr.create_stackup_layer("v", libEcad.ELayerType.DielectricLayer, 0, 20)
layer_v.elevation = -10
assert(layer_v.elevation == -10)

#create layer map
layer_map_name = "layer_map"
layer_map = mgr.create_layer_map(database, layer_map_name)
assert(layer_map.get_database().suuid() == database.suuid())

#create padstack def
padstack_def_name = "padstack_def"
padstack_def = mgr.create_padstack_def(database, padstack_def_name)

#create padstack def data
padstackdef_data = mgr.create_padstack_def_data()
###EDataMgr

###ECell

###ECell
print("end")

