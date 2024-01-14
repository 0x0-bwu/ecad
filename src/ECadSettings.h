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
};

struct EPrismaMeshSettings : public EMeshSettings
{
    virtual ~EPrismaMeshSettings() = default;
    size_t iteration = 0;
};

struct EThermalModelExtractionSettings
{
    virtual ~EThermalModelExtractionSettings() = default;
    std::string workDir;
protected:
    EThermalModelExtractionSettings() = default;
};

struct EGridThermalModelExtractionSettings : public EThermalModelExtractionSettings
{
    virtual ~EGridThermalModelExtractionSettings() = default;
    bool dumpHotmaps = false;
    bool dumpSpiceFile = false;
    bool dumpDensityFile = false;
    bool dumpTemperatureFile = false;
    EMetalFractionMappingSettings metalFractionMappingSettings;
};

struct EPrismaThermalModelExtractionSettings : public EThermalModelExtractionSettings
{
    EPrismaMeshSettings meshSettings;
    ELayoutPolygonMergeSettings polygonMergeSettings;
};

struct EThermalSimulationSetup
{
    virtual ~EThermalSimulationSetup() = default;
    std::string workDir;
    EFloat environmentTemperature = 25;
protected:
    EThermalSimulationSetup() = default;
};

struct EThermalStaticSettings
{
    EFloat residual = 0.5;
    size_t iteration = 10;
    bool dumpHotmaps = false;
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
    bool adaptive{true};
    bool dumpRawData{false};
    EFloat step{1};
    EFloat duration{10};
    EFloat absoluteError{1e-6};
    EFloat relativeError{1e-6};
    EFloat minSamplingInterval{0};
    EFloat samplingWindow{maxFloat};
    CPtr<EThermalTransientExcitation> excitation{nullptr};
};

struct EThermalTransientSimulationSetup : public EThermalSimulationSetup
{
    std::vector<FPoint3D> monitors; //todo
    EThermalTransientSettings settings;
};

struct EThermalNetworkSolveSettings
{
    virtual ~EThermalNetworkSolveSettings() = default;
    EFloat iniT = 25;
    size_t threads = 1;
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