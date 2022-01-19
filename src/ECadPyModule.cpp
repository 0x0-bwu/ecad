#include "ECadCommon.h"

#ifdef ECAD_BOOST_PYTHON_SUPPORT
#include <boost/python/stl_iterator.hpp>
#include "generic/tools/Format.hpp"
#include "EPadstackDefData.h"
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
        auto get_iter = iterator<std::vector<T> >();
        auto iter = get_iter(vec);
        boost::python::list l(iter);
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
    Ptr<IPrimitive> EDataMgrCreateGeometry2DWrap(EDataMgr & mgr, Ptr<ILayoutView> layout, ELayerId layer, ENetId net, Ptr<EShape> shape)
    {
        //todo, enhance, copy issue here
        return mgr.CreateGeometry2D(layout, layer, net, shape->Clone());
    }
    
    BOOST_PYTHON_FUNCTION_OVERLOADS(makeETransform2DWithNoMirror, makeETransform2D, 3, 4)
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDataMgrSaveDatabaseTxt, EDataMgr::SaveDatabase, 2, 3)
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDataMgrLoadDatabaseTxt, EDataMgr::LoadDatabase, 2, 3)

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

        //ConnObj
        class_<EConnObj, bases<EObject> >("EConnObj", no_init)
            .add_property("net", &EConnObj::GetNet, &EConnObj::SetNet)
        ;

        //Padstack Def
        class_<IPadstackDef, boost::noncopyable>("IPadstackDef", no_init)
        ;

        class_<EPadstackDef, bases<EDefinition, IPadstackDef> >("EPadstackDef", init<std::string>())
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

        //Database
        class_<IDatabase, std::shared_ptr<IDatabase>, boost::noncopyable>("IDatabase", no_init)
        ;
        
        class_<EDatabase, bases<IDatabase>, std::shared_ptr<EDatabase> >("EDatabase", init<std::string>())
            .add_property("name", make_function(&EDatabase::GetName, return_value_policy<copy_const_reference>()), &EDatabase::SetName)
            .add_property("suuid", &EDatabase::sUuid)
        ;

        implicitly_convertible<std::shared_ptr<EDatabase>, std::shared_ptr<IDatabase> >();

        //DataMgr
        class_<EDataMgr,boost::noncopyable>("EDataMgr", no_init)
            .def("instance", &EDataMgr::Instance, return_value_policy<reference_existing_object>())
            .staticmethod("instance")
            .def("create_database", &EDataMgr::CreateDatabase)
            .def("open_database", &EDataMgr::OpenDatabase)
            .def("remove_database", &EDataMgr::RemoveDatabase)
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
            .def("save_database", &EDataMgr::SaveDatabase)
            .def("save_database", static_cast<bool(EDataMgr::*)(SPtr<IDatabase>, const std::string &, EArchiveFormat)>(&EDataMgr::SaveDatabase), EDataMgrSaveDatabaseTxt())
            .def("load_database", &EDataMgr::LoadDatabase)
            .def("load_database", static_cast<bool(EDataMgr::*)(SPtr<IDatabase>, const std::string &, EArchiveFormat)>(&EDataMgr::LoadDatabase), EDataMgrLoadDatabaseTxt())
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
        ;
    }
}
#endif//ECAD_BOOST_PYTHON_SUPPORT