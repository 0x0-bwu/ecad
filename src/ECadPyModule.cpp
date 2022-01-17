#include "ECadCommon.h"

#ifdef ECAD_BOOST_PYTHON_SUPPORT
#include "EDataMgr.h"
#include "EDatabase.h"
#include "EObject.h"

namespace ecad {
using namespace boost::python;
}//namespace ecad

namespace {
    using namespace ecad;
    using namespace boost::python;
    BOOST_PYTHON_MODULE(ECAD_LIB_NAME)
    {
        //Point
        class_<EPoint2D>("EPoint2D", init<ECoord, ECoord>())
        ;

        //Object
        class_<EObject>("EObject")
            .def("set_name", &EObject::SetName)
            .def("get_name", &EObject::GetName, return_value_policy<copy_const_reference>())
            .def("suuid", &EObject::sUuid)
        ;

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
        ;
    }
}
#endif//ECAD_BOOST_PYTHON_SUPPORT