#include "ECadCommon.h"

#ifdef ECAD_BOOST_PYTHON_SUPPORT
#include "ELayoutView.h"
#include "EDatabase.h"
#include "EDataMgr.h"
#include "ECell.h"
#include "ENet.h"
#include "EObject.h"

namespace ecad {
using namespace boost::python;
}//namespace ecad

namespace {
    using namespace ecad;
    using namespace boost::python;
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

        //Point
        class_<EPoint2D>("EPoint2D", init<ECoord, ECoord>())
        ;

        //Object
        class_<EObject>("EObject")
            .def("set_name", &EObject::SetName)
            .def("get_name", &EObject::GetName, return_value_policy<copy_const_reference>())
            .def("suuid", &EObject::sUuid)
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

        //LayoutView
        class_<ILayoutView, boost::noncopyable>("ILayoutView", no_init)
        ;

        class_<ELayoutView, bases<ILayoutView> >("ELayoutView", no_init)
        ;

        //Cell
        class_<ICell, boost::noncopyable>("ICell", no_init)
        ;

        class_<ECell, bases<EDefinition, ICell>, boost::noncopyable>("ECell", no_init)
            .def("get_name", &ECell::GetName, return_value_policy<copy_const_reference>())
            .def("get_database", &ECell::GetDatabase, return_internal_reference<>())
            .def("get_layout_view", &ECell::GetLayoutView,  return_internal_reference<>()) 
        ;

        class_<ECircuitCell, bases<ECell> >("ECircuitCell", no_init)
            .def("get_layout_view", &ECircuitCell::GetLayoutView,  return_internal_reference<>()) 
        ;

        // implicitly_convertible<std::shared_ptr<ECell>, std::shared_ptr<ICell> >();

        //Database
        class_<IDatabase, std::shared_ptr<IDatabase>, boost::noncopyable>("IDatabase", no_init)
        ;
        
        class_<EDatabase, bases<IDatabase>, std::shared_ptr<EDatabase> >("EDatabase", init<std::string>())
            .def("set_name", &EDatabase::SetName)
            .def("get_name", &EDatabase::GetName, return_value_policy<copy_const_reference>())
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
        ;
    }
}
#endif//ECAD_BOOST_PYTHON_SUPPORT