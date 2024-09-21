#pragma once
#include "PyEcadCommon.hpp"

void ecad_init_design(py::module_ & m)
{
    py::class_<ICell>(m, "Cell")
        .def("get_name", &ICell::GetName, py::return_value_policy::reference)
        .def("get_flattened_layout_view", &ICell::GetFlattenedLayoutView, py::return_value_policy::reference)
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

    py::class_<IComponent>(m, "Component")
        .def("get_name", &IComponent::GetName, py::return_value_policy::reference)
        .def("get_bounding_box", &IComponent::GetBoundingBox)
    ;

    py::class_<ILayoutView>(m, "LayoutView")
        .def("get_name", &ILayoutView::GetName, py::return_value_policy::reference)
        .def("get_coord_units", &ILayoutView::GetCoordUnits, py::return_value_policy::reference)
        .def("find_component_by_name", &ILayoutView::FindComponentByName, py::return_value_policy::reference)
        .def("run_thermal_simulation", [](ILayoutView & layout, const EThermalStaticSimulationSetup & simulationSetup){
            std::vector<EFloat> temperatures;
            auto range = layout.RunThermalSimulation(simulationSetup, temperatures);
            return std::make_tuple(range.first, range.second, temperatures);
        })
    ;

    py::class_<IMaterialDef>(m, "MaterialDef")
        .def("set_property", [](IMaterialDef & mat, EMaterialPropId id, Ptr<IMaterialProp> prop){
            mat.SetProperty(id, prop->Clone());
        })
        .def("set_material_type", &IMaterialDef::SetMaterialType)
    ;

    py::class_<IMaterialProp>(m, "MaterialProp")
    ;

}