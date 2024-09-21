#pragma once
#include "PyEcadCommon.hpp"

void ecad_init_design(py::module_ & m)
{
    py::class_<ICell>(m, "Cell")
        .def("get_name", &ICell::GetName, py::return_value_policy::reference)
        .def("get_layout_view", &ICell::GetLayoutView, py::return_value_policy::reference)
        .def("get_flattened_layout_view", &ICell::GetFlattenedLayoutView, py::return_value_policy::reference)
    ;

    py::class_<IDatabase>(m, "Database")
        .def("get_name", &IDatabase::GetName, py::return_value_policy::reference)
        .def_property("coord_units", &IDatabase::GetCoordUnits, &IDatabase::SetCoordUnits)
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

    py::class_<IComponentDefPin>(m, "ComponentDefPin")
    ;

    py::class_<IComponentDef>(m, "ComponentDef")
        .def("set_component_type", &IComponentDef::SetComponentType)
        .def("set_solder_ball_bump_height", &IComponentDef::SetSolderBallBumpHeight)
        .def("set_solder_filling_material", &IComponentDef::SetSolderFillingMaterial)
        .def("set_bonding_box", &IComponentDef::SetBondingBox)
        .def("set_material", &IComponentDef::SetMaterial)
        .def("set_height", &IComponentDef::SetHeight)
    ;

    py::class_<IPadstackDefData>(m, "PadstackDefData")
        .def("set_top_solder_bump_material", &IPadstackDefData::SetTopSolderBumpMaterial)
        .def("set_bot_solder_ball_material", &IPadstackDefData::SetBotSolderBallMaterial)
        .def("set_top_solder_bump_parameters", [](IPadstackDefData & data, Ptr<EShape> shape, EFloat thickness){
            data.SetTopSolderBumpParameters(shape->Clone(), thickness);
        })
        .def("set_bot_solder_ball_parameters", [](IPadstackDefData & data, Ptr<EShape> shape, EFloat thickness){
            data.SetBotSolderBallParameters(shape->Clone(), thickness);
        })
    ;

    py::class_<IPadstackDef>(m, "PadstackDef")
        .def("set_padstack_def_data", [](IPadstackDef & padstackDef, Ptr<IPadstackDefData> data){
            padstackDef.SetPadstackDefData(data->Clone());
        })
    ;

}