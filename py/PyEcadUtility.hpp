#pragma once
#include "PyEcadCommon.hpp"
#include "utility/ELayoutRetriever.h"

void ecad_init_utility(py::module_ & m)
{
    using namespace ecad::utils;
    py::class_<ELayoutRetriever>(m, "LayoutRetriever")
        .def(py::init<CPtr<ILayoutView>>())
        .def("get_component_height_thickness", [](const ELayoutRetriever & retriever, CPtr<IComponent> comp){
            EFloat elevation{0}, thickness{0};
            auto res = retriever.GetComponentHeightThickness(comp, elevation , thickness);
            return std::make_tuple(res, elevation, thickness);
        })
    ;

}
