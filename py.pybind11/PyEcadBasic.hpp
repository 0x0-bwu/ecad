#pragma once
#include "PyEcadCommon.hpp"

namespace ecad::wrapper {

    ECoordUnits CreateCoordUnits(ECoordUnits::Unit user = ECoordUnits::Unit::Micrometer, 
                                 ECoordUnits::Unit data = ECoordUnits::Unit::Nanometer)
    {
        return ECoordUnits(user, data);
    }
} // namespace ecad::wrapper
void ecad_init_basic(py::module_ & m)
{
    m.doc() = R"pbdoc(
        ECAD LIBRARY beta
    )pbdoc";

    m.attr("__version__") = toString(CURRENT_VERSION);

    py::enum_<ELogLevel>(m, "LogLevel")
        .value("TRACE", ELogLevel::Trace)
        .value("DEBUG", ELogLevel::Debug)
        .value("INFO" , ELogLevel::Info)
        .value("WARN" , ELogLevel::Warn)
        .value("ERROR", ELogLevel::Error)
        .value("FATAL", ELogLevel::Fatal)
        .value("OFF"  , ELogLevel::Off)
    ;

    py::enum_<EArchiveFormat>(m, "ArchiveFormat")
        .value("TXT", EArchiveFormat::TXT)
        .value("XML", EArchiveFormat::XML)
        .value("BIN", EArchiveFormat::BIN)
    ;

    py::enum_<ECoordUnits::Unit>(m, "CoordUnit")
        .value("NANOMETER", ECoordUnits::Unit::Nanometer)
        .value("MICROMETER", ECoordUnits::Unit::Micrometer)
        .value("MILLIMETER", ECoordUnits::Unit::Millimeter)
        .value("METER", ECoordUnits::Unit::Meter)
        .value("INCH", ECoordUnits::Unit::Inch)
    ;

    py::class_<ECoordUnits>(m, "CoordUnits")
        .def(py::init(&ecad::wrapper::CreateCoordUnits))
    ;

    py::enum_<EMaterialType>(m, "MaterialType")
        .value("RIGID", EMaterialType::Rigid)
        .value("FLUID", EMaterialType::Fluid)
    ;

    py::enum_<EMaterialPropId>(m, "MaterialPropId")
        .value("PERMITTIVITY", EMaterialPropId::Permittivity)
        .value("PERMEABILITY", EMaterialPropId::Permeability)
        .value("CONDUCTIVITY", EMaterialPropId::Conductivity)
        .value("DIELECTRIC_LOSS_TANGENT", EMaterialPropId::DielectricLossTangent)
        .value("MAGNETIC_LOSS_TANGENT", EMaterialPropId::MagneticLossTangent)
        .value("RESISTIVITY", EMaterialPropId::Resistivity)
        .value("THERMAL_CONDUCTIVITY", EMaterialPropId::ThermalConductivity)
        .value("MASS_DENSITY", EMaterialPropId::MassDensity)
        .value("SPECIFIC_HEAT", EMaterialPropId::SpecificHeat)
        .value("YOUNGS_MODULUS", EMaterialPropId::YoungsModulus)
        .value("POISSONS_RATIO", EMaterialPropId::PoissonsRatio)
        .value("THERMAL_EXPANSION_COEFFICIENT", EMaterialPropId::ThermalExpansionCoefficient)
    ;

    py::enum_<ETemperatureUnit>(m, "TemperatureUnit")
        .value("CELSIUS", ETemperatureUnit::Celsius)
        .value("KELVINS", ETemperatureUnit::Kelvins)
    ;

    py::enum_<EThermalBondaryConditionType>(m, "ThermalBondaryConditionType")
        .value("HTC", EThermalBondaryConditionType::HTC)
        .value("HEAT_FLUX", EThermalBondaryConditionType::HeatFlux)
    ;

    py::enum_<EThermalNetworkStaticSolverType>(m, "ThermalNetworkStaticSolverType")
        .value("SPARSE_LU", EThermalNetworkStaticSolverType::SparseLU)
        .value("SIMPLICIAL_CHOLESKY", EThermalNetworkStaticSolverType::SimplicialCholesky)
        .value("CONJUGATE_GRADIENT", EThermalNetworkStaticSolverType::ConjugateGradient)
    ;

    py::class_<FBox2D>(m, "FBox2D")
    ;

    py::class_<ETemperature>(m, "Temperature")
        .def_readwrite("value", &ETemperature::value)
        .def_readwrite("unit", &ETemperature::unit)
        .def("in_celsius", &ETemperature::inCelsius)
        .def("in_kelvins", &ETemperature::inKelvins)
        .def_static("kelvins2celsius", &ETemperature::Kelvins2Celsius)
        .def_static("celsius2Kelvins", &ETemperature::Celsius2Kelvins)
    ;

    py::class_<EThermalStaticSettings>(m, "ThermalStaticSettings")
        .def(py::init<size_t>())
        .def_readwrite("maximum_res", &EThermalStaticSettings::maximumRes)
        .def_readwrite("dump_hotmaps", &EThermalStaticSettings::dumpHotmaps)
        .def_readwrite("residual", &EThermalStaticSettings::residual)
        .def_readwrite("iteration", &EThermalStaticSettings::iteration)
        .def_readwrite("solver_type", &EThermalStaticSettings::solverType)
    ;

    py::class_<EThermalStaticSimulationSetup>(m, "ThermalStaticSimulationSetup")
        .def(py::init<std::string, size_t, ENetIdSet>())
        .def_readwrite("settings", &EThermalStaticSimulationSetup::settings)
    ;

    py::class_<EThermalBondaryCondition>(m, "ThermalBondaryCondition")
        .def_readwrite("type", &EThermalBondaryCondition::type)
        .def_readwrite("value", &EThermalBondaryCondition::value)
        .def("is_valid", &EThermalBondaryCondition::isValid)
    ;
}
