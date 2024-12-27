#pragma once
#include "PyEcadCommon.hpp"
#include "generic/geometry/Utility.hpp"

void ecad_init_geometry(py::module_ & m)
{
    using namespace generic::geometry;
    m.def("contains", &Contains<Polygon2D<ECoord>, Box2D<ECoord>>);
    m.def("contains", &Contains<Polygon2D<ECoord>, Polygon2D<ECoord>>);
}
