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
assert(mgr.save_database(database, txt_file))
xml_file = current_dir + '/' + db_name + '.xml'
assert(mgr.save_database(database, xml_file, libEcad.EArchiveFormat.XML))

#load database
assert(mgr.load_database(database, txt_file))
assert(mgr.load_database(database, xml_file, libEcad.EArchiveFormat.XML))

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



###EDataMgr

###ECell

###ECell
print("end")

