#pragma once
#include <pybind11/pybind11.h>
#include "EDataMgr.h"

using namespace ecad;
namespace py = pybind11;

void ecad_init_basic(py::module_ & m)
{
    m.doc() = R"pbdoc(
        ECAD LIBRARY beta
    )pbdoc";

    m.attr("__version__") = toString(CURRENT_VERSION);

    py::enum_<ELogLevel>(m, "ELogLevel")
        .value("TRACE", ELogLevel::Trace)
        .value("DEBUG", ELogLevel::Debug)
        .value("INFO" , ELogLevel::Info)
        .value("WARN" , ELogLevel::Warn)
        .value("ERROR", ELogLevel::Error)
        .value("FATAL", ELogLevel::Fatal)
        .value("OFF"  , ELogLevel::Off)
    ;

    py::enum_<EArchiveFormat>(m, "EArchiveFormat")
        .value("TXT", EArchiveFormat::TXT)
        .value("XML", EArchiveFormat::XML)
        .value("BIN", EArchiveFormat::BIN)
    ;
}
