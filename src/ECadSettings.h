#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include <unordered_set>
#include <functional>
#include <array>
#include <set>
namespace ecad {

struct EDataMgrSettings
{
    char hierSep = '/';
    size_t threads = 8;
    size_t circleDiv = 16;
};

struct ELayoutPolygonMergeSettings
{
    size_t threads = 1;
    std::string outFile;
    bool mtByLayer{true};
    bool includePadstackInst = true;
    bool includeDielectricLayer = true;
    bool skipTopBotDielectricLayers = false;
    std::unordered_set<ENetId> selectNets;
};

struct EMetalFractionMappingSettings
{
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
};

struct EMeshSettings
{
    virtual ~EMeshSettings() = default;
    EFloat minAlpha{20};//Deg
    EFloat minLen{0};
    EFloat maxLen{maxFloat};
    EFloat tolerance{0};
};

struct EPrismaMeshSettings : public EMeshSettings
{
    virtual ~EPrismaMeshSettings() = default;
    size_t iteration = 0;
};

struct ELayerCutModelExtractionSettings
{
    bool dumpSketchImg = true;
    size_t layerCutPrecision = 6;
    EFloat layerTransitionRatio = 2;
    bool addCircleCenterAsSteinerPoints{false};
};

using ELayerCutModelBuildSettings = ELayerCutModelExtractionSettings;

struct EThermalBondaryCondition
{
    using BCType = EThermalBondaryConditionType;
    BCType type{BCType::HTC};
    EFloat value{invalidFloat};
    EThermalBondaryCondition() = default;
    EThermalBondaryCondition(EFloat value, BCType type) : type(type), value(value) {}
    bool isValid() const { return ecad::isValid(value); }
};

struct EThermalModelExtractionSettings
{
    using BlockBoundaryCondition = std::pair<FBox2D, EThermalBondaryCondition>;
    virtual ~EThermalModelExtractionSettings() = default;
    std::string workDir;
    EThermalBondaryCondition topUniformBC;
    EThermalBondaryCondition botUniformBC;
    std::vector<BlockBoundaryCondition> topBlockBC;
    std::vector<BlockBoundaryCondition> botBlokcBC;
    ETemperature envTemperature{25, ETemperatureUnit::Celsius};
protected:
    EThermalModelExtractionSettings() = default;
};

struct EGridThermalModelExtractionSettings : public EThermalModelExtractionSettings
{
    virtual ~EGridThermalModelExtractionSettings() = default;
    bool dumpHotmaps = false;
    bool dumpDensityFile = false;
    bool dumpTemperatureFile = false;
    EMetalFractionMappingSettings metalFractionMappingSettings;
};

struct EPrismaThermalModelExtractionSettings : public EThermalModelExtractionSettings
{
    EPrismaMeshSettings meshSettings;
    ELayoutPolygonMergeSettings polygonMergeSettings;
    ELayerCutModelExtractionSettings layerCutSettings;
};

struct EThermalSimulationSetup
{
    virtual ~EThermalSimulationSetup() = default;
    std::string workDir;
protected:
    EThermalSimulationSetup() = default;
};

struct EThermalStaticSettings
{
    bool maximumRes = true;
    bool dumpHotmaps = false;
    EFloat residual = 0.1;
    size_t threads = 1;
    size_t iteration = 10;
    ETemperature envTemperature{25, ETemperatureUnit::Celsius};
};

struct EThermalStaticSimulationSetup : public EThermalSimulationSetup
{
    EThermalStaticSettings settings;
};

using EThermalTransientExcitation = std::function<EFloat(EFloat)>;

struct EThermalTransientSettings
{
    virtual ~EThermalTransientSettings() = default;

    bool mor{false};
    bool verbose{false};
    bool adaptive{true};
    bool dumpRawData{false};
    size_t threads = 1;
    EFloat step{1};
    EFloat duration{10};
    EFloat absoluteError{1e-6};
    EFloat relativeError{1e-6};
    EFloat minSamplingInterval{0};
    EFloat samplingWindow{maxFloat};
    CPtr<EThermalTransientExcitation> excitation{nullptr};
    ETemperature envTemperature{25, ETemperatureUnit::Celsius};
};

struct EThermalTransientSimulationSetup : public EThermalSimulationSetup
{
    std::vector<FPoint3D> monitors; //todo
    EThermalTransientSettings settings;
};

struct EThermalNetworkSolveSettings
{
    virtual ~EThermalNetworkSolveSettings() = default;
    std::string workDir;
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
    std::set<size_t> probs;

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