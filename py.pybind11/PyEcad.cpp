#include "PyEcadBasic.hpp"
#include "PyEcadDesign.hpp"
#include "PyEcadUtility.hpp"
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

        // material
        .def("create_material_def", [](Ptr<IDatabase> database, const std::string & name)
            { return EDataMgr::Instance().CreateMaterialDef(database, name); }, py::return_value_policy::reference)
        .def("create_simple_material_prop", [](EFloat value)
            { return EDataMgr::Instance().CreateSimpleMaterialProp(value); }) 


        // settings
        .def("hier_sep", []
            { return EDataMgr::Instance().HierSep(); })
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
}