#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include <unordered_set>
#include <array>
namespace ecad {

struct EDataMgrSettings
{
    size_t threads = 8;
    size_t circleDiv = 16;
};

struct ELayoutPolygonMergeSettings
{
    std::string outFile;
    bool includePadstackInst = true;
    bool includeDielectricLayer = true;
    bool skipTopBotDielectricLayers = false;
    std::unordered_set<ENetId> selectNets;
};

struct EMetalFractionMappingSettings
{
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

enum class EThermalSimuType { Static, Transient };
struct EThermalSimulationSetup
{
    EThermalSimuType simuType = EThermalSimuType::Static;
    std::string workDir;
    EFloat environmentTemperature = 25;
};

struct EThermalNetworkSolveSettings
{
    bool dumpHotmaps = true;
    EFloat iniT = 25;
    EFloat residual = 0.5;
    size_t iteration = 10;
    std::string spiceFile;
    std::string workDir;
};

struct ELayout2CtmSettings
{
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