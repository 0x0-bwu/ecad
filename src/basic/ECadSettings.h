#pragma once
#include "ECadCommon.h"
#include <unordered_set>
#include <functional>
#include <array>
#include <set>
namespace ecad {

struct ECadSettings
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION

    virtual bool operator== (const ECadSettings & settings) const = 0;
    bool operator != (const ECadSettings & settings) const
    {
        return not (*this == settings);
    }

    virtual ~ECadSettings() = default;
protected:
    ECadSettings() = default;
};

struct EDataMgrSettings
{
    char hierSep = '/';
    size_t threads = 1;
    size_t circleDiv = 16;
};

struct ELayoutPolygonMergeSettings : public ECadSettings
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECadSettings);
        ar & boost::serialization::make_nvp("threads", threads);
        ar & boost::serialization::make_nvp("out_file", outFile);
        ar & boost::serialization::make_nvp("mt_by_layer", mtByLayer);
        ar & boost::serialization::make_nvp("include_padstack_inst", includePadstackInst);
        ar & boost::serialization::make_nvp("include_dielectric_layer", includeDielectricLayer);
        ar & boost::serialization::make_nvp("skip_top_bot_dielectric_layers", skipTopBotDielectricLayers);
        ar & boost::serialization::make_nvp("select_nets", selectNets);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    size_t threads = 1;
    std::string outFile;
    bool mtByLayer{true};
    bool includePadstackInst = true;
    bool includeDielectricLayer = true;
    bool skipTopBotDielectricLayers = false;
    std::unordered_set<ENetId> selectNets;

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<ELayoutPolygonMergeSettings>>(&settings);
        if (nullptr == ps) return false;
        if (includePadstackInst != ps->includePadstackInst ||
            includeDielectricLayer != ps->includeDielectricLayer ||
            skipTopBotDielectricLayers != ps->skipTopBotDielectricLayers ||
            selectNets != ps->selectNets) return false;
        return true;
    }
};

struct EMetalFractionMappingSettings : public ECadSettings
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECadSettings);
        ar & boost::serialization::make_nvp("threads", threads);
        ar & boost::serialization::make_nvp("out_file", outFile);
        ar & boost::serialization::make_nvp("region_ext_top", regionExtTop);
        ar & boost::serialization::make_nvp("region_ext_bot", regionExtBot);
        ar & boost::serialization::make_nvp("region_ext_left", regionExtLeft);
        ar & boost::serialization::make_nvp("region_ext_right", regionExtRight);
        ar & boost::serialization::make_nvp("merge_geom_before_mapping", mergeGeomBeforeMapping);
        ar & boost::serialization::make_nvp("grid", grid);
        ar & boost::serialization::make_nvp("select_nets", selectNets);
        ar & boost::serialization::make_nvp("polygon_merge_settings", polygonMergeSettings);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    size_t threads = 1;
    std::string outFile;
    EFloat regionExtTop = 0;
    EFloat regionExtBot = 0;
    EFloat regionExtLeft = 0;
    EFloat regionExtRight = 0;
    bool mergeGeomBeforeMapping = true;
    std::array<size_t, 2> grid = {1, 1};
    std::unordered_set<ENetId> selectNets;
    ELayoutPolygonMergeSettings polygonMergeSettings;

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<EMetalFractionMappingSettings>>(&settings);
        if (nullptr == ps) return false;
        if (math::NE(regionExtTop, ps->regionExtTop) ||
            math::NE(regionExtBot, ps->regionExtBot) ||
            math::NE(regionExtLeft, ps->regionExtLeft) ||
            math::NE(regionExtRight, ps->regionExtRight) ||
            mergeGeomBeforeMapping != ps->mergeGeomBeforeMapping ||
            grid != ps->grid ||
            selectNets != selectNets ||
            polygonMergeSettings != polygonMergeSettings) return false;
        return true;
    }
};

struct EMeshSettings : public ECadSettings
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECadSettings);
        ar & boost::serialization::make_nvp("min_alpha", minAlpha);
        ar & boost::serialization::make_nvp("min_len", minLen);
        ar & boost::serialization::make_nvp("max_len", maxLen);
        ar & boost::serialization::make_nvp("tolerance", tolerance);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    virtual ~EMeshSettings() = default;
    EFloat minAlpha{20};//Deg
    EFloat minLen{0};
    EFloat maxLen{maxFloat};
    EFloat tolerance{0};

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<EMeshSettings>>(&settings);
        if (nullptr == ps) return false;
        if (math::NE(minAlpha, ps->minAlpha) ||
            math::NE(minLen, ps->minLen) ||
            math::NE(maxLen, ps->maxLen) ||
            math::NE(tolerance, ps->tolerance)) return false;
        return true;
    }
};

struct EPrismMeshSettings : public EMeshSettings
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EMeshSettings);
        ar & boost::serialization::make_nvp("iteration", iteration);
        ar & boost::serialization::make_nvp("dump_mesh_file", dumpMeshFile);
        ar & boost::serialization::make_nvp("gen_mesh_by_layer", genMeshByLayer);
        ar & boost::serialization::make_nvp("imprint_upper_layer", imprintUpperLayer);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    virtual ~EPrismMeshSettings() = default;
    size_t iteration = 0;
    bool dumpMeshFile = false;
    bool genMeshByLayer = false;
    bool imprintUpperLayer = false;

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<EPrismMeshSettings>>(&settings);
        if (nullptr == ps) return false;
    
        if (not EMeshSettings::operator==(settings)) return false;
        if (iteration != ps->iteration ||
            genMeshByLayer != ps->genMeshByLayer ||
            imprintUpperLayer != ps->imprintUpperLayer) return false;
        return true;
    }
};

struct ELayerCutModelExtractionSettings : public ECadSettings
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECadSettings);
        ar & boost::serialization::make_nvp("dump_sketch_img", dumpSketchImg);
        ar & boost::serialization::make_nvp("layer_cut_precision", layerCutPrecision);
        ar & boost::serialization::make_nvp("layer_transition_ratio", layerTransitionRatio);
        ar & boost::serialization::make_nvp("add_circle_center_as_steiner_points", addCircleCenterAsSteinerPoints);
        ar & boost::serialization::make_nvp("imprint_box", imprintBox);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    bool dumpSketchImg = true;
    size_t layerCutPrecision = 6;
    EFloat layerTransitionRatio = 2;
    bool addCircleCenterAsSteinerPoints{false};
    std::vector<FBox2D> imprintBox;

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<ELayerCutModelExtractionSettings>>(&settings);
        if (nullptr == ps) return false;
        if (layerCutPrecision != ps->layerCutPrecision ||
            layerTransitionRatio != ps->layerTransitionRatio ||
            addCircleCenterAsSteinerPoints != ps->addCircleCenterAsSteinerPoints ||
            imprintBox != ps->imprintBox) return false;
        return true;
    }
};

using ELayerCutModelBuildSettings = ELayerCutModelExtractionSettings;

struct EThermalBondaryCondition
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("type", type);
        ar & boost::serialization::make_nvp("value", value);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    using BCType = EThermalBondaryConditionType;
    BCType type{BCType::HTC};
    EFloat value{invalidFloat};
    EThermalBondaryCondition() = default;
    EThermalBondaryCondition(EFloat value, BCType type) : type(type), value(value) {}
    bool isValid() const { return ecad::isValid(value); }

    bool operator== (const EThermalBondaryCondition & settings) const
    {
        return type == settings.type && math::EQ(value, settings.value);
    }

    bool operator != (const EThermalBondaryCondition & settings) const
    {
        return not (*this == settings);
    }
};

struct EThermalModelExtractionSettings : public ECadSettings
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECadSettings);
        ar & boost::serialization::make_nvp("threads", threads);
        ar & boost::serialization::make_nvp("work_dir", workDir);
        ar & boost::serialization::make_nvp("top_uniform_BC", topUniformBC);
        ar & boost::serialization::make_nvp("bot_uniform_BC", botUniformBC);
        ar & boost::serialization::make_nvp("top_block_BC", topBlockBC);
        ar & boost::serialization::make_nvp("bot_block_BC", botBlokcBC);
        ar & boost::serialization::make_nvp("env_temperature", envTemperature);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    using BCType = EThermalBondaryConditionType;
    using BlockBoundaryCondition = std::pair<FBox2D, EThermalBondaryCondition>;
    virtual ~EThermalModelExtractionSettings() = default;
    size_t threads;
    std::string workDir;
    EThermalBondaryCondition topUniformBC;
    EThermalBondaryCondition botUniformBC;
    std::vector<BlockBoundaryCondition> topBlockBC;
    std::vector<BlockBoundaryCondition> botBlokcBC;
    ETemperature envTemperature{25, ETemperatureUnit::Celsius};

    virtual EModelType GetModelType() const = 0;

    virtual void AddBlockBC(EOrientation orient, FBox2D block, BCType type, EFloat value)
    {
        if (EOrientation::Top == orient)
            topBlockBC.emplace_back(std::move(block), EThermalBondaryCondition(value, type));
        if (EOrientation::Bot == orient)
            botBlokcBC.emplace_back(std::move(block), EThermalBondaryCondition(value, type));
    }

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<EThermalModelExtractionSettings>>(&settings);
        if (nullptr == ps) return false;
        if (topUniformBC != ps->topUniformBC ||
            botUniformBC != ps->botUniformBC ||
            topBlockBC != ps->topBlockBC ||
            botBlokcBC != ps->botBlokcBC ||
            envTemperature != ps->envTemperature) return false;
        return true;
    }
protected:
    EThermalModelExtractionSettings() = default;
};

struct EGridThermalModelExtractionSettings : public EThermalModelExtractionSettings
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) 
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EThermalModelExtractionSettings);
        ar & boost::serialization::make_nvp("dump_hotmaps", dumpHotmaps);
        ar & boost::serialization::make_nvp("dump_density_file", dumpDensityFile);
        ar & boost::serialization::make_nvp("dump_temperature_file", dumpTemperatureFile);
        ar & boost::serialization::make_nvp("metal_fraction_mapping_settings", metalFractionMappingSettings);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    virtual ~EGridThermalModelExtractionSettings() = default;
    bool dumpHotmaps = false;
    bool dumpDensityFile = false;
    bool dumpTemperatureFile = false;
    EMetalFractionMappingSettings metalFractionMappingSettings;

    virtual EModelType GetModelType() const override { return EModelType::ThermalGrid; }

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<EGridThermalModelExtractionSettings>>(&settings);
        if (nullptr == ps) return false;

        if (not EThermalModelExtractionSettings::operator==(settings)) return false;
        if (metalFractionMappingSettings != ps->metalFractionMappingSettings) return false;
        return true;
    }
};

struct EPrismThermalModelExtractionSettings : public EThermalModelExtractionSettings
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EThermalModelExtractionSettings);
        ar & boost::serialization::make_nvp("mesh_settings", meshSettings);
        ar & boost::serialization::make_nvp("polygon_merge_settings", polygonMergeSettings);
        ar & boost::serialization::make_nvp("layer_cut_settings", layerCutSettings);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    EPrismMeshSettings meshSettings;
    ELayoutPolygonMergeSettings polygonMergeSettings;
    ELayerCutModelExtractionSettings layerCutSettings;
    virtual ~EPrismThermalModelExtractionSettings() = default;
    
    virtual EModelType GetModelType() const override
    {
        return meshSettings.genMeshByLayer ? EModelType::ThermalStackupPrism : EModelType::ThermalPrism;
    }

    virtual void AddBlockBC(EOrientation orient, FBox2D block, BCType type, EFloat value) override
    {
        EThermalModelExtractionSettings::AddBlockBC(orient, block, type, value);
        layerCutSettings.imprintBox.emplace_back(std::move(block));
    }

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<EPrismThermalModelExtractionSettings>>(&settings);
        if (nullptr == ps) return false;

        if (not EThermalModelExtractionSettings::operator==(settings)) return false;
        if (meshSettings != ps->meshSettings ||
            polygonMergeSettings != ps->polygonMergeSettings ||
            layerCutSettings != ps->layerCutSettings) return false;
        return true;
    }
};

struct EThermalSimulationSetup
{
    virtual ~EThermalSimulationSetup() = default;
    std::string workDir;
    std::vector<FPoint3D> monitors;
protected:
    EThermalSimulationSetup() = default;
};

enum class EThermalNetworkStaticSolverType
{
    SparseLU = 0,
    SimplicialCholesky = 1,
    ConjugateGradient = 2,
};

struct EThermalSettings
{
    virtual ~EThermalSettings() = default;
    bool dumpResults = true;
    size_t threads = 1;
    ETemperature envTemperature{25, ETemperatureUnit::Celsius};
};

struct EThermalStaticSettings : public EThermalSettings
{
    bool maximumRes = true;
    bool dumpHotmaps = false;
    EFloat residual = 0.1;
    size_t iteration = 10;
    EThermalNetworkStaticSolverType solverType = EThermalNetworkStaticSolverType::ConjugateGradient;
};

struct EThermalStaticSimulationSetup : public EThermalSimulationSetup
{
    EThermalStaticSettings settings;
};

using EThermalTransientExcitation = std::function<EFloat(EFloat, size_t)>;//power = f(t, scenario)

struct EThermalTransientSettings : public EThermalSettings
{
    virtual ~EThermalTransientSettings() = default;

    bool mor{false};
    bool verbose{false};
    bool adaptive{true};
    bool temperatureDepend{true};
    EFloat step{1};
    EFloat duration{10};
    EFloat absoluteError{1e-6};
    EFloat relativeError{1e-6};
    EFloat minSamplingInterval{0};
    EFloat samplingWindow{0};
    CPtr<EThermalTransientExcitation> excitation{nullptr};
};

struct EThermalTransientSimulationSetup : public EThermalSimulationSetup
{
    EThermalTransientSettings settings;
};

struct EThermalNetworkSolveSettings
{
    virtual ~EThermalNetworkSolveSettings() = default;
    std::string workDir;
    std::vector<size_t> probs;
protected:
    EThermalNetworkSolveSettings() = default;
};

struct EThermalNetworkStaticSolveSettings : public EThermalNetworkSolveSettings, EThermalStaticSettings
{
    void operator= (const EThermalStaticSettings & settings)
    {
        dynamic_cast<EThermalStaticSettings&>(*this) = settings;
    }
};

struct EThermalNetworkTransientSolveSettings : public EThermalNetworkSolveSettings, EThermalTransientSettings
{

    void operator= (const EThermalTransientSettings & settings)
    {
        dynamic_cast<EThermalTransientSettings&>(*this) = settings;
    }
};

struct ELayout2CtmSettings
{
    size_t threads = 1;
    std::string dirName;
    std::string filename;
    EFloat resolution = 10;//unit: um
    EFloat regionExtTop = 0;
    EFloat regionExtBot = 0;
    EFloat regionExtLeft = 0;
    EFloat regionExtRight = 0;
    std::unordered_set<ENetId> selectNets;
};

struct ELayoutViewRendererSettings
{
    enum class Format {
        PNG = 0,
        VTK = 1//todo
    };
    Format format;
    size_t width = 1024;
    std::string dirName;
    std::unordered_set<ENetId> selectNets;
    std::unordered_set<ELayerId> selectLayers;
};

}//namespace ecad