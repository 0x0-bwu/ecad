#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include <unordered_set>
#include <array>
namespace ecad {

struct EDataMgrSettings {
    size_t threads = 8;
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
};

struct EThermalNetworkExtractionSettings
{
    std::string outDir;
    bool dumpHotmaps = false;
    bool dumpSpiceFile = false;
    bool dumpDensityFile = false;
    bool dumpTemperatureFile = false;
    EFloat regionExtTop = 0;
    EFloat regionExtBot = 0;
    EFloat regionExtLeft = 0;
    EFloat regionExtRight = 0;
    std::array<size_t, 2> grid = {1, 1};
    bool mergeGeomBeforeMetalMapping = true;
};

struct EThermalNetworkSolveSettings
{
    size_t iteration = 10;
    EFloat residual = 0.5;
    std::string spiceFile;
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