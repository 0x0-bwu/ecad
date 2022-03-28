#ifndef ECAD_ECADSETTINGS_H
#define ECAD_ECADSETTINGS_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include <unordered_set>
#include <array>
namespace ecad {

struct ELayoutPolygonMergeSettings
{
    size_t threads = 1;
    std::string outFile;
    bool includePadstackInst = true;
    bool includeDielectricLayer = true;
    bool skipTopBotDielectricLayers = false;
    std::unordered_set<ENetId> selectNets;
};

struct EMetalFractionMappingSettings
{
    size_t threads = 1;
    std::string outFile;
    EValue regionExtTop = 0;
    EValue regionExtBot = 0;
    EValue regionExtLeft = 0;
    EValue regionExtRight = 0;
    bool mergeGeomBeforeMapping = true;
    std::array<size_t, 2> grid = {1, 1};
    std::unordered_set<ENetId> selectNets;
};

struct EThermalNetworkExtractionSettings
{
    size_t threads = 1;
    std::string outDir;
#ifdef BOOST_GIL_IO_PNG_SUPPORT
    bool dumpHotmaps = false;
#endif//#ifdef BOOST_GIL_IO_PNG_SUPPORT
    bool dumpDensityFile = false;
    bool dumpTemperatureFile = false;
    EValue regionExtTop = 0;
    EValue regionExtBot = 0;
    EValue regionExtLeft = 0;
    EValue regionExtRight = 0;
    std::array<size_t, 2> grid = {1, 1};
    bool mergeGeomBeforeMetalMapping = true;
};

struct EThermalNetworkSolveSettings
{
    size_t iteration = 10;
    EValue residual = 0.1;
};

}//namespace ecad
#endif//ECAD_ECADSETTINGS_H