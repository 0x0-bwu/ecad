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
        class_<EDataMgr,boost::noncopyable>("EDataMgr", no_init)
            .def("instance", &EDataMgr::Instance, return_value_policy<reference_existing_object>())
            .staticmethod("instance")
            // .def("create_database", &EDataMgr::CreateDatabase)
        ;

        class_<EObject>("EObject")
            .def("set_name", &EObject::SetName)
            .def("get_name", &EObject::GetName, return_value_policy<copy_const_reference>())
            .def("suuid", &EObject::sUuid)
        ;
    }
}
#endif//ECAD_BOOST_PYTHON_SUPPORT