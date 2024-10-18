#pragma once
#include "PyEcadCommon.hpp"

namespace ecad::wrapper {

    class PyShape : public EShape
    {
    public:
        using EShape::EShape;
        EPolygonData GetContour() const
        {
            PYBIND11_OVERRIDE_PURE(EPolygonData, EShape, GetContour);
        }
    };

    class PyECadSettings : public ECadSettings
    {
    public:
        using ECadSettings::ECadSettings;
    };

    class PyThermalModelExtractionSettings : public EThermalModelExtractionSettings
    {
    public:
        using EThermalModelExtractionSettings::BCType;
        using EThermalModelExtractionSettings::EThermalModelExtractionSettings;

        void AddBlockBC(EOrientation orient, FBox2D block, BCType type, EFloat value) override
        {
            PYBIND11_OVERRIDE(void, EThermalModelExtractionSettings, AddBlockBC, orient, block, type, value);
        }
    };

    template <typename Coord>
    std::vector<generic::geometry::Point2D<Coord>> Coord2Point2D(const std::vector<Coord> & coords)
    {
        std::vector<generic::geometry::Point2D<Coord>> points(coords.size() / 2);
        for (size_t i = 0; i < points.size(); ++i)
            points[i] = generic::geometry::Point2D<Coord>(coords[i * 2], coords[i * 2 + 1]);
        return points;
    }

} // namespace ecad::wrapper

void ecad_init_basic(py::module_ & m)
{
    using namespace ecad::wrapper;

    m.doc() = R"pbdoc(
        ECAD LIBRARY beta
    )pbdoc";

    m.attr("__version__") = toString(CURRENT_VERSION);

    m.def("coord_to_point2d", &Coord2Point2D<ECoord>);
    m.def("coord_to_fpoint2d", &Coord2Point2D<FCoord>);

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

    py::enum_<EFlattenOption>(m, "FlattenOption")
        .value("NO", EFlattenOption::NO)
    ;

    py::enum_<ECoordUnits::Unit>(m, "CoordUnit")
        .value("NANOMETER", ECoordUnits::Unit::Nanometer)
        .value("MICROMETER", ECoordUnits::Unit::Micrometer)
        .value("MILLIMETER", ECoordUnits::Unit::Millimeter)
        .value("METER", ECoordUnits::Unit::Meter)
        .value("INCH", ECoordUnits::Unit::Inch)
    ;

    py::class_<ECoordUnits>(m, "CoordUnits")
        .def(py::init<>())
        .def(py::init<ECoordUnits::Unit>())
        .def(py::init<ECoordUnits::Unit, ECoordUnits::Unit>())
        .def("to_coord", py::overload_cast<const FPoint2D &>(&ECoordUnits::toCoord, py::const_))
        .def("to_unit", py::overload_cast<const EPoint2D &>(&ECoordUnits::toUnit<ECoord>, py::const_))
        .def("to_unit", py::overload_cast<const FPoint2D &>(&ECoordUnits::toUnit<FCoord>, py::const_))
    ;

    py::enum_<EOrientation>(m, "Orientation")
        .value("TOP", EOrientation::Top)
        .value("BOT", EOrientation::Bot)
        // .value("LEFT", EOrientation::Left)
        // .value("RIGHT", EOrientation::Right)
        // .value("FRONT", EOrientation::Front)
        // .value("BACK", EOrientation::Back)
    ;

    py::enum_<ENetId>(m, "NetId")
        .value("NO_NET", ENetId::noNet)
    ;

    py::enum_<ELayerType>(m, "LayerType")
        .value("INVALID", ELayerType::Invalid)
        .value("DIELECTRIC_LAYER", ELayerType::DielectricLayer)
        .value("CONDUCTING_LAYER", ELayerType::ConductingLayer)
        .value("METALIZED_SIGNAL", ELayerType::MetalizedSignal)
    ;

    py::enum_<ELayerId>(m, "LayerId")
        .value("NO_LAYER", ELayerId::noLayer)
    ;

    py::enum_<EBondwireType>(m, "BondwireType")
        .value("INVALID", EBondwireType::Invalid)
        .value("SIMPLE", EBondwireType::Simple)
        .value("JEDEC4", EBondwireType::JEDEC4)
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

    py::enum_<EPinIOType>(m, "PinIOType")
        .value("INVALID", EPinIOType::Invalid)
        .value("DRIVER", EPinIOType::Driver)
        .value("RECEIVER", EPinIOType::Receiver)
        .value("BI_DIRECTIONAL", EPinIOType::BiDirectional)
        .value("DRIVER_TERMINATOR", EPinIOType::DriverTerminator)
        .value("RECEIVER_TERMINATOR", EPinIOType::ReceiverTerminator)
    ;

    py::enum_<EComponentType>(m, "ComponentType")
        .value("INVALID", EComponentType::Invalid)
        .value("OTHER", EComponentType::Other)
        .value("RESISTOR", EComponentType::Resistor)
        .value("INDUCTOR", EComponentType::Inductor)
        .value("CAPACITOR", EComponentType::Capacitor)
        .value("IC", EComponentType::IC)
        .value("IO", EComponentType::IO)
        .value("MOLDING", EComponentType::Molding)
    ;

    py::enum_<EThermalBondaryConditionType>(m, "ThermalBondaryConditionType")
        .value("HTC", EThermalBondaryConditionType::HTC)
        .value("HEAT_FLUX", EThermalBondaryConditionType::HeatFlux)
    ;

    py::enum_<EThermalNetworkStaticSolverType>(m, "ThermalNetworkStaticSolverType")
        .value("SPARSE_LU", EThermalNetworkStaticSolverType::SparseLU)
        .value("SIMPLICIAL_CHOLESKY", EThermalNetworkStaticSolverType::Cholesky)
        .value("LLT", EThermalNetworkStaticSolverType::LLT)
        .value("LDLT", EThermalNetworkStaticSolverType::LDLT)
        .value("CONJUGATE_GRADIENT", EThermalNetworkStaticSolverType::ConjugateGradient)
    ;

    py::class_<EPoint2D>(m, "Point2D")
        .def(py::init<>())
        .def(py::init<ECoord, ECoord>())
        .def_property("x", [](const EPoint2D & p){ return p[0]; }, [](EPoint2D & p, ECoord x){ return p[0] = x; })
        .def_property("y", [](const EPoint2D & p){ return p[1]; }, [](EPoint2D & p, ECoord y){ return p[1] = y; })
    ;

    py::class_<FPoint2D>(m, "FPoint2D")
        .def(py::init<>())
        .def(py::init<FCoord, FCoord>())
        .def_property("x", [](const FPoint2D & p){ return p[0]; }, [](FPoint2D & p, FCoord x){ return p[0] = x; })
        .def_property("y", [](const FPoint2D & p){ return p[1]; }, [](FPoint2D & p, FCoord y){ return p[1] = y; })
    ;

    m.attr("FVector2D") = m.attr("FPoint2D");

    py::class_<EPoint3D>(m, "Point3D")
        .def(py::init<>())
        .def(py::init<ECoord, ECoord, ECoord>())
        .def_property("x", [](const EPoint3D & p){ return p[0]; }, [](EPoint3D & p, ECoord x){ return p[0] = x; })
        .def_property("y", [](const EPoint3D & p){ return p[1]; }, [](EPoint3D & p, ECoord y){ return p[1] = y; })
        .def_property("z", [](const EPoint3D & p){ return p[2]; }, [](EPoint3D & p, ECoord z){ return p[2] = z; })
    ;

    py::class_<FPoint3D>(m, "FPoint3D")
        .def(py::init<>())
        .def(py::init<FCoord, FCoord, FCoord>())
        .def_property("x", [](const FPoint3D & p){ return p[0]; }, [](FPoint3D & p, FCoord x){ return p[0] = x; })
        .def_property("y", [](const FPoint3D & p){ return p[1]; }, [](FPoint3D & p, FCoord y){ return p[1] = y; })
        .def_property("z", [](const FPoint3D & p){ return p[2]; }, [](FPoint3D & p, FCoord z){ return p[2] = z; })
    ;

    py::class_<EBox2D>(m, "Box2D")
        .def(py::init<>())
        .def(py::init<ECoord, ECoord, ECoord, ECoord>())
        .def(py::init<EPoint2D, EPoint2D>())
        .def_property("ll", [](const EBox2D & box){ return box[0]; }, [](EBox2D & box, EPoint2D & ll){ box[0] = ll; })
        .def_property("ur", [](const EBox2D & box){ return box[1]; }, [](EBox2D & box, EPoint2D & ur){ box[1] = ur; })
        .def("center", &EBox2D::Center)
        .def("scale", &EBox2D::Scale)
        .def_static("collision", py::overload_cast<const EBox2D &, const EBox2D &, bool>(&EBox2D::Collision))
    ;

    py::class_<FBox2D>(m, "FBox2D")
        .def(py::init<>())
        .def(py::init<FCoord, FCoord, FCoord, FCoord>())
        .def(py::init<FPoint2D, FPoint2D>())
        .def_property("ll", [](const FBox2D & box){ return box[0]; }, [](FBox2D & box, FPoint2D & ll){ box[0] = ll; })
        .def_property("ur", [](const FBox2D & box){ return box[1]; }, [](FBox2D & box, FPoint2D & ur){ box[1] = ur; })
    ;

    py::class_<EBox3D>(m, "Box3D")
        .def(py::init<>())
        .def(py::init<ECoord, ECoord, ECoord, ECoord, ECoord, ECoord>())
        .def(py::init<EPoint3D, EPoint3D>())
        .def_property("ll", [](const EBox3D & box){ return box[0]; }, [](EBox3D & box, EPoint3D & ll){ box[0] = ll; })
        .def_property("ur", [](const EBox3D & box){ return box[1]; }, [](EBox3D & box, EPoint3D & ur){ box[1] = ur; })
    ;

    py::class_<FBox3D>(m, "FBox3D")
        .def(py::init<>())
        .def(py::init<FCoord, FCoord, FCoord, FCoord, FCoord, FCoord>())
        .def(py::init<FPoint3D, FPoint3D>())
        .def_property("ll", [](const FBox3D & box){ return box[0]; }, [](FBox3D & box, FPoint3D & ll){ box[0] = ll; })
        .def_property("ur", [](const FBox3D & box){ return box[1]; }, [](FBox3D & box, FPoint3D & ur){ box[1] = ur; })
    ;

    py::class_<EPolygonData>(m, "PolygonData")

    ;

    py::class_<EPolygonWithHolesData>(m, "PolygonWithHolesData")
        .def(py::init<>())
        .def_readwrite("outline", &EPolygonWithHolesData::outline)
        .def_readwrite("holes", &EPolygonWithHolesData::holes)
    ;

    py::enum_<EMirror2D>(m, "Mirror2D")
        .value("NO", EMirror2D::No)
        .value("X" , EMirror2D::X)
        .value("Y" , EMirror2D::Y)
        .value("XY", EMirror2D::XY)
    ;

    py::class_<ETransform2D>(m, "Transform2D")
    ;

    py::class_<EShape, PyShape>(m, "Shape")
        .def("get_contour", &EShape::GetContour)
    ;

    py::enum_<ETemperatureUnit>(m, "TemperatureUnit")
        .value("CELSIUS", ETemperatureUnit::Celsius)
        .value("KELVINS", ETemperatureUnit::Kelvins)
    ;

    py::class_<ETemperature>(m, "Temperature")
        .def(py::init<EFloat, ETemperatureUnit>())
        .def_readwrite("value", &ETemperature::value)
        .def_readwrite("unit", &ETemperature::unit)
        .def("in_celsius", &ETemperature::inCelsius)
        .def("in_kelvins", &ETemperature::inKelvins)
        .def_static("kelvins2celsius", &ETemperature::Kelvins2Celsius)
        .def_static("celsius2Kelvins", &ETemperature::Celsius2Kelvins)
    ;

    py::class_<ECadSettings, PyECadSettings>(m, "ECadSettings")
    ;

    py::class_<EMeshSettings, ECadSettings>(m, "MeshSettings")
        .def(py::init<>())
        .def_readwrite("min_alpha", &EMeshSettings::minAlpha)
        .def_readwrite("min_len", &EMeshSettings::minLen)
        .def_readwrite("max_len", &EMeshSettings::maxLen)
        .def_readwrite("tolerance", &EMeshSettings::tolerance)
    ;

    py::class_<ELayoutPolygonMergeSettings>(m, "PolygonMergeSettings")
        .def(py::init<size_t, const ENetIdSet &>())
        .def_readwrite("threads", &ELayoutPolygonMergeSettings::threads)
        .def_readwrite("out_file", &ELayoutPolygonMergeSettings::outFile)
        .def_readwrite("mt_by_layer", &ELayoutPolygonMergeSettings::mtByLayer)
        .def_readwrite("include_padstack_inst", &ELayoutPolygonMergeSettings::includePadstackInst)
        .def_readwrite("include_dielectric_layer", &ELayoutPolygonMergeSettings::includeDielectricLayer)
        .def_readwrite("skip_top_bot_dielectric_layers", &ELayoutPolygonMergeSettings::skipTopBotDielectricLayers)
        .def_readwrite("select_nets", &ELayoutPolygonMergeSettings::selectNets)
    ;

    py::class_<ELayerCutModelExtractionSettings>(m, "LayerCutModelExtractionSettgins")
        .def_readwrite("dump_sketch_img", &ELayerCutModelExtractionSettings::dumpSketchImg)
        .def_readwrite("layer_cut_precision", &ELayerCutModelExtractionSettings::layerCutPrecision)
        .def_readwrite("layer_transition_ratio", &ELayerCutModelExtractionSettings::layerTransitionRatio)
        .def_readwrite("add_circle_center_as_steiner_point", &ELayerCutModelExtractionSettings::addCircleCenterAsSteinerPoint)
        .def_readwrite("imprint_box", &ELayerCutModelExtractionSettings::imprintBox)
    ;

    py::class_<EPrismMeshSettings, EMeshSettings>(m, "PrismMeshSettings")
        .def(py::init<>())
        .def_readwrite("iteration", &EPrismMeshSettings::iteration)
        .def_readwrite("dump_mesh_file", &EPrismMeshSettings::dumpMeshFile)
        .def_readwrite("gen_mesh_by_layer", &EPrismMeshSettings::genMeshByLayer)
        .def_readwrite("imprint_upper_layer", &EPrismMeshSettings::imprintUpperLayer)
    ;

    py::class_<EThermalBondaryCondition>(m, "ThermalBondaryCondition")
        .def_readwrite("type", &EThermalBondaryCondition::type)
        .def_readwrite("value", &EThermalBondaryCondition::value)
        .def("is_valid", &EThermalBondaryCondition::isValid)
    ;

    py::class_<EThermalModelExtractionSettings, PyThermalModelExtractionSettings>(m, "ThermalModelExtractionSettings")
        .def_readwrite("force_rebuild", &EThermalModelExtractionSettings::forceRebuild)
        .def_readwrite("threads", &EThermalModelExtractionSettings::threads)
        .def_readwrite("work_dir", &EThermalModelExtractionSettings::workDir)
        .def_readwrite("top_uniform_bc", &EThermalModelExtractionSettings::topUniformBC)
        .def_readwrite("bot_uniform_bc", &EThermalModelExtractionSettings::botUniformBC)
        .def_readwrite("top_block_bc", &EThermalModelExtractionSettings::topBlockBC)
        .def_readwrite("bot_block_bc", &EThermalModelExtractionSettings::topBlockBC)
        .def_readwrite("env_temperature", &EThermalModelExtractionSettings::envTemperature)
        .def("add_block_bc", &EThermalModelExtractionSettings::AddBlockBC)
    ;

    py::class_<EPrismThermalModelExtractionSettings, EThermalModelExtractionSettings>(m, "PrismThermalModelExtractionSettings")
        .def(py::init<std::string, size_t, const ENetIdSet &>())
        .def_readwrite("mesh_settings", &EPrismThermalModelExtractionSettings::meshSettings)
        .def_readwrite("polygon_merge_settings", &EPrismThermalModelExtractionSettings::polygonMergeSettings)
        .def_readwrite("layer_cut_settings", &EPrismThermalModelExtractionSettings::layerCutSettings)
        .def("add_block_bc", &EPrismThermalModelExtractionSettings::AddBlockBC)
    ;

    py::class_<EThermalSettings>(m, "ThermalSettings")
        .def(py::init<size_t>())
        .def_readwrite("dump_results", &EThermalSettings::dumpResults)
        .def_readwrite("threads", &EThermalSettings::threads)
        .def_readwrite("env_temperature", &EThermalSettings::envTemperature)
    ;

    py::class_<EThermalStaticSettings, EThermalSettings>(m, "ThermalStaticSettings")
        .def(py::init<size_t>())
        .def_readwrite("maximum_res", &EThermalStaticSettings::maximumRes)
        .def_readwrite("dump_hotmaps", &EThermalStaticSettings::dumpHotmaps)
        .def_readwrite("residual", &EThermalStaticSettings::residual)
        .def_readwrite("iteration", &EThermalStaticSettings::iteration)
        .def_readwrite("solver_type", &EThermalStaticSettings::solverType)
    ;

    py::class_<EThermalSimulationSetup>(m, "ThermalSimulationSetup")
        .def_readwrite("work_dir", &EThermalSimulationSetup::workDir)
        .def_readwrite("monitors", &EThermalSimulationSetup::monitors)
        .def("set_extraction_settings", [](EThermalSimulationSetup & setup, Ptr<EThermalModelExtractionSettings> settings){
            setup.extractionSettings = settings->Clone();
        })
        .def("get_grid_thermal_model_extraction_settings", &EThermalSimulationSetup::GetGridThermalModelExtractionSettings, py::return_value_policy::reference)
        .def("get_prism_thermal_model_extraction_settings", &EThermalSimulationSetup::GetPrismThermalModelExtractionSettings, py::return_value_policy::reference)
    ;

    py::class_<EThermalStaticSimulationSetup, EThermalSimulationSetup>(m, "ThermalStaticSimulationSetup")
        .def(py::init<std::string, size_t, ENetIdSet>())
        .def_readwrite("settings", &EThermalStaticSimulationSetup::settings)
    ;
}
