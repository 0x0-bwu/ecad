#pragma once
#include <pybind11/pybind11.h>
#include "design/EDatabase.h"
#include "EDataMgr.h"

using namespace ecad;
namespace py = pybind11;

void ecad_init_design(py::module_ & m)
{
    py::class_<IDatabase>(m, "IDatabase")
        .def("get_name", [](const IDatabase & database){
            return dynamic_cast<const EDatabase &>(database).GetName();
        }, py::return_value_policy::reference)
        .def("find_cell_by_name", [](const IDatabase & database, const std::string & name){
            return dynamic_cast<const EDatabase &>(database).FindCellByName(name);
        }, py::return_value_policy::reference)
    ;
}