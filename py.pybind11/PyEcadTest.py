import os
import sys

py_ecad = os.path.abspath(os.path.dirname(__file__) + '/../build.release/lib')
sys.path.append(py_ecad)
import PyEcad as ecad

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
def test_static_thermal_flow() :

    design_filename = os.path.dirname(__file__) + '/../examples/wolfspeed/data/design/CAS300M12BM2.ecad'
    mgr = ecad.DataMgr
    mgr.init(ecad.LogLevel.TRACE)
    database = mgr.load_database(design_filename)
    print(database.get_name())
    cell = database.find_cell_by_name("Base")
    layout = cell.get_flattened_layout_view()

    # setup = ecad.ThermalStaticSimulationSetup()
    # results = layout.run_thermal_simulation(setup)
    # print(results)

def main() :
    print('ecad version: ' + ecad.__version__)
    test_data_mgr()
    test_static_thermal_flow()
    print('every thing is fine')

if __name__ == '__main__' :
    main()