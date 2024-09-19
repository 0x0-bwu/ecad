#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "EDataMgr.h"
using namespace ecad;
namespace py = pybind11;

void ecad_init_design(py::module_ & m)
{
    py::class_<ICell>(m, "Cell")
        .def("get_name", &ICell::GetName, py::return_value_policy::reference)
    ;

    py::class_<IDatabase>(m, "Database")
        .def("get_name", &IDatabase::GetName, py::return_value_policy::reference)
        .def("find_cell_by_name", &IDatabase::FindCellByName, py::return_value_policy::reference)
        .def("get_top_cells", [](const IDatabase & database) {
            std::vector<Ptr<ICell>> cells;
            database.GetTopCells(cells);
            return cells;
        }, py::return_value_policy::reference)
    ;

    py::class_<ILayoutView>(m, "LayoutView")

    ;


}