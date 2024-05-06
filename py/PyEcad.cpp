#include "basic/ECadSettings.h"
#include "basic/EShape.h"
#include "collection/EHierarchyObjCollection.h"
#include "collection/EPadstackInstCollection.h"
#include "collection/EPadstackDefCollection.h"
#include "collection/EPrimitiveCollection.h"
#include "collection/ECellInstCollection.h"
#include "collection/ELayerMapCollection.h"
#include "collection/EConnObjCollection.h"
#include "collection/ELayerCollection.h"
#include "collection/ECellCollection.h"
#include "collection/ENetCollection.h"
#include "design/EPadstackDefData.h"
#include "design/EPadstackInst.h"
#include "design/EPadstackDef.h"
#include "design/ELayoutView.h"
#include "design/EPrimitive.h"
#include "design/ECellInst.h"
#include "design/EDatabase.h"
#include "design/ELayerMap.h"
#include "design/EConnObj.h"
#include "design/EObject.h"
#include "design/ELayer.h"
#include "design/ECell.h"
#include "design/ENet.h"
#include "EDataMgr.h"

#include "generic/common/Traits.hpp"
#include "generic/tools/Format.hpp"

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python.hpp>

namespace ecad {
    using namespace boost::python;
    using namespace generic::common;
  
    template <typename Container, typename T = typename Container::value_type,
              typename std::enable_if<std::is_same<typename Container::value_type, T>::value, bool>::type = true>
    Container py_list_to_std_container(const boost::python::list & bpl)
    {
        try{
            Container container;
            for(int i = 0; i < len(bpl); ++i){
                append_to_std_container(container, boost::python::extract<T>(bpl[i]), 0);
            }
            return container;
        }
        catch (...) {
            throw(std::invalid_argument("Could not convert python list to std container, illegal object type"));
        }
    }

    template <typename Container, typename T = typename Container::value_type,
              typename std::enable_if<std::is_same<typename Container::value_type, T>::value, bool>::type = true>
    boost::python::list std_container_to_py_list(const Container & con)
    {
        boost::python::list bpl;
        for(auto t : con) bpl.append(t);
        return bpl;
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
    template<typename Derived, typename Base, typename Deleter = std::default_delete<Base> >
    UPtr<Base, Deleter> ECloneWrap(const Derived & derived)
    {
        static_assert(std::is_base_of<Base, Derived>::value, "not a derived class from base!");
        return static_cast<const Base &>(derived).Clone();
    }

    Ptr<INet> ENetCollectionAddNetWrap(ENetCollection & collection, Ptr<INet> net)
    {
        //todo, enhance, copy issue here
        return collection.AddNet(net->Clone());
    }

    ELayerId ELayerCollectionAppendLayerWrap(ELayerCollection & collection, Ptr<ILayer> layer)
    {
        //todo, enhance, copy issue here
        return collection.AppendLayer(layer->Clone());
    }

    boost::python::list ELayerCollectionAppendLayersWrap(ELayerCollection & collection, const boost::python::list & layers)
    {
        //todo, enhance, copy issue here
        auto vec = py_list_to_std_container<std::vector<Ptr<ILayer> > >(layers);
        std::vector<UPtr<ILayer> > input;
        for(auto layer : vec)
            input.emplace_back(layer->Clone());
        auto res = collection.AppendLayers(std::move(input));
        return std_container_to_py_list(res);
    }

    std::vector<Ptr<IStackupLayer> > ELayerCollectionGetStackupLayersWrap(const ELayerCollection & collection)
    {
        std::vector<Ptr<IStackupLayer> > layers;
        collection.GetStackupLayers(layers);
        return layers;
    }

    void EPadstackDefSetPadstackDefDataWrap(EPadstackDef & def, Ptr<IPadstackDefData> data)
    {
        //todo, enhance, copy issue here
        return def.SetPadstackDefData(data->Clone());
    }

    void EViaSetViaShapeWrap(EVia & via, Ptr<EShape> shape)
    {
        //todo, enhance, copy issue here
        via.shape = shape->Clone();
    }

    void EPadSetPadShapeWrap(EPad & pad, Ptr<EShape> shape)
    {
        //todo, enhance, copy issue here
        pad.shape = shape->Clone();
    }

    bool EPadstackDefDataSetPadParametersWrapWithLayerId(EPadstackDefData & data, ELayerId layer, Ptr<EShape> shape, const EPoint2D & offset, EFloat rotation)
    {
        //todo, enhance, copy issue here
        return data.SetPadParameters(layer, shape->Clone(), offset, rotation);
    }

    bool EPadstackDefDataSetPadParametersWrapWithLayerName(EPadstackDefData & data, const std::string & layer, Ptr<EShape> shape, const EPoint2D & offset, EFloat rotation)
    {
        //todo, enhance, copy issue here
        return data.SetPadParameters(layer, shape->Clone(), offset, rotation);
    }

    object EPadstackDefDataGetPadParametersWrapWithLayerId(const EPadstackDefData & data, ELayerId layer)
    {
        EFloat rotation = 0;
        EPoint2D offset{0, 0};
        CPtr<EShape> shape{nullptr};
        if(!data.GetPadParameters(layer, shape, offset, rotation)) return boost::python::object();
        return boost::python::make_tuple(offset, rotation);
    }

    CPtr<EShape> EPadstackDefDataGetPadShapeWrapWithLayerId(const EPadstackDefData & data, ELayerId layer)
    {
        EFloat rotation = 0;
        EPoint2D offset{0, 0};
        CPtr<EShape> shape{nullptr};
        if(!data.GetPadParameters(layer, shape, offset, rotation)) return nullptr;
        return shape;
    }

    object EPadstackDefDataGetPadParametersWrapWithLayerName(const EPadstackDefData & data, const std::string & layer)
    {
        EFloat rotation = 0;
        EPoint2D offset{0, 0};
        CPtr<EShape> shape{nullptr};
        if(!data.GetPadParameters(layer, shape, offset, rotation)) return boost::python::object();
        return boost::python::make_tuple(offset, rotation);
    }

    CPtr<EShape> EPadstackDefDataGetPadShapeWrapWithLayerName(const EPadstackDefData & data, const std::string & layer)
    {
        EFloat rotation = 0;
        EPoint2D offset{0, 0};
        CPtr<EShape> shape{nullptr};
        if(!data.GetPadParameters(layer, shape, offset, rotation)) return nullptr;
        return shape;
    }

    void EPadstackDefDataSetViaParametersWrap(EPadstackDefData & data, Ptr<EShape> shape, const EPoint2D & offset, EFloat rotation)
    {
        //todo, enhance, copy issue here
        data.SetViaParameters(shape->Clone(), offset, rotation);
    }

    boost::python::tuple EPadstackDefDataGetViaParametersWrap(const EPadstackDefData & data)
    {
        EFloat rotation = 0;
        EPoint2D offset{0, 0};
        CPtr<EShape> shape{nullptr};
        data.GetViaParameters(shape, offset, rotation);
        return boost::python::make_tuple(offset, rotation);
    }

    CPtr<EShape> EPadstackDefDataGetViaShapeWrap(const EPadstackDefData & data)
    {
        CPtr<EShape> shape{nullptr};
        EPoint2D offset{0, 0};
        EFloat rotation = 0;
        data.GetViaParameters(shape, offset, rotation);
        return shape; 
    }

    Ptr<IPadstackInst> EPadstackInstCollectionAddPadstackInstWrap(EPadstackInstCollection & collection, Ptr<IPadstackInst> inst)
    {
        //todo, enhance, copy isuue here
        return collection.AddPadstackInst(inst->Clone());
    }

    Ptr<ICellInst> ECellInstCollectionAddCellInstWrap(ECellInstCollection & collection, Ptr<ICellInst> inst)
    {
        //todo, enhance, copy issue here
        return collection.AddCellInst(inst->Clone());
    }

    Ptr<IPrimitive> EPrimitiveCollectionAddPrimitiveWrap(EPrimitiveCollection & collection, Ptr<IPrimitive> primitive)
    {
        //todo, enhance, copy issue here
        return collection.AddPrimitive(primitive->Clone());
    }

    Ptr<IPrimitive> EPrimitiveCollectionCreatePrimitiveWrap(EPrimitiveCollection & collection, ELayerId layer, ENetId net, Ptr<EShape> shape)
    {
        //todo, enhance, copy issue here
        return collection.CreateGeometry2D(layer, net, shape->Clone());
    }
    
    void EGeometry2DSetShapeWrap(EGeometry2D & geom, Ptr<EShape> shape)
    {
        //todo, enhance, copy issue here
        return geom.SetShape(shape->Clone());
    }

    ELayerId ELayoutViewAppendLayerWrap(const ELayoutView & layout, Ptr<ILayer> layer)
    {
        //todo, enhance, copy issue here
        return layout.AppendLayer(layer->Clone());
    }

    boost::python::list ELayoutViewAppendLayersWrap(const ELayoutView & layout, const boost::python::list & layers)
    {
        //todo, enhance, copy issue here
        auto vec = py_list_to_std_container<std::vector<Ptr<ILayer> > >(layers);
        std::vector<UPtr<ILayer> > input;
        for(auto layer : vec)
            input.emplace_back(layer->Clone());
        auto res = layout.AppendLayers(std::move(input));
        return std_container_to_py_list(res);
    }

    std::vector<Ptr<IStackupLayer> > ELayoutViewGetStackupLayersWrap(const ELayoutView & layout)
    {
        std::vector<Ptr<IStackupLayer> > layers;
        layout.GetStackupLayers(layers);
        return layers;
    }

    Ptr<IPrimitive> ELayoutViewCreateGeometry2DWrap(ELayoutView & layout, ELayerId layer, ENetId net, Ptr<EShape> shape)
    {
        //todo, enhance, copy issue here
        return layout.CreateGeometry2D(layer, net, shape->Clone());
    }

    void ELayoutViewSetBoundaryWrap(ELayoutView & layout, Ptr<EPolygon> boundary)
    {
        //todo, enhance, copy issue here
        auto shape = boundary->Clone();
        auto polygon = UPtr<EPolygon>(dynamic_cast<Ptr<EPolygon> >(shape.get()));
        shape.release();
        return layout.SetBoundary(std::move(polygon));
    }

    bool ECellSetLayoutViewWrap(ECell & cell, Ptr<ILayoutView> layout)
    {
        //todo, enhance, copy issue here
        return cell.SetLayoutView(layout->Clone());
    }

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

    UPtr<ILayer> EDataMgrCreateStackupLayerWrap(EDataMgr & mgr, const std::string & name, ELayerType type, EFloat elevation, ECoord thickness)
    {
        return mgr.CreateStackupLayer(name, type, elevation, thickness);
    }

    Ptr<IPrimitive> EDataMgrCreateGeometry2DWrap(EDataMgr & mgr, Ptr<ILayoutView> layout, ELayerId layer, ENetId net, Ptr<EShape> shape)
    {
        //todo, enhance, copy issue here
        return mgr.CreateGeometry2D(layout, layer, net, shape->Clone());
    }

}//namespace ecad

namespace {
    using namespace ecad;
    using namespace boost::python;

    BOOST_PYTHON_FUNCTION_OVERLOADS(makeETransform2DWithNoMirror, makeETransform2D, 3, 4)
    
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDatabaseSaveBin, EDatabase::Save, 1, 2)
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDatabaseLoadBin, EDatabase::Load, 1, 2)

    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDataMgrSaveDatabaseBin, EDataMgr::SaveDatabase, 2, 3)
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDataMgrLoadDatabaseBin, EDataMgr::LoadDatabase, 1, 2)
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDataMgrShutDownWithAutoSave, EDataMgr::ShutDown, 0, 1)
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(EDataMgrCreateDatabaseFromGdsWithoutLayrMap, EDataMgr::CreateDatabaseFromGds, 2, 3)

    BOOST_PYTHON_MODULE(PyEcad)
    {           
        //Enum
        enum_<EArchiveFormat>("EArchiveFormat")
            .value("TXT", EArchiveFormat::TXT)
            .value("XML", EArchiveFormat::XML)
            .value("BIN", EArchiveFormat::BIN)
        ;

        enum_<ECellType>("ECellType")
            .value("INVALID", ECellType::Invalid)
            .value("CIRCUITCELL", ECellType::CircuitCell)
        ;

        enum_<ELayerId>("ELayerId")
            .value("NOLAYER", ELayerId::noLayer)
        ;

        enum_<ENetId>("ENetId")
            .value("NONET", ENetId::noNet)
        ;

        enum_<ELayerType>("ELayerType")
            .value("INVALID", ELayerType::Invalid)
            .value("DIELECTRICLAYER", ELayerType::DielectricLayer)
            .value("CONDUCTINGLAYER", ELayerType::ConductingLayer)
            .value("METALIZEDSIGNAL", ELayerType::MetalizedSignal)
        ;

        enum_<EPrimitiveType>("EPrimitiveType")
            .value("INVALID", EPrimitiveType::Invalid)
            .value("GEOMETRY2D", EPrimitiveType::Geometry2D)
            .value("TEXT", EPrimitiveType::Text)
        ;

        enum_<EDefinitionType>("EDefinitionType")
            .value("INVALID", EDefinitionType::Invalid)
            .value("COMPONENTDEF", EDefinitionType::ComponentDef)
            .value("PADSTACKDEF", EDefinitionType::PadstackDef)
            .value("MATERIALDEF", EDefinitionType::MaterialDef)
            .value("LAYERMAP", EDefinitionType::LayerMap)
            .value("CELL", EDefinitionType::Cell)
        ;

        enum_<EFlattenOption>("EFlattenOption")
        ;

        enum_<EMirror2D>("EMirror2D")
            .value("NO", EMirror2D::No)
            .value("X", EMirror2D::X)
            .value("Y", EMirror2D::Y)
        ;

        //Unit
        enum_<ECoordUnits::Unit>("ELengthUnit")
            .value("NM", ECoordUnits::Unit::Nanometer)
            .value("UM", ECoordUnits::Unit::Micrometer)
            .value("MM", ECoordUnits::Unit::Millimeter)
            .value("METER",  ECoordUnits::Unit::Meter)
        ;

        class_<ECoordUnits>("ECoordUnits", init<ECoordUnits::Unit, ECoordUnits::Unit>())
        ;

        //Point
        class_<EPoint2D>("EPoint2D", init<ECoord, ECoord>())
            .def("__str__", +[](const EPoint2D & p){ std::stringstream ss; ss << p; return ss.str(); })
            .def("__eq__", &EPoint2D::operator==)
            .def("__ne__", &EPoint2D::operator!=)
            .add_property("x",
                            +[](const EPoint2D & p){ return p[0]; },
                            make_function([](EPoint2D & p, ECoord x){ p[0] = x; }, default_call_policies(), boost::mpl::vector<void, EPoint2D &, ECoord>()))
            .add_property("y",
                            +[](const EPoint2D & p){ return p[1]; },
                            make_function([](EPoint2D & p, ECoord y){ p[1] = y; }, default_call_policies(), boost::mpl::vector<void, EPoint2D &, ECoord>()))
        ;

        //box
        class_<EBox2D>("EBox2D", init<EPoint2D, EPoint2D>())
            .def("__eq__", &EBox2D::operator==)
            .def("__ne__", &EBox2D::operator!=)
            .add_property("ll",
                            +[](const EBox2D & b){ return b[0]; },
                            make_function([](EBox2D & b, const EPoint2D & p){ b[0] = p; }, default_call_policies(), boost::mpl::vector<void, EBox2D &, const EPoint2D &>()))
            .add_property("ur",
                            +[](const EBox2D & b){ return b[1]; },
                            make_function([](EBox2D & b, const EPoint2D & p){ b[1] = p; }, default_call_policies(), boost::mpl::vector<void, EBox2D &, const EPoint2D &>()))
        ;

        //polygon
        class_<EPolygonData>("EPolygonData")
            .def("__str__", +[](const EPolygonData & polygon){ std::stringstream ss; ss << polygon; return ss.str(); })
            .def("size", &EPolygonData::Size)
            .def("set_points", make_function([](EPolygonData & polygon, const boost::python::list & points){
                                                polygon.Set(py_list_to_std_container<std::vector<EPoint2D> >(points));
                                             },
                                             default_call_policies(), boost::mpl::vector<void, EPolygonData &, const boost::python::list &>()))
        ;

        class_<EPolygonHolesData>("EPolygonHolesData")
            .def("size", &EPolygonHolesData::size)
            .def("add_hole", +[](EPolygonHolesData & holes, EPolygonData hole){
                                    if(hole.isCCW()) hole.Reverse();
                                    holes.emplace_back(std::move(hole));
                                })
            .def("clear", &EPolygonHolesData::clear)
            ;

        class_<EPolygonWithHolesData>("EPolygonWithHolesData")
            .def("__str__", +[](const EPolygonWithHolesData & pwh){ std::stringstream ss; ss << pwh; return ss.str(); })
            .def_readwrite("outline", &EPolygonWithHolesData::outline)
            .def_readwrite("holes", &EPolygonWithHolesData::holes)
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
            .def("get_bbox", &EShape::GetBBox)
            .def("get_contour", &EShape::GetContour)
            .def("get_polygon_with_holes", &EShape::GetPolygonWithHoles)
            .def("transform", &EShape::Transform)
            .def("get_shape_type", &EShape::GetShapeType)
        ;

        class_<ERectangle, bases<EShape> >("ERectangle")
        ;

        class_<EPath, bases<EShape> >("EPath")
            .def_readwrite("shape", &EPath::shape)
        ;

        class_<EPolygon, bases<EShape> >("EPolygon")
            .def_readwrite("shape", &EPolygon::shape)
            .def("set_points", make_function([](EPolygon & polygon, const boost::python::list & points){
                                        polygon.SetPoints(py_list_to_std_container<std::vector<EPoint2D> >(points));
                                    },
                                    default_call_policies(), boost::mpl::vector<void, EPolygon &, const boost::python::list &>()))
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
            .def("get_definition_type", &EDefinition::GetDefinitionType)
        ;

        //Net
        class_<INet, boost::noncopyable>("INet", no_init)
        ;

        class_<ENet, bases<EObject, INet> >("ENet", init<std::string>())
            .def("clone", adapt_unique(&ECloneWrap<ENet, INet>))
            .add_property("net_id", &ENet::GetNetId, &ENet::SetNetId)
        ;

        //Net Iterator
        class_<IIterator<INet>, boost::noncopyable>("INetIter", no_init)
        ;

        class_<ENetIterator, bases<IIterator<INet> >, boost::noncopyable>("ENetIter", no_init)
            .def("next", &ENetIterator::Next, return_internal_reference<>())
            .def("current", &ENetIterator::Current, return_internal_reference<>())
        ;

        //Net Collection
        class_<INetCollection, boost::noncopyable>("INetCollection", no_init)
        ;

        class_<ENetCollection, bases<INetCollection> >("ENetCollection")
            .def("__len__", &ENetCollection::Size)
            .def("__getitem__", +[](const ENetCollection & c, const std::string & name){ return c.At(name).get(); }, return_internal_reference<>())
            .def("__contains__", &ENetCollection::Count)
            .def("next_net_name", &ENetCollection::NextNetName)
            .def("find_net_by_name", &ENetCollection::FindNetByName, return_internal_reference<>())
            .def("create_net", &ENetCollection::CreateNet, return_internal_reference<>())
            .def("add_net", &ENetCollectionAddNetWrap, return_internal_reference<>())
            .def("get_net_iter", adapt_unique(&ENetCollection::GetNetIter))
            .def("size", &ENetCollection::Size)
            .def("clear", &ENetCollection::Clear)
        ;

        //Layer
        class_<ILayer, boost::noncopyable>("ILayer", no_init)
        ;

        class_<IStackupLayer, boost::noncopyable>("IStackupLayer", no_init)
        ;

        class_<IViaLayer, boost::noncopyable>("IViaLayer", no_init)
        ;

        class_<ELayer, bases<EObject, ILayer>, boost::noncopyable>("ELayer", no_init)
            .def("clone", adapt_unique(&ECloneWrap<ELayer, ILayer>))
            .add_property("layer_id", &ELayer::GetLayerId, &ELayer::SetLayerId)
            .def("get_layer_type", &ELayer::GetLayerType)
            .def("get_stackup_layer_from_layer", &ELayer::GetStackupLayerFromLayer, return_internal_reference<>())
            .def("get_via_layer_from_layer", &ELayer::GetViaLayerFromLayer, return_internal_reference<>())
        ;

        class_<EStackupLayer, bases<ELayer, IStackupLayer> >("EStackupLayer", init<std::string, ELayerType>())
            .add_property("elevation", &EStackupLayer::GetElevation, &EStackupLayer::SetElevation)
            .add_property("thickness", &EStackupLayer::GetThickness, &EStackupLayer::GetThickness)
        ;

        class_<EViaLayer, bases<ELayer, IViaLayer> >("EViaLayer", init<std::string>())
        ;

        class_<std::vector<Ptr<ILayer> > >("ELayerContainer")
            .def(vector_indexing_suite<std::vector<Ptr<ILayer> > >())
        ;

        class_<std::vector<Ptr<IStackupLayer> > >("EStackupLayerContainer")
            .def(vector_indexing_suite<std::vector<Ptr<IStackupLayer> > >())
        ;

        //Layer Iterator
        class_<IIterator<ILayer>, boost::noncopyable>("ILayerIter", no_init)
        ;

        class_<ELayerIterator, bases<IIterator<ILayer> >, boost::noncopyable>("ELayerIter", no_init)
            .def("next", &ENetIterator::Next, return_internal_reference<>())
            .def("current", &ENetIterator::Current, return_internal_reference<>())
        ;

        //Layer Collection
        class_<ILayerCollection, boost::noncopyable>("ILayerCollection", no_init)
        ;

        class_<ELayerCollection, bases<ILayerCollection> >("ELayerCollection")
            .def("__len__", &ELayerCollection::Size)
            .def("append_layer", &ELayerCollectionAppendLayerWrap)
            .def("append_layers", &ELayerCollectionAppendLayersWrap)
            .def("add_default_dielectric_layers", adapt_unique(&ELayerCollection::AddDefaultDielectricLayers))
            .def("get_default_layer_map", adapt_unique(&ELayerCollection::GetDefaultLayerMap))
            .def("get_stackup_layers", &ELayerCollectionGetStackupLayersWrap)
            .def("get_layer_iter", adapt_unique(&ELayerCollection::GetLayerIter))
            .def("size", &ELayerCollection::Size)
            .def("find_layer_by_name", &ELayerCollection::FindLayerByName, return_internal_reference<>())
            .def("get_next_layer_name", &ELayerCollection::GetNextLayerName)
        ;

        //Layer Map Iterator
        class_<IIterator<ILayer>, boost::noncopyable>("ILayerIter", no_init)
        ;

        class_<ELayerIterator, bases<IIterator<ILayer> >, boost::noncopyable>("ELayerIter", no_init)
            .def("next", &ELayerIterator::Next, return_internal_reference<>())
            .def("current", &ELayerIterator::Current, return_internal_reference<>())
        ;

        //Layer Map
        class_<ILayerMap, boost::noncopyable>("ILayerMap", no_init)
        ;

        class_<ELayerMap, bases<EDefinition, ILayerMap> >("ELayerMap", init<std::string, CPtr<IDatabase> >())
            .def("clone", adapt_unique(&ECloneWrap<ELayerMap, ILayerMap>))
            .def("get_database", &ELayerMap::GetDatabase, return_internal_reference<>())
            .def("set_mapping", &ELayerMap::SetMapping)
            .def("mapping_left", &ELayerMap::MappingLeft)
            .def("mapping_right", &ELayerMap::MappingRight)
            .def("get_mapping_forward", &ELayerMap::GetMappingForward)
            .def("get_mapping_backward", &ELayerMap::GetMappingBackward)
        ;

        //Layer Map Collection
        class_<ILayerMapCollection, boost::noncopyable>("ILayerMapCollection", no_init)
        ;

        class_<ELayerMapCollection, bases<ILayerMapCollection> >("ELayerMapCollection")
            .def("__len__", &ELayerMapCollection::Size)
            .def("get_layer_map_iter", adapt_unique(&ELayerMapCollection::GetLayerMapIter))
            .def("size", &ELayerMapCollection::Size)
            .def("clear", &ELayerMapCollection::Clear)
        ;

        //Layer Map Iterator
        class_<IIterator<ILayerMap>, boost::noncopyable>("ILayerMapIter", no_init)
        ;

        class_<ELayerMapIterator, bases<IIterator<ILayerMap> >, boost::noncopyable>("ELayerMapIter", no_init)
            .def("next", &ELayerMapIterator::Next, return_internal_reference<>())
            .def("current", &ELayerMapIterator::Current, return_internal_reference<>())
        ;
        
        //ConnObj
        class_<IConnObj, boost::noncopyable>("IConnObj", no_init)
        ;

        class_<EConnObj, bases<EObject, IConnObj> >("EConnObj", no_init)
            .add_property("net", &EConnObj::GetNet, &EConnObj::SetNet)
        ;

        //ConnObj Iterator
        class_<IIterator<IConnObj>, boost::noncopyable>("IConnObjIter", no_init)
        ;

        class_<EConnObjIterator, bases<IIterator<IConnObj> >, boost::noncopyable>("EConnObjIter", no_init)
            .def("next", &EConnObjIterator::Next, return_internal_reference<>())
            .def("current", &EConnObjIterator::Current, return_internal_reference<>())
        ;

        //ConnObj Collection
        class_<IConnObjCollection, boost::noncopyable>("IConnObjCollection", no_init)
        ;

        class_<EConnObjCollection, bases<IConnObjCollection> >("EConnObjCollection")
            .def("__len__", &EConnObjCollection::Size)
            .def("get_primitive_collection", &EConnObjCollection::GetPrimitiveCollection, return_internal_reference<>())
            .def("get_padstack_inst_collection", &EConnObjCollection::GetPadstackInstCollection, return_internal_reference<>())
            .def("get_conn_obj_iter", adapt_unique(&EConnObjCollection::GetConnObjIter))
            .def("size", &EConnObjCollection::Size)
        ;

        //Padstack Def
        class_<IPadstackDef, boost::noncopyable>("IPadstackDef", no_init)
        ;

        class_<EPadstackDef, bases<EDefinition, IPadstackDef> >("EPadstackDef", init<std::string, CPtr<IDatabase> >())
            .def("clone", adapt_unique(&ECloneWrap<EPadstackDef, IPadstackDef>))
            .def("set_padstack_def_data", &EPadstackDefSetPadstackDefDataWrap)
            .def("get_padstack_def_data", &EPadstackDef::GetPadstackDefData, return_internal_reference<>())
        ;

        //Padstack Def Collection
        class_<IPadstackDefCollection, boost::noncopyable>("IPadstackDefCollection", no_init)
        ;

        class_<EPadstackDefCollection, bases<IPadstackDefCollection> >("EPadstackDefCollection")
            .def("__len__", &EPadstackDefCollection::Size)
            .def("get_padstack_def_iter", adapt_unique(&EPadstackDefCollection::GetPadstackDefIter))
            .def("size", &EPadstackDefCollection::Size)
            .def("clear", &EPadstackDefCollection::Clear)
        ;

        //Padstack Def Iterator
        class_<IIterator<IPadstackDef>, boost::noncopyable>("IPadstackDefIter", no_init)
        ;

        class_<EPadstackDefIterator, bases<IIterator<IPadstackDef> >, boost::noncopyable>("EPadstackDefIter", no_init)
            .def("next", &EPadstackDefIterator::Next, return_internal_reference<>())
            .def("current", &EPadstackDefIterator::Current, return_internal_reference<>())
        ;

        //Via
        class_<EVia>("EVia")
            .def_readwrite("rotation", &EVia::rotation)
            .def_readwrite("offset", &EVia::offset)
            .add_property("shape",
                           make_function([](const EVia & via){ return via.shape.get(); },
                           return_value_policy<reference_existing_object>(), boost::mpl::vector<CPtr<EShape>, const EVia &>()),
                           make_function(&EViaSetViaShapeWrap,
                           default_call_policies(), boost::mpl::vector<void, EVia &, Ptr<EShape> >()))
        ;

        //Pad
        class_<EPad>("EPad")
            .def_readwrite("layer", &EPad::lyr)
            .def_readwrite("rotation", &EPad::rotation)
            .def_readwrite("offset", &EPad::offset)
            .add_property("shape",
                           make_function([](const EPad & pad){ return pad.shape.get(); },
                           return_value_policy<reference_existing_object>(), boost::mpl::vector<CPtr<EShape>, const EPad &>()),
                           make_function(&EPadSetPadShapeWrap,
                           default_call_policies(), boost::mpl::vector<void, EPad &, Ptr<EShape> >()))
        ;

        //Padstack Def Data
        class_<IPadstackDefData, boost::noncopyable>("IPadstackDefData", no_init)
        ;

        class_<EPadstackDefData, bases<IPadstackDefData> >("EPadstackDefData")
            .def("clone", adapt_unique(&ECloneWrap<EPadstackDefData, IPadstackDefData>))
            .add_property("material", &EPadstackDefData::GetMaterial, &EPadstackDefData::SetMaterial)
            .def("set_layers", +[](EPadstackDefData & data, const boost::python::list & layers){ return data.SetLayers(py_list_to_std_container<std::vector<std::string> >(layers)); })
            .def("set_pad_parameters", &EPadstackDefDataSetPadParametersWrapWithLayerId)
            .def("get_pad_parameters", &EPadstackDefDataGetPadParametersWrapWithLayerId)
            .def("get_pad_shape", &EPadstackDefDataGetPadShapeWrapWithLayerId, return_internal_reference<>())
            .def("set_pad_parameters", &EPadstackDefDataSetPadParametersWrapWithLayerName)
            .def("get_pad_parameters", &EPadstackDefDataGetPadParametersWrapWithLayerName)
            .def("get_pad_shape", &EPadstackDefDataGetPadShapeWrapWithLayerName, return_internal_reference<>())
            .def("set_via_parameters", &EPadstackDefDataSetViaParametersWrap)
            .def("get_via_parameters", &EPadstackDefDataGetViaParametersWrap)
            .def("get_via_shape", &EPadstackDefDataGetViaShapeWrap, return_internal_reference<>())
        ;

        //Padstack Inst
        class_<IPadstackInst, boost::noncopyable>("IPadstackInst", no_init)
        ;

        class_<EPadstackInst, bases<EConnObj, IPadstackInst> >("EPadstackInst", init<std::string, CPtr<IPadstackDef>, ENetId>())
            .def("clone", adapt_unique(&ECloneWrap<EPadstackInst, IPadstackInst>))
            .add_property("net", &EPadstackInst::GetNet, &EPadstackInst::SetNet)
            .def("set_layer_range", &EPadstackInst::SetLayerRange)
            .def("get_layer_range", +[](const EPadstackInst & inst){
                                        ELayerId top, bot;
                                        inst.GetLayerRange(top, bot);
                                        return boost::python::make_tuple(top, bot);
                                    })
            .add_property("layer_map",
                            make_function(&EPadstackInst::GetLayerMap, return_value_policy<reference_existing_object>()),
                            &EPadstackInst::SetLayerMap)
            .def("get_padstack_def", &EPadstackInst::GetPadstackDef, return_internal_reference<>())
            .add_property("is_layout_pin", &EPadstackInst::isLayoutPin, &EPadstackInst::SetIsLayoutPin)
            .def("get_layer_shape", adapt_unique(&EPadstackInst::GetLayerShape))
        ;

        //Padstack Inst Iterator
        class_<IIterator<IPadstackInst>, boost::noncopyable>("IPadstackInstIter", no_init)
        ;

        class_<EPadstackInstIterator, bases<IIterator<IPadstackInst> >, boost::noncopyable>("EPadstackInstIter", no_init)
            .def("next", &EPadstackInstIterator::Next, return_internal_reference<>())
            .def("current", &EPadstackInstIterator::Current, return_internal_reference<>())
        ;

        //Primitive Collection
        class_<IPadstackInstCollection, boost::noncopyable>("IPadstackInstCollection", no_init)
        ;

        class_<EPadstackInstCollection, bases<IPadstackInstCollection> >("EPadstackInstCollection")
            .def("__len__", &EPadstackInstCollection::Size)
            .def("add_padstack_inst", &EPadstackInstCollectionAddPadstackInstWrap, return_internal_reference<>())
            .def("create_padstack_inst", &EPadstackInstCollection::CreatePadstackInst, return_internal_reference<>())
            .def("map", &EPadstackInstCollection::Map)
            .def("get_padstack_inst_iter", adapt_unique(&EPadstackInstCollection::GetPadstackInstIter))
            .def("size", &EPadstackInstCollection::Size)
        ;

        //HierarchyObj
        class_<IHierarchyObj, boost::noncopyable>("IHierarchyObj", no_init)
        ;

        class_<EHierarchyObj, bases<EObject, IHierarchyObj> >("EHierarchyObj", no_init)
        ;

        //HierarchyObj Iterator
        class_<IIterator<IHierarchyObj>, boost::noncopyable>("IHierarchyObjIter", no_init)
        ;

        class_<EHierarchyObjIterator, bases<IIterator<IHierarchyObj> >, boost::noncopyable>("EHierarchyObjIter", no_init)
            .def("next", &EHierarchyObjIterator::Next, return_internal_reference<>())
            .def("current", &EHierarchyObjIterator::Current, return_internal_reference<>())
        ;

        //HierarchyObj Collection
        class_<IHierarchyObjCollection, boost::noncopyable>("IHierarchyObjCollection", no_init)
        ;

        class_<EHierarchyObjCollection, bases<IHierarchyObjCollection> >("EHierarchyObjCollection")
            .def("__len__", &EHierarchyObjCollection::Size)
            .def("get_cell_inst_collection", &EHierarchyObjCollection::GetCellInstCollection, return_internal_reference<>())
            .def("get_hierarchy_obj_iter", adapt_unique(&EHierarchyObjCollection::GetHierarchyObjIter))
            .def("size", &EHierarchyObjCollection::Size)
        ;

        //CellInst
        class_<ICellInst, boost::noncopyable>("ICellInst", no_init)
        ;

        class_<ECellInst, bases<EHierarchyObj, ICellInst> >("ECellInst", no_init)
            .def("clone", adapt_unique(&ECloneWrap<ECellInst, ICellInst>))
            .add_property("ref_layout",
                            make_function(&ECellInst::GetRefLayoutView, return_value_policy<reference_existing_object>()),
                            &ECellInst::SetRefLayoutView)
            .add_property("def_layout",
                            make_function(&ECellInst::GetDefLayoutView, return_value_policy<reference_existing_object>()),
                            &ECellInst::SetDefLayoutView)
            .def("get_flattened_layout_view", &ECellInst::GetFlattenedLayoutView, return_internal_reference<>())
        ;

        //CellInst Iterator
        class_<IIterator<ICellInst>, boost::noncopyable>("ICellInstIter", no_init)
        ;

        class_<ECellInstIterator, bases<IIterator<ICellInst> >, boost::noncopyable>("ECellInstIter", no_init)
            .def("next", &ECellInstIterator::Next, return_internal_reference<>())
            .def("current", &ECellInstIterator::Current, return_internal_reference<>())
        ;

        //CellInst Collection
        class_<ICellInstCollection, boost::noncopyable>("ICellInstCollection", no_init)
        ;

        class_<ECellInstCollection, bases<ICellInstCollection> >("ECellInstCollection")
            .def("__len__", &ECellInstCollection::Size)
            .def("add_cell_inst", &ECellInstCollectionAddCellInstWrap, return_internal_reference<>())
            .def("create_cell_inst", &ECellInstCollection::CreateCellInst, return_internal_reference<>())
            .def("get_cell_inst_iter", adapt_unique(&ECellInstCollection::GetCellInstIter))
            .def("size", &ECellInstCollection::Size)
            .def("clear", &ECellInstCollection::Clear)
        ;

        //Primitive
        class_<IPrimitive, boost::noncopyable>("IPrimitive", no_init)
            .def("clone", adapt_unique(&ECloneWrap<EPrimitive, IPrimitive>))
        ;

        class_<EPrimitive, bases<EConnObj, IPrimitive>, boost::noncopyable>("EPrimitive", no_init)
            .def("get_text_from_primitive", &EPrimitive::GetTextFromPrimitive, return_internal_reference<>())
            .def("get_conn_obj_from_primitive", &EPrimitive::GetConnObjFromPrimitive, return_internal_reference<>())
            .def("get_geometry_2d_from_primitive", &EPrimitive::GetGeometry2DFromPrimitive, return_internal_reference<>())
            .add_property("layer", &EPrimitive::GetLayer, &EPrimitive::SetLayer)
            .def("get_primitive_type", &EPrimitive::GetPrimitiveType)
        ;

        //Primitive Iterator
        class_<IIterator<IPrimitive>, boost::noncopyable>("IPrimitiveIter", no_init)
        ;

        class_<EPrimitiveIterator, bases<IIterator<IPrimitive> >, boost::noncopyable>("EPrimitiveIter", no_init)
            .def("next", &EPrimitiveIterator::Next, return_internal_reference<>())
            .def("current", &EPrimitiveIterator::Current, return_internal_reference<>())
        ;

        //Primitive Collection
        class_<IPrimitiveCollection, boost::noncopyable>("IPrimitiveCollection", no_init)
        ;

        class_<EPrimitiveCollection, bases<IPrimitiveCollection> >("EPrimitiveCollection")
            .def("__len__", &EPrimitiveCollection::Size)
            .def("add_primitive", &EPrimitiveCollectionAddPrimitiveWrap, return_internal_reference<>())
            .def("create_geometry_2d", &EPrimitiveCollectionCreatePrimitiveWrap, return_internal_reference<>())
            .def("create_text", &EPrimitiveCollection::CreateText, return_internal_reference<>())
            .def("map", &EPrimitiveCollection::Map)
            .def("get_primitive_iter", adapt_unique(&EPrimitiveCollection::GetPrimitiveIter))
            .def("size", &EPrimitiveCollection::Size)
            .def("clear", &EPrimitiveCollection::Clear)
        ;

        //Geometry2D
        class_<IGeometry2D, boost::noncopyable>("IGeometry2D", no_init)
        ;

        class_<EGeometry2D, bases<EPrimitive, IGeometry2D> >("EGeometry2D", init<ELayerId, ENetId>())
            .def("set_shape", &EGeometry2DSetShapeWrap)
            .def("get_shape", &EGeometry2D::GetShape, return_internal_reference<>())
            .def("transform", &EGeometry2D::Transform)
        ;

        class_<IText, boost::noncopyable>("IText", no_init)
        ;

        class_<EText, bases<EPrimitive, IText> >("EText", no_init)
            .def(init<std::string>())
            .def(init<std::string, ELayerId, ENetId>())
            .add_property("text", make_function(&EText::GetText, return_value_policy<copy_const_reference>()))
            .def("get_position", &EText::GetPosition)
        ;

        //LayoutView
        class_<ILayoutView, boost::noncopyable>("ILayoutView", no_init)
        ;

        class_<ELayoutView, bases<ILayoutView> >("ELayoutView", init<std::string, Ptr<ICell> >())
            .add_property("name", make_function(&ELayoutView::GetName, return_value_policy<copy_const_reference>()))
            .add_property("suuid", &ELayoutView::sUuid)
            .def("get_net_iter", adapt_unique(&ELayoutView::GetNetIter))
            .def("get_layer_iter", adapt_unique(&ELayoutView::GetLayerIter))
            .def("get_conn_obj_iter", adapt_unique(&ELayoutView::GetConnObjIter))
            .def("get_cell_inst_iter", adapt_unique(&ELayoutView::GetCellInstIter))
            .def("get_primitive_iter", adapt_unique(&ELayoutView::GetPrimitiveIter))
            .def("get_hierarchy_obj_iter", adapt_unique(&ELayoutView::GetHierarchyObjIter))
            .def("get_padstack_inst_iter", adapt_unique(&ELayoutView::GetPadstackInstIter))
            .def("get_cell", &ELayoutView::GetCell, return_internal_reference<>())
            .def("append_layer", &ELayoutViewAppendLayerWrap)
            .def("append_layers", &ELayoutViewAppendLayersWrap)
            .def("get_stackup_layers", &ELayoutViewGetStackupLayersWrap)
            .def("add_default_dielectric_layers", adapt_unique(&ELayoutView::AddDefaultDielectricLayers))
            .def("create_net", &ELayoutView::CreateNet, return_internal_reference<>())
            .def("create_cell_inst", &ELayoutView::CreateCellInst, return_internal_reference<>())
            .def("create_padstack_inst", &ELayoutView::CreatePadstackInst, return_internal_reference<>())
            .def("create_geometry_2d", &ELayoutViewCreateGeometry2DWrap, return_internal_reference<>())
            .def("create_text", &ELayoutView::CreateText, return_internal_reference<>())
            .def("get_net_collection", &ELayoutView::GetNetCollection, return_internal_reference<>())
            .def("get_layer_collection", &ELayoutView::GetLayerCollection, return_internal_reference<>())
            .def("get_conn_obj_collection", &ELayoutView::GetConnObjCollection, return_internal_reference<>())
            .def("get_cell_inst_collection", &ELayoutView::GetCellInstCollection, return_internal_reference<>())
            .def("get_primitive_collection", &ELayoutView::GetPrimitiveCollection, return_internal_reference<>())
            .def("get_hierarchy_obj_collection", &ELayoutView::GetHierarchyObjCollection, return_internal_reference<>())
            .def("get_padstack_inst_collection", &ELayoutView::GetPadstackInstCollection, return_internal_reference<>())
            .def("set_boundary", &ELayoutViewSetBoundaryWrap)
            .def("get_boundary", &ELayoutView::GetBoundary, return_internal_reference<>())//todo, should be immutable
            .def("generate_metal_fraction_mapping", &ELayoutView::GenerateMetalFractionMapping)
            .def("connectivity_extraction", &ELayoutView::ExtractConnectivity)
            .def("merge_layer_polygons", &ELayoutView::MergeLayerPolygons)
            .def("flatten", &ELayoutView::Flatten)
            .def("map", &ELayoutView::Map)
        ;

        //Cell
        class_<ICell, boost::noncopyable>("ICell", no_init)
        ;

        class_<ECell, bases<EDefinition, ICell>, boost::noncopyable>("ECell", no_init)
            .def("set_layout_view", &ECellSetLayoutViewWrap)
            .def("get_cell_type", &ECell::GetCellType)
            .def("get_database", &ECell::GetDatabase, return_internal_reference<>())
            .def("get_layout_view", &ECell::GetLayoutView,  return_internal_reference<>())
            .def("get_flattened_layout_view", &ECell::GetFlattenedLayoutView, return_internal_reference<>())
        ;

        class_<ECircuitCell, bases<ECell> >("ECircuitCell", init<std::string, CPtr<IDatabase> >())
        ;

        class_<std::vector<Ptr<ICell> > >("ECellContainer")
            .def("__iter__", boost::python::iterator<std::vector<Ptr<ICell> > >())
            .def(vector_indexing_suite<std::vector<Ptr<ICell> > >())
        ;

        //Cell Collection
        class_<ICellCollection, boost::noncopyable>("ICellCollection", no_init)
        ;

        class_<ECellCollection, bases<ICellCollection> >("ECellCollection")
            .def("__len__", &ECellCollection::Size)
            .def("__getitem__", +[](const ECellCollection & c, const std::string & name){ return c.At(name).get(); }, return_internal_reference<>())
            .def("__contains__", &ECellCollection::Count)
            .def("get_cell_iter", adapt_unique(&ECellCollection::GetCellIter))
            .def("size", &ECellCollection::Size)
            .def("clear", &ECellCollection::Clear)
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
            .def("clone", adapt_unique(&ECloneWrap<EDatabase, IDatabase>))
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
            .def("create_database", &EDataMgr::CreateDatabase, return_internal_reference<>())
            .def("open_database", &EDataMgr::OpenDatabase, return_internal_reference<>())
            .def("remove_database", &EDataMgr::RemoveDatabase)
            .def("shutdown", &EDataMgr::SaveDatabase)
            .def("shutdown", static_cast<void(EDataMgr::*)(bool)>(&EDataMgr::ShutDown), EDataMgrShutDownWithAutoSave())
            .def("create_database_from_gds", &EDataMgr::CreateDatabaseFromGds, return_internal_reference<>())
            // .def("create_database_from_gds", static_cast<Ptr<IDatabase>(EDataMgr::*)(const std::string &, const std::string &, const std::string &)>(&EDataMgr::CreateDatabaseFromGds), EDataMgrCreateDatabaseFromGdsWithoutLayrMap())
            .def("create_database_from_xfl", &EDataMgr::CreateDatabaseFromXfl, return_internal_reference<>())
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
            .def("save_database", &EDataMgr::SaveDatabase)
            .def("save_database", static_cast<bool(EDataMgr::*)(CPtr<IDatabase>, const std::string &, EArchiveFormat)>(&EDataMgr::SaveDatabase), EDataMgrSaveDatabaseBin())
            .def("load_database", &EDataMgr::LoadDatabase, return_internal_reference<>())
            .def("load_database", static_cast<Ptr<IDatabase>(EDataMgr::*)(const std::string &, EArchiveFormat)>(&EDataMgr::LoadDatabase), EDataMgrLoadDatabaseBin()[return_internal_reference<>()])
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
            .def("create_circuit_cell", &EDataMgr::CreateCircuitCell, return_internal_reference<>())
            .def("find_cell_by_name", &EDataMgr::FindCellByName, return_internal_reference<>())
            .def("create_net", &EDataMgr::CreateNet, return_internal_reference<>())
            .def("create_stackup_layer", adapt_unique(&EDataMgr::CreateStackupLayer))
            .def("create_stackup_layer_with_default_materials", adapt_unique(&EDataMgrCreateStackupLayerWrap))
            .def("create_layer_map", &EDataMgr::CreateLayerMap, return_internal_reference<>())
            .def("create_padstack_def", &EDataMgr::CreatePadstackDef, return_internal_reference<>())
            .def("create_padstack_def_data", adapt_unique(&EDataMgr::CreatePadstackDefData))
            .def("create_padstack_inst", &EDataMgr::CreatePadstackInst, return_internal_reference<>())
            .def("create_cell_inst", &EDataMgr::CreateCellInst, return_internal_reference<>())
            .def("create_geometry_2d", &EDataMgrCreateGeometry2DWrap, return_internal_reference<>())
            .def("create_shape_polygon", adapt_unique(+[](EDataMgr & mgr, const boost::python::list & points){ return mgr.CreateShapePolygon(py_list_to_std_container<std::vector<EPoint2D> >(points));}))
            .def("create_shape_polygon", adapt_unique(+[](EDataMgr & mgr, const EPolygonData & polygon){ return mgr.CreateShapePolygon(polygon);}))
            .def("create_shape_polygon_with_holes", adapt_unique(&EDataMgr::CreateShapePolygonWithHoles))
            .def("create_text", &EDataMgr::CreateText, return_internal_reference<>())
            .def("instance", &EDataMgr::Instance, return_value_policy<reference_existing_object>())
            .staticmethod("instance")
        ;

        //ECadSettings
        class_<EMetalFractionMappingSettings>("EMetalFractionMappingSettings")
            .def_readwrite("threads", &EMetalFractionMappingSettings::threads)
            .def_readwrite("out_file", &EMetalFractionMappingSettings::outFile)
            .def_readwrite("region_ext_top", &EMetalFractionMappingSettings::regionExtTop)
            .def_readwrite("region_ext_bot", &EMetalFractionMappingSettings::regionExtBot)
            .def_readwrite("region_ext_left", &EMetalFractionMappingSettings::regionExtLeft)
            .def_readwrite("region_ext_right", &EMetalFractionMappingSettings::regionExtRight)
            .add_property("grid_x",
                            +[](const EMetalFractionMappingSettings & settings){ return settings.grid[0]; },
                            make_function([](EMetalFractionMappingSettings & settings, size_t x){ settings.grid[0] = x; },
                            default_call_policies(), boost::mpl::vector<void, EMetalFractionMappingSettings &, size_t>()))
            .add_property("grid_y",
                            +[](const EMetalFractionMappingSettings & settings){ return settings.grid[1]; },
                            make_function([](EMetalFractionMappingSettings & settings, size_t y){ settings.grid[1] = y; },
                            default_call_policies(), boost::mpl::vector<void, EMetalFractionMappingSettings &, size_t>()))
            .add_property("select_nets", +[](const EMetalFractionMappingSettings & settings){ return std_container_to_py_list(settings.selectNets); },
                            make_function([](EMetalFractionMappingSettings & settings, const boost::python::list & l){ settings.selectNets = py_list_to_std_container<std::unordered_set<ENetId> >(l); },
                            default_call_policies(), boost::mpl::vector<void, EMetalFractionMappingSettings &, const boost::python::list &>()))
            ;
        
        class_<ELayoutPolygonMergeSettings>("ELayoutPolygonMergeSettings")
            .def_readwrite("threads", &ELayoutPolygonMergeSettings::threads)
            .def_readwrite("out_file", &ELayoutPolygonMergeSettings::outFile)
            .def_readwrite("mt_by_layer", &ELayoutPolygonMergeSettings::mtByLayer)
            .def_readwrite("include_padstack_inst", &ELayoutPolygonMergeSettings::includePadstackInst)
            .def_readwrite("include_dielectric_layer", &ELayoutPolygonMergeSettings::includeDielectricLayer)
            .def_readwrite("skip_top_bot_dielectric_layers", &ELayoutPolygonMergeSettings::skipTopBotDielectricLayers)
            .add_property("select_nets", +[](const ELayoutPolygonMergeSettings & settings){ return std_container_to_py_list(settings.selectNets); },
                            make_function([](ELayoutPolygonMergeSettings & settings, const boost::python::list & l){ settings.selectNets = py_list_to_std_container<std::unordered_set<ENetId> >(l); },
                            default_call_policies(), boost::mpl::vector<void, ELayoutPolygonMergeSettings &, const boost::python::list &>()))
            ;
    }
}