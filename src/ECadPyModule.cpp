#include "ECadCommon.h"

#ifdef ECAD_BOOST_PYTHON_SUPPORT
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/stl_iterator.hpp>
#include "generic/tools/Format.hpp"
#include "EPadstackDefCollection.h"
#include "ELayerMapCollection.h"
#include "EPadstackDefData.h"
#include "ECellCollection.h"
#include "ENetCollection.h"
#include "EPadstackInst.h"
#include "EPadstackDef.h"
#include "ELayoutView.h"
#include "EPrimitive.h"
#include "ECellInst.h"
#include "EDatabase.h"
#include "ELayerMap.h"
#include "EDataMgr.h"
#include "EConnObj.h"
#include "EObject.h"
#include "ELayer.h"
#include "EShape.h"
#include "ECell.h"
#include "ENet.h"

namespace {
    using namespace ecad;
    using namespace boost::python;

    template <typename T>
    std::vector<T> py_list_to_std_vector(const object & iterable)
    {
        try{
            return std::vector<T>(stl_input_iterator<T>(iterable), stl_input_iterator<T>());
        }
        catch (...) {
            throw(std::invalid_argument("Could not convert python list to std vector, illegal object type"));
        }
    }

    template <typename T>
    boost::python::list std_vector_to_py_list(const std::vector<T> & vec)
    {
        boost::python::list l;
        for(auto t : vec) l.append(t);
        return l;
    }

    /**
     * @brief Adapter a non-member function that returns a unique_ptr to
     * a python function object that returns a raw pointer but
     * explicitly passes ownership to Python.
     * 
     * The lambda delegates to the original function, 
     * and releases() ownership of the instance to Python,
     * and the call policy indicates that Python will take ownership of the value returned from the lambda.
     * The mpl::vector describes the call signature to Boost.Python,
     * allowing it to properly manage function dispatching between the languages.
     */
    template <typename T, typename ...Args>
    object adapt_unique(std::unique_ptr<T> (*fn)(Args...))
    {
        return make_function([fn](Args... args){ return fn(args...).release(); },
                                return_value_policy<manage_new_object>(),
                                boost::mpl::vector<T*, Args...>());
    }

    /**
     * @brief Adapter of member function
     */
    template <typename T, typename C, typename ...Args>
    object adapt_unique(std::unique_ptr<T> (C::*fn)(Args...))
    {
        return make_function([fn](C & self, Args... args){ return (self.*fn)(args...).release(); },
                                return_value_policy<manage_new_object>(),
                                boost::mpl::vector<T *, C &, Args...>());
    }

    /**
     * @brief Adapter of const member function
     */
    template <typename T, typename C, typename ...Args>
    object adapt_unique(std::unique_ptr<T> (C::*fn)(Args...) const)
    {
        return make_function([fn](C & self, Args... args){ return (self.*fn)(args...).release(); },
                                return_value_policy<manage_new_object>(),
                                boost::mpl::vector<T *, C &, Args...>());
    }

    //Wrappers
    std::vector<Ptr<ICell> > EDatabaseGetCircuitCellsWrap(const EDatabase & database)
    {
        std::vector<Ptr<ICell> > cells;
        database.GetCircuitCells(cells);
        return cells;
    }

    std::vector<Ptr<ICell> > EDatabaseGetTopCellsWrap(const EDatabase & database)
    {
        std::vector<Ptr<ICell> > cells;
        database.GetTopCells(cells);
        return cells;
    }

    bool EDatabaseAddLayerMapWrap(EDatabase & database, Ptr<ILayerMap> layerMap)
    {
        //todo, enhance, copy issue here
        return database.AddLayerMap(layerMap->Clone());
    }

    Ptr<IPrimitive> EDataMgrCreateGeometry2DWrap(EDataMgr & mgr, Ptr<ILayoutView> layout, ELayerId layer, ENetId net, Ptr<EShape> shape)
    {
        //todo, enhance, copy issue here
        return mgr.CreateGeometry2D(layout, layer, net, shape->Clone());
    }
    
    BOOST_PYTHON_FUNCTION_OVERLOADS(makeETransform2DWithNoMirror, makeETransform2D, 3, 4)
    
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDatabaseSaveBin, EDatabase::Save, 1, 2)
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDatabaseLoadBin, EDatabase::Load, 1, 2)

    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDataMgrSaveDatabaseBin, EDataMgr::SaveDatabase, 2, 3)
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDataMgrLoadDatabaseBin, EDataMgr::LoadDatabase, 2, 3)
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDataMgrShutDownWithAutoSave, EDataMgr::ShutDown, 0, 1)

    BOOST_PYTHON_MODULE(ECAD_LIB_NAME)
    {
        //Enum
        enum_<EArchiveFormat>("EArchiveFormat")
            .value("TXT", EArchiveFormat::TXT)
            .value("XML", EArchiveFormat::XML)
            .value("BIN", EArchiveFormat::BIN)
        ;

        enum_<ELayerId>("ELayerId")
            .value("noLayer", ELayerId::noLayer)
        ;

        enum_<ENetId>("ENetId")
            .value("noNet", ENetId::noNet)
        ;

        enum_<ELayerType>("ELayerType")
            .value("Invalid", ELayerType::Invalid)
            .value("DielectricLayer", ELayerType::DielectricLayer)
            .value("ConductingLayer", ELayerType::ConductingLayer)
            .value("MetalizedSignal", ELayerType::MetalizedSignal)
        ;

        enum_<EDefinitionType>("EDefinitionType")
            .value("Invalid", EDefinitionType::Invalid)
            .value("PadstackDef", EDefinitionType::PadstackDef)
            .value("LayerMap", EDefinitionType::LayerMap)
            .value("Cell", EDefinitionType::Cell)
        ;

        //Unit
        enum_<ECoordUnits::Unit>("ELengthUnit")
            .value("nm", ECoordUnits::Unit::Nanometer)
            .value("um", ECoordUnits::Unit::Micrometer)
            .value("mm", ECoordUnits::Unit::Millimeter)
            .value("m",  ECoordUnits::Unit::Meter)
        ;

        class_<ECoordUnits>("ECoordUnits", init<ECoordUnits::Unit, ECoordUnits::Unit>())
        ;

        //Point
        class_<EPoint2D>("EPoint2D", init<ECoord, ECoord>())
            .add_property("x",
                            +[](const EPoint2D & p){ return p[0]; },
                            make_function([](EPoint2D & p, ECoord x){ p[0] = x; }, default_call_policies(), boost::mpl::vector<void, EPoint2D &, ECoord>()))
            .add_property("y",
                            +[](const EPoint2D & p){ return p[1]; },
                            make_function([](EPoint2D & p, ECoord y){ p[1] = y; }, default_call_policies(), boost::mpl::vector<void, EPoint2D &, ECoord>()))
            .def("__str__", +[](const EPoint2D & p){ std::stringstream ss; ss << p; return ss.str(); })
        ;

        //Polygon
        class_<EPolygonData>("EPolygonData")
            .def("size", &EPolygonData::Size)
            .def("set_points", make_function([](EPolygonData & polygon, const boost::python::list & points){
                                                polygon.Set(py_list_to_std_vector<EPoint2D>(points));
                                             },
                                             default_call_policies(), boost::mpl::vector<void, EPolygonData &, const boost::python::list &>()))
            .def("__str__", +[](const EPolygonData & polygon){ std::stringstream ss; ss << polygon; return ss.str(); })
        ;

        class_<EPolygonWithHolesData>("EPolygonWithHolesData")
            .def("__str__", +[](const EPolygonWithHolesData & pwh){ std::stringstream ss; ss << pwh; return ss.str(); })
        ;

        //Transform
        class_<typename ETransform2D::Transform>("Transform2D")
            .add_property("a11", +[](const typename ETransform2D::Transform & t){ return t(0, 0); })
            .add_property("a12", +[](const typename ETransform2D::Transform & t){ return t(0, 1); })
            .add_property("a13", +[](const typename ETransform2D::Transform & t){ return t(0, 2); })
            .add_property("a21", +[](const typename ETransform2D::Transform & t){ return t(1, 0); })
            .add_property("a22", +[](const typename ETransform2D::Transform & t){ return t(1, 1); })
            .add_property("a23", +[](const typename ETransform2D::Transform & t){ return t(1, 2); })
        ;
        
        class_<ETransform2D>("ETransform2D")
            .def("append", &ETransform2D::Append)
            .def("get_transform", &ETransform2D::GetTransform, return_value_policy<copy_const_reference>())
        ;

        def("make_transform_2d", &makeETransform2D);
        def("make_transform_2d", &makeETransform2D, makeETransform2DWithNoMirror());

        //Shape
        class_<EShape, boost::noncopyable>("EShape", no_init)
            .def("has_hole", &EShape::hasHole)
        ;

        class_<ERectangle, bases<EShape> >("ERectangle")
        ;

        class_<EPath, bases<EShape> >("EPath")
        ;

        class_<EPolygon, bases<EShape> >("EPolygon")
            .def_readwrite("shape", &EPolygon::shape)
        ;

        class_<EPolygonWithHoles, bases<EShape> >("EPolygonWithHoles")
            .def_readwrite("shape", &EPolygonWithHoles::shape)
        ;
    
        //Object
        class_<EObject>("EObject")
            .add_property("name", make_function(&EObject::GetName, return_value_policy<copy_const_reference>()), &EObject::SetName)
            .add_property("suuid", &EObject::sUuid)
        ;

        //Definition
        class_<IDefinition, boost::noncopyable>("IDefinition", no_init)
        ;

        class_<EDefinition, bases<EObject, IDefinition> >("EDefinition", no_init)
        ;

        //Net
        class_<INet, boost::noncopyable>("INet", no_init)
        ;

        class_<ENet, bases<EObject, INet> >("ENet", no_init)
        ;

        class_<IIterator<INet>, boost::noncopyable>("INetIter", no_init)
        ;

        class_<ENetIterator, bases<IIterator<INet> >, boost::noncopyable>("ENetIter", no_init)
            .def("next", &ENetIterator::Next, return_internal_reference<>())
            .def("current", &ENetIterator::Current, return_internal_reference<>())
        ;

        //Layer
        class_<ILayer, boost::noncopyable>("ILayer", no_init)
        ;

        class_<ELayer, bases<EObject, ILayer>, boost::noncopyable>("ELayer", no_init)
        ;

        class_<EStackupLayer, bases<ELayer> >("EStackupLayer", init<std::string, ELayerType>())
            .add_property("elevation", &EStackupLayer::GetElevation, &EStackupLayer::SetElevation)
            .add_property("thickness", &EStackupLayer::GetThickness, &EStackupLayer::GetThickness)
        ;

        //Layer Map
        class_<ILayerMap, boost::noncopyable>("ILayerMap", no_init)
        ;

        class_<ELayerMap, bases<EDefinition, ILayerMap> >("ELayerMap", no_init)
            .def("get_database", &ELayerMap::GetDatabase, return_internal_reference<>())
        ;

        //Layer Map Collection
        class_<ILayerMapCollection, boost::noncopyable>("ILayerMapCollection", no_init)
        ;

        class_<ELayerMapCollection, bases<ILayerMapCollection> >("ELayerMapCollection", no_init)
            .def("size", &ELayerMapCollection::Size)
        ;

        //Layer Map Iterator
        class_<IIterator<ILayerMap>, boost::noncopyable>("ILayerMapIter", no_init)
        ;

        class_<ELayerMapIterator, bases<IIterator<ILayerMap> >, boost::noncopyable>("ELayerMapIter", no_init)
            .def("next", &ELayerMapIterator::Next, return_internal_reference<>())
            .def("current", &ELayerMapIterator::Current, return_internal_reference<>())
        ;

        //ConnObj
        class_<EConnObj, bases<EObject> >("EConnObj", no_init)
            .add_property("net", &EConnObj::GetNet, &EConnObj::SetNet)
        ;

        //Padstack Def
        class_<IPadstackDef, boost::noncopyable>("IPadstackDef", no_init)
        ;

        class_<EPadstackDef, bases<EDefinition, IPadstackDef> >("EPadstackDef", init<std::string>())
        ;

        //Padstack Def Collection
        class_<IPadstackDefCollection, boost::noncopyable>("IPadstackDefCollection", no_init)
        ;

        class_<EPadstackDefCollection, bases<IPadstackDefCollection> >("EPadstackDefCollection", no_init)
            .def("size", &EPadstackDefCollection::Size)
        ;

        //Padstack Def Iterator
        class_<IIterator<IPadstackDef>, boost::noncopyable>("IPadstackDefIter", no_init)
        ;

        class_<EPadstackDefIterator, bases<IIterator<IPadstackDef> >, boost::noncopyable>("EPadstackDefIter", no_init)
            .def("next", &EPadstackDefIterator::Next, return_internal_reference<>())
            .def("current", &EPadstackDefIterator::Current, return_internal_reference<>())
        ;

        //Padstack Def Data
        class_<IPadstackDefData, boost::noncopyable>("IPadstackDefData", no_init)
        ;

        class_<EPadstackDefData, bases<IPadstackDefData> >("EPadstackDefData", no_init)
        ;

        class_<IPadstackInst, boost::noncopyable>("IPadstackInst", no_init)
        ;

        class_<EPadstackInst, bases<EConnObj, IPadstackInst> >("EPadstackInst", no_init)
        ;

        //CellInst
        class_<ICellInst, boost::noncopyable>("ICellInst", no_init)
        ;

        class_<ECellInst, bases<ICellInst> >("ECellInst", no_init)
        ;

        //Primitive
        class_<IPrimitive, boost::noncopyable>("IPrimitive", no_init)
        ;

        class_<EPrimitive, bases<EConnObj, IPrimitive>, boost::noncopyable>("EPrimitive", no_init)
        ;

        class_<IText, boost::noncopyable>("IText", no_init)
        ;

        class_<EText, bases<EPrimitive, IText> >("EText", no_init)
            .def(init<std::string>())
            .def(init<std::string, ELayerId, ENetId>())
            .add_property("text", make_function(&EText::GetText, return_value_policy<copy_const_reference>()))
        ;

        //LayoutView
        class_<ILayoutView, boost::noncopyable>("ILayoutView", no_init)
        ;

        class_<ELayoutView, bases<ILayoutView> >("ELayoutView", no_init)
            .def("get_net_iter", adapt_unique(&ELayoutView::GetNetIter))
        ;

        //Cell
        class_<ICell, boost::noncopyable>("ICell", no_init)
        ;

        class_<ECell, bases<EDefinition, ICell>, boost::noncopyable>("ECell", no_init)
            .def("get_database", &ECell::GetDatabase, return_internal_reference<>())
            .def("get_layout_view", &ECell::GetLayoutView,  return_internal_reference<>()) 
        ;

        class_<ECircuitCell, bases<ECell> >("ECircuitCell", no_init)
            .def("get_layout_view", &ECircuitCell::GetLayoutView,  return_internal_reference<>()) 
        ;

        class_<std::vector<Ptr<ICell> > >("ECellContainer")
            .def(vector_indexing_suite<std::vector<Ptr<ICell> > >())
        ;

        //Cell Collection
        class_<ICellCollection, boost::noncopyable>("ICellCollection", no_init)
        ;

        class_<ECellCollection, bases<ICellCollection> >("ECellCollection", no_init)
            .def("size", &ECellCollection::Size)
        ;

        //Cell Iterator
        class_<IIterator<ICell>, boost::noncopyable>("ICellIter", no_init)
        ;

        class_<ECellIterator, bases<IIterator<ICell> >, boost::noncopyable>("ECellIter", no_init)
            .def("next", &ECellIterator::Next, return_internal_reference<>())
            .def("current", &ECellIterator::Current, return_internal_reference<>())
        ;

        //Database
        class_<IDatabase, std::shared_ptr<IDatabase>, boost::noncopyable>("IDatabase", no_init)
        ;
        
        class_<EDatabase, bases<IDatabase>, std::shared_ptr<EDatabase> >("EDatabase", init<std::string>())
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
            .def("save", &EDatabase::Save)
            .def("save", static_cast<bool(EDatabase::*)(const std::string &, EArchiveFormat) const>(&EDatabase::Save), EDatabaseSaveBin())
            .def("load", &EDatabase::Load)
            .def("load", static_cast<bool(EDatabase::*)(const std::string &, EArchiveFormat)>(&EDatabase::Load), EDatabaseLoadBin())
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
            .add_property("coord_units", make_function(&EDatabase::GetCoordUnits, return_value_policy<copy_const_reference>()), &EDatabase::SetCoordUnits)
            .def("get_next_def_name", &EDatabase::GetNextDefName)
            .def("get_cell_collection", &EDatabase::GetCellCollection, return_internal_reference<>())
            .def("create_circuit_cell", &EDatabase::CreateCircuitCell, return_internal_reference<>())
            .def("find_cell_by_name", &EDatabase::FindCellByName, return_internal_reference<>())
            .def("get_circuit_cells", &EDatabaseGetCircuitCellsWrap)
            .def("get_top_cells", &EDatabaseGetTopCellsWrap)
            .def("flatten", &EDatabase::Flatten)
            .def("get_layer_map_collection", &EDatabase::GetLayerMapCollection, return_internal_reference<>())
            .def("create_layer_map", &EDatabase::CreateLayerMap, return_internal_reference<>())
            .def("add_layer_map",&EDatabaseAddLayerMapWrap)
            .def("get_padstack_def_collection", &EDatabase::GetPadstackDefCollection, return_internal_reference<>())
            .def("create_padstack_def", &EDatabase::CreatePadstackDef, return_internal_reference<>())
            .def("get_cell_iter", adapt_unique(&EDatabase::GetCellIter))
            .def("get_layer_map_iter", adapt_unique(&EDatabase::GetLayerMapIter))
            .def("get_padstack_def_iter", adapt_unique(&EDatabase::GetPadstackDefIter))           
            .add_property("name", make_function(&EDatabase::GetName, return_value_policy<copy_const_reference>()))
            .add_property("suuid", &EDatabase::sUuid)
            .def("clear", &EDatabase::Clear)
        ;

        implicitly_convertible<std::shared_ptr<EDatabase>, std::shared_ptr<IDatabase> >();

        //DataMgr
        class_<EDataMgr,boost::noncopyable>("EDataMgr", no_init)
            .def("create_database", &EDataMgr::CreateDatabase)
            .def("open_database", &EDataMgr::OpenDatabase)
            .def("remove_database", &EDataMgr::RemoveDatabase)
            .def("shutdown", &EDataMgr::SaveDatabase)
            .def("shutdown", static_cast<void(EDataMgr::*)(bool)>(&EDataMgr::ShutDown), EDataMgrShutDownWithAutoSave())
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
            .def("save_database", &EDataMgr::SaveDatabase)
            .def("save_database", static_cast<bool(EDataMgr::*)(SPtr<IDatabase>, const std::string &, EArchiveFormat)>(&EDataMgr::SaveDatabase), EDataMgrSaveDatabaseBin())
            .def("load_database", &EDataMgr::LoadDatabase)
            .def("load_database", static_cast<bool(EDataMgr::*)(SPtr<IDatabase>, const std::string &, EArchiveFormat)>(&EDataMgr::LoadDatabase), EDataMgrLoadDatabaseBin())
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
            .def("create_circuit_cell", &EDataMgr::CreateCircuitCell, return_internal_reference<>())
            .def("find_cell_by_name", &EDataMgr::FindCellByName, return_internal_reference<>())
            .def("create_net", &EDataMgr::CreateNet, return_internal_reference<>())
            .def("create_stackup_layer", adapt_unique(&EDataMgr::CreateStackupLayer))
            .def("create_layer_map", &EDataMgr::CreateLayerMap, return_internal_reference<>())
            .def("create_padstack_def", &EDataMgr::CreatePadstackDef, return_internal_reference<>())
            .def("create_padstack_def_data", adapt_unique(&EDataMgr::CreatePadstackDefData))
            .def("create_padstack_inst", &EDataMgr::CreatePadstackInst, return_internal_reference<>())
            .def("create_cell_inst", &EDataMgr::CreateCellInst, return_internal_reference<>())
            .def("create_geometry_2d", &EDataMgrCreateGeometry2DWrap, return_internal_reference<>())
            .def("create_shape_polygon", adapt_unique(+[](EDataMgr & mgr, const boost::python::list & points){ return mgr.CreateShapePolygon(py_list_to_std_vector<EPoint2D>(points));}))
            .def("create_shape_polygon", adapt_unique(+[](EDataMgr & mgr, const EPolygonData & polygon){ return mgr.CreateShapePolygon(polygon);}))
            .def("create_shape_polygon_with_holes", adapt_unique(&EDataMgr::CreateShapePolygonWithHoles))
            .def("create_text", &EDataMgr::CreateText, return_internal_reference<>())
            .def("instance", &EDataMgr::Instance, return_value_policy<reference_existing_object>())
            .staticmethod("instance")
        ;
    }
}
#endif//ECAD_BOOST_PYTHON_SUPPORT