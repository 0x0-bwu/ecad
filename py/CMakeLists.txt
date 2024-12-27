pybind11_add_module(PyEcad PyEcad.cpp)
target_link_libraries(PyEcad PRIVATE Ecad)
