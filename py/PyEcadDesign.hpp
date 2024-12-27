#pragma once
#include "PyEcadCommon.hpp"

void ecad_init_design(py::module_ & m)
{
    py::class_<ICell>(m, "Cell")
        .def("get_name", &ICell::GetName, py::return_value_policy::reference)
        .def("get_layout_view", &ICell::GetLayoutView, py::return_value_policy::reference)
        .def("get_flattened_layout_view", &ICell::GetFlattenedLayoutView, py::return_value_policy::reference)
    ;

    py::class_<ICellInst>(m, "CellInst")
        .def("set_layer_map", &ICellInst::SetLayerMap)
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
        .def("get_boundary", &IComponent::GetBoundary, py::return_value_policy::reference)
        .def("set_loss_power", &IComponent::SetLossPower)
        .def("set_dynamic_power_scenario", &IComponent::SetDynamicPowerScenario)
    ;

    py::class_<INet>(m, "Net")
    ;

    py::class_<ILayer>(m, "Layer")
        .def("get_layer_id", &ILayer::GetLayerId)
    ;

    py::class_<IIterator<ILayer>>(m, "LayerIter")
        .def("next", &IIterator<ILayer>::Next, py::return_value_policy::reference)
    ;

    py::class_<ILayerMap>(m, "LayerMap")
        .def("set_mapping", &ILayerMap::SetMapping)
    ;
    
    py::class_<IPrimitive>(m, "Primitive")
        .def("get_bondwire_from_primitive", &IPrimitive::GetBondwireFromPrimitive, py::return_value_policy::reference)
    ;

    py::class_<IIterator<IPrimitive>>(m, "PrimitiveIter")
        .def("next", &IIterator<IPrimitive>::Next, py::return_value_policy::reference)
    ;

    py::class_<ILayoutView>(m, "LayoutView")
        .def("get_name", &ILayoutView::GetName, py::return_value_policy::reference)
        .def("get_coord_units", &ILayoutView::GetCoordUnits, py::return_value_policy::reference)
        .def("get_database", &ILayoutView::GetDatabase, py::return_value_policy::reference)
        .def("find_component_by_name", &ILayoutView::FindComponentByName, py::return_value_policy::reference)
        .def("set_boundary", [](ILayoutView & layout, CPtr<EShape> shape){
            layout.SetBoundary(shape->Clone());
        })
        .def("append_layer", [](ILayoutView & layout, CPtr<ILayer> layer){
            return layout.AppendLayer(layer->Clone());
        })
        .def("get_layer_iter", &ILayoutView::GetLayerIter)
        .def("get_primitive_iter", &ILayoutView::GetPrimitiveIter)
        .def("flatten", &ILayoutView::Flatten)
        .def("run_thermal_simulation", [](ILayoutView & layout, const EThermalStaticSimulationSetup & simulationSetup){
            std::vector<EFloat> temperatures;
            auto range = layout.RunThermalSimulation(simulationSetup, temperatures);
            return std::make_tuple(range.first, range.second, temperatures);
        })
        .def("run_thermal_simulation", py::overload_cast<const EThermalTransientSimulationSetup &, const EThermalTransientExcitation &>(&ILayoutView::RunThermalSimulation))
    ;

    py::class_<IBondwire>(m, "Bondwire")
        .def("get_name", &IBondwire::GetName, py::return_value_policy::reference)
        .def("set_start_layer", py::overload_cast<ELayerId, const EPoint2D &, bool>(&IBondwire::SetStartLayer))
        .def("set_start_layer", py::overload_cast<ELayerId>(&IBondwire::SetStartLayer))
        .def("set_end_layer", py::overload_cast<ELayerId, const EPoint2D &, bool>(&IBondwire::SetEndLayer))
        .def("set_end_layer", py::overload_cast<ELayerId>(&IBondwire::SetEndLayer))
        .def("set_start_component", &IBondwire::SetStartComponent)
        .def("set_end_component", &IBondwire::SetEndComponent)
        .def("set_bondwire_type", &IBondwire::SetBondwireType)
        .def("set_solder_joints", &IBondwire::SetSolderJoints)
        .def("set_material", &IBondwire::SetMaterial)
        .def("set_height", &IBondwire::SetHeight)
        .def("get_height", &IBondwire::GetHeight)
        .def("get_radius", &IBondwire::GetRadius)
        .def("set_current", &IBondwire::SetCurrent)
        .def("set_dynamic_power_scenario", &IBondwire::SetDynamicPowerScenario)
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
        .def("set_boundary", [](IComponentDef & compDef, CPtr<EShape> shape){
            compDef.SetBoundary(shape->Clone());
        })
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