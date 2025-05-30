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
    ENetIdSet selectNets;

    explicit ELayoutPolygonMergeSettings(size_t threads, const ENetIdSet & selectNets)
     : threads(threads), selectNets(selectNets) {}

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
    ESize2D grid = ESize2D(1, 1);
    ENetIdSet selectNets;
    ELayoutPolygonMergeSettings polygonMergeSettings;

    explicit EMetalFractionMappingSettings(size_t threads, const ENetIdSet & selectNets)
     : threads(threads), selectNets(selectNets), polygonMergeSettings(threads, selectNets) {}
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
        ar & boost::serialization::make_nvp("add_circle_center_as_steiner_point", addCircleCenterAsSteinerPoint);
        ar & boost::serialization::make_nvp("imprint_box", imprintBox);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    bool dumpSketchImg = true;
    size_t layerCutPrecision = 6;
    EFloat layerTransitionRatio = 2;
    bool addCircleCenterAsSteinerPoint{false};
    std::vector<FBox2D> imprintBox;

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<ELayerCutModelExtractionSettings>>(&settings);
        if (nullptr == ps) return false;
        if (layerCutPrecision != ps->layerCutPrecision ||
            layerTransitionRatio != ps->layerTransitionRatio ||
            addCircleCenterAsSteinerPoint != ps->addCircleCenterAsSteinerPoint ||
            imprintBox != ps->imprintBox) return false;
        return true;
    }
};

using ELayerCutModelBuildSettings = ELayerCutModelExtractionSettings;

struct EThermalBoundaryCondition
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

    using BCType = EThermalBoundaryConditionType;
    BCType type{BCType::HTC};
    EFloat value{invalidFloat};
    EThermalBoundaryCondition() = default;
    EThermalBoundaryCondition(EFloat value, BCType type) : type(type), value(value) {}
    bool isValid() const { return ecad::isValid(value); }

    bool operator== (const EThermalBoundaryCondition & settings) const
    {
        return type == settings.type && math::EQ(value, settings.value);
    }

    bool operator != (const EThermalBoundaryCondition & settings) const
    {
        return not (*this == settings);
    }
};

struct EThermalModelExtractionSettings : public ECadSettings, public Clonable<EThermalModelExtractionSettings>
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECadSettings);
        ar & boost::serialization::make_nvp("force_rebuild", forceRebuild);
        ar & boost::serialization::make_nvp("threads", threads);
        ar & boost::serialization::make_nvp("work_dir", workDir);
        ar & boost::serialization::make_nvp("top_uniform_bc", topUniformBC);
        ar & boost::serialization::make_nvp("bot_uniform_bc", botUniformBC);
        ar & boost::serialization::make_nvp("top_block_bc", topBlockBC);
        ar & boost::serialization::make_nvp("bot_block_bc", botBlockBC);
        ar & boost::serialization::make_nvp("env_temperature", envTemperature);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    using BCType = EThermalBoundaryConditionType;
    using BlockBoundaryCondition = std::pair<FBox2D, EThermalBoundaryCondition>;
    virtual ~EThermalModelExtractionSettings() = default;
    bool forceRebuild{false};
    size_t threads;
    std::string workDir;
    EThermalBoundaryCondition topUniformBC;
    EThermalBoundaryCondition botUniformBC;
    std::vector<BlockBoundaryCondition> topBlockBC;
    std::vector<BlockBoundaryCondition> botBlockBC;
    ETemperature envTemperature{25, ETemperatureUnit::Celsius};

    virtual EModelType GetModelType() const = 0;

    virtual void AddBlockBC(EOrientation orient, FBox2D block, BCType type, EFloat value)
    {
        if (EOrientation::Top == orient)
            topBlockBC.emplace_back(std::move(block), EThermalBoundaryCondition(value, type));
        if (EOrientation::Bot == orient)
            botBlockBC.emplace_back(std::move(block), EThermalBoundaryCondition(value, type));
    }

    virtual bool operator== (const ECadSettings & settings) const override
    {
        auto ps = dynamic_cast<CPtr<EThermalModelExtractionSettings>>(&settings);
        if (nullptr == ps) return false;
        if (topUniformBC != ps->topUniformBC ||
            botUniformBC != ps->botUniformBC ||
            topBlockBC != ps->topBlockBC ||
            botBlockBC != ps->botBlockBC ||
            envTemperature != ps->envTemperature) return false;
        return true;
    }
protected:
    explicit EThermalModelExtractionSettings(std::string workDir_, size_t threads)
     : threads(threads), workDir(std::move(workDir_))
    {
        if (workDir.empty()) workDir = fs::CurrentPath();
    }
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
    explicit EGridThermalModelExtractionSettings(std::string workDir, size_t threads, const ENetIdSet & selectNets)
     : EThermalModelExtractionSettings(std::move(workDir), threads), metalFractionMappingSettings(threads, selectNets) {}
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

private:
    virtual Ptr<EGridThermalModelExtractionSettings> CloneImp() const override { return new EGridThermalModelExtractionSettings(*this); }
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
    explicit EPrismThermalModelExtractionSettings(std::string workDir, size_t threads, const ENetIdSet & selectNets)
     : EThermalModelExtractionSettings(std::move(workDir), threads), polygonMergeSettings(threads, selectNets) {}
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

private:
    virtual Ptr<EPrismThermalModelExtractionSettings> CloneImp() const override { return new EPrismThermalModelExtractionSettings(*this); }
};

struct EThermalSimulationSetup
{
    virtual ~EThermalSimulationSetup() = default;
    std::string workDir;
    std::vector<FPoint3D> monitors;
    UPtr<EThermalModelExtractionSettings> extractionSettings;

    Ptr<EGridThermalModelExtractionSettings> GetGridThermalModelExtractionSettings()
    {
        return dynamic_cast<Ptr<EGridThermalModelExtractionSettings>>(extractionSettings.get());
    }

    Ptr<EPrismThermalModelExtractionSettings> GetPrismThermalModelExtractionSettings()
    {
        return dynamic_cast<Ptr<EPrismThermalModelExtractionSettings>>(extractionSettings.get());
    }
protected:
    explicit EThermalSimulationSetup(std::string workDir_, size_t threads, const ENetIdSet & selectNets)
     : workDir(std::move(workDir_))
    {
        if (workDir.empty()) workDir = fs::CurrentPath();
        extractionSettings.reset(new EPrismThermalModelExtractionSettings(workDir, threads, selectNets));
    }
};

enum class EThermalNetworkStaticSolverType
{
    SparseLU = 0,
    Cholesky = 1,
    LLT = 2,
    LDLT = 3,
    ConjugateGradient = 10,
};

struct EThermalSettings
{
    bool dumpResults = true;
    size_t threads = 1;
    ETemperature envTemperature{25, ETemperatureUnit::Celsius};
    explicit EThermalSettings(size_t threads) : threads(threads) {}
    virtual ~EThermalSettings() = default;
};

struct EThermalStaticSettings : public EThermalSettings
{
    bool maximumRes = true;
    bool dumpHotmaps = false;
    bool dumpMatrices = false;
    EFloat residual = 0.1;
    size_t iteration = 10;
    EThermalNetworkStaticSolverType solverType = EThermalNetworkStaticSolverType::ConjugateGradient;
    explicit EThermalStaticSettings(size_t threads) : EThermalSettings(threads) {}
};

struct EThermalStaticSimulationSetup : public EThermalSimulationSetup
{
    explicit EThermalStaticSimulationSetup(std::string workDir, size_t threads, const ENetIdSet & selectNets)
     : EThermalSimulationSetup(std::move(workDir), threads, selectNets), settings(threads) {}
    EThermalStaticSettings settings;
};

using EThermalTransientExcitation = std::function<EFloat(EFloat, size_t)>;//ratio = f(t, scenario), range[0, 1]

struct EThermalModelReductionSettings
{
    size_t order = 0;
    std::string romLoadFile;
    std::string romSaveFile;
};

struct EThermalTransientSettings : public EThermalSettings
{
    bool verbose{false};
    bool adaptive{true};
    bool temperatureDepend{true};
    EFloat step{1};
    EFloat duration{10};
    EFloat absoluteError{1e-6};
    EFloat relativeError{1e-6};
    EFloat minSamplingInterval{0};
    EFloat samplingWindow{0};
    EThermalModelReductionSettings mor;
    explicit EThermalTransientSettings(size_t threads) : EThermalSettings(threads) {}
    virtual ~EThermalTransientSettings() = default;
};

struct EThermalTransientSimulationSetup : public EThermalSimulationSetup
{
    explicit EThermalTransientSimulationSetup(std::string workDir, size_t threads, const ENetIdSet & selectNets)
     : EThermalSimulationSetup(std::move(workDir), threads, selectNets), settings(threads) {}
    EThermalTransientSettings settings;
};

struct EThermalNetworkSolveSettings
{
    virtual ~EThermalNetworkSolveSettings() = default;
    std::string workDir;
    std::vector<size_t> probs;
protected:
    explicit EThermalNetworkSolveSettings(std::string workDir)
     : workDir(std::move(workDir)) {}
};

struct EThermalNetworkStaticSolveSettings : public EThermalNetworkSolveSettings, EThermalStaticSettings
{
    explicit EThermalNetworkStaticSolveSettings(std::string workDir, size_t threads)
     : EThermalNetworkSolveSettings(workDir), EThermalStaticSettings(threads) {}
    void operator= (const EThermalStaticSettings & settings)
    {
        dynamic_cast<EThermalStaticSettings&>(*this) = settings;
    }
};

struct EThermalNetworkTransientSolveSettings : public EThermalNetworkSolveSettings, EThermalTransientSettings
{
    explicit EThermalNetworkTransientSolveSettings(std::string workDir, size_t threads)
     : EThermalNetworkSolveSettings(workDir), EThermalTransientSettings(threads) {}
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
    ENetIdSet selectNets;
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
    ENetIdSet selectNets;
    ELayerIdSet selectLayers;
};

}//namespace ecad