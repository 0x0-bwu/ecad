#include "PyEcadBasic.hpp"
#include "PyEcadDesign.hpp"
#include "PyEcadUtility.hpp"
#include "PyEcadGeometry.hpp"
void ecad_init_datamgr(py::module_ & m)
{
    //DataMgr
    py::class_<EDataMgr, std::unique_ptr<EDataMgr, py::nodelete>>(m, "DataMgr")
        .def(py::init([]{ return std::unique_ptr<EDataMgr, py::nodelete>(&EDataMgr::Instance()); }))
        .def("init", [](ELogLevel level, const std::string & workDir)
            { EDataMgr::Instance().Init(level, workDir); })
        .def("init", [](ELogLevel level)
            { EDataMgr::Instance().Init(level); })
        .def("init", []
            { EDataMgr::Instance().Init(); })
        .def("create_database", [](const std::string & name)
            { return EDataMgr::Instance().CreateDatabase(name); }, py::return_value_policy::reference)
        .def("open_database", [](const std::string & name)
            { return EDataMgr::Instance().OpenDatabase(name); }, py::return_value_policy::reference)
        .def("remove_database", [](const std::string & name)
            { return EDataMgr::Instance().RemoveDatabase(name); })
        .def("shut_down", [](bool autoSave)
            { EDataMgr::Instance().ShutDown(autoSave); })
        .def("shut_down", []
            { EDataMgr::Instance().ShutDown(); })
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
        .def("save_database", [](CPtr<IDatabase> database, const std::string & archive, EArchiveFormat fmt)
            { EDataMgr::Instance().SaveDatabase(database, archive, fmt); })
        .def("save_database", [](CPtr<IDatabase> database, const std::string & archive)
            { EDataMgr::Instance().SaveDatabase(database, archive); })
        .def("load_database", [](const std::string & archive, EArchiveFormat fmt)
            { return EDataMgr::Instance().LoadDatabase(archive, fmt); }, py::return_value_policy::reference)
        .def("load_database", [](const std::string & archive)
            { return EDataMgr::Instance().LoadDatabase(archive); }, py::return_value_policy::reference)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

        // cell
        .def("create_circuit_cell", [](Ptr<IDatabase> database, const std::string & name)
            { return EDataMgr::Instance().CreateCircuitCell(database, name); }, py::return_value_policy::reference)

        // net
        .def("create_net", [](Ptr<ILayoutView> layout, const std::string & name)
            { return EDataMgr::Instance().CreateNet(layout, name); }, py::return_value_policy::reference)

        // layer
        .def("create_stackup_layer", [](const std::string & name, ELayerType type, EFloat elevation, EFloat thickness, const std::string & conductingMat, const std::string & dielectricMat)
            { return EDataMgr::Instance().CreateStackupLayer(name, type, elevation, thickness, conductingMat, dielectricMat); })

        // layermap
        .def("create_layer_map", [](Ptr<IDatabase> database, const std::string & name)
            { return EDataMgr::Instance().CreateLayerMap(database, name); }, py::return_value_policy::reference)

        // material
        .def("create_material_def", [](Ptr<IDatabase> database, const std::string & name)
            { return EDataMgr::Instance().CreateMaterialDef(database, name); }, py::return_value_policy::reference)
        .def("create_simple_material_prop", [](EFloat value)
            { return EDataMgr::Instance().CreateSimpleMaterialProp(value); }) 
        .def("create_polynomial_material_prop", [](std::vector<std::vector<EFloat>> coefficients)
            { return EDataMgr::Instance().CreatePolynomialMaterialProp(std::move(coefficients)); })
        
        // component def
        .def("create_component_def", [](Ptr<IDatabase> database, const std::string & name)
            { return EDataMgr::Instance().CreateComponentDef(database, name); }, py::return_value_policy::reference)
        .def("find_component_def_by_name", [](Ptr<IDatabase> database, const std::string & name)
            { return EDataMgr::Instance().FindComponentDefByName(database, name); }, py::return_value_policy::reference)

        .def("create_component_def_pin", [](Ptr<IComponentDef> compDef, const std::string & pinName, FPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef, ELayerId lyr)
            { return EDataMgr::Instance().CreateComponentDefPin(compDef, pinName, loc, type, psDef, lyr); }, py::return_value_policy::reference)
        .def("create_component_def_pin", [](Ptr<IComponentDef> compDef, const std::string & pinName, FPoint2D loc, EPinIOType type)
            { return EDataMgr::Instance().CreateComponentDefPin(compDef, pinName, loc, type); }, py::return_value_policy::reference)

        .def("create_component", [](Ptr<ILayoutView> layout, const std::string & name, CPtr<IComponentDef> compDef, ELayerId layer, const ETransform2D & transform, bool flipped)
            { return EDataMgr::Instance().CreateComponent(layout, name, compDef, layer, transform, flipped); }, py::return_value_policy::reference)

        // bondwire
        .def("create_bondwire", [](Ptr<ILayoutView> layout, std::string name, ENetId net, EFloat radius)
            { return EDataMgr::Instance().CreateBondwire(layout, std::move(name), net, radius); }, py::return_value_policy::reference)

        // padstack
        .def("create_padstack_def", [](Ptr<IDatabase> database, const std::string & name)
            { return EDataMgr::Instance().CreatePadstackDef(database, name); }, py::return_value_policy::reference)
        .def("create_padstack_def_data", []
            { return EDataMgr::Instance().CreatePadstackDefData(); })
        
        // cell inst
        .def("create_cell_inst", [](Ptr<ILayoutView> layout, const std::string & name, Ptr<ILayoutView> defLayout, const ETransform2D & transform)
            { return EDataMgr::Instance().CreateCellInst(layout, name, defLayout, transform); }, py::return_value_policy::reference)

        // primitive
        .def("create_geometry_2d", [](Ptr<ILayoutView> layout, ELayerId layer, ENetId net, CPtr<EShape> shape)
            { return EDataMgr::Instance().CreateGeometry2D(layout, layer, net, shape->Clone()); }, py::return_value_policy::reference)

        // shape
        .def("create_shape_rectangle", [](const ECoordUnits & coordUnits, const FPoint2D & ll, const FPoint2D & ur)
            { return EDataMgr::Instance().CreateShapeRectangle(coordUnits, ll, ur); })
        .def("create_shape_circle", [](const ECoordUnits & coordUnits, const FPoint2D & loc, EFloat radius)
            { return EDataMgr::Instance().CreateShapeCircle(coordUnits, loc, radius); })
        .def("create_shape_path", [](const ECoordUnits & coordUnits, const std::vector<FPoint2D> & points, EFloat width)
            { return EDataMgr::Instance().CreateShapePath(coordUnits, points, width); })
        .def("create_shape_polygon", [](const ECoordUnits & coordUnits, const std::vector<FPoint2D> & points, EFloat cornerR)
            { return EDataMgr::Instance().CreateShapePolygon(coordUnits, points, cornerR); })
        .def("create_shape_polygon", [](const ECoordUnits & coordUnits, const std::vector<FCoord> & coords, EFloat cornerR)
            { return EDataMgr::Instance().CreateShapePolygon(coordUnits, coords, cornerR); })
        .def("create_shape_polygon_with_holes", [](EPolygonWithHolesData pwh)
            { return EDataMgr::Instance().CreateShapePolygonWithHoles(std::move(pwh)); })
        .def("create_box", [](const ECoordUnits & coordUnits, const FPoint2D & ll, const FPoint2D & ur)
            { return EDataMgr::Instance().CreateBox(coordUnits, ll, ur); })

        .def("create_transform_2d", [](const ECoordUnits & coordUnits, EFloat scale, EFloat rotation, const FVector2D & offset, EMirror2D mirror)
            { return EDataMgr::Instance().CreateTransform2D(coordUnits, scale, rotation, offset, mirror); })
            
        // settings
        .def("hier_sep", []
            { return EDataMgr::Instance().HierSep(); })
        .def("set_threads", [](size_t threads)
            { return EDataMgr::Instance().SetThreads(threads); })
        .def("threads", []
            { return EDataMgr::Instance().Threads(); })   

    ;
}

PYBIND11_MODULE(PyEcad, ecad)
{
    ecad_init_basic(ecad);
    ecad_init_design(ecad);
    ecad_init_utility(ecad);
    ecad_init_datamgr(ecad);

    //sub modules
    py::module_ mGeom = ecad.def_submodule("geom", "geometry module");
    ecad_init_geometry(mGeom);
}