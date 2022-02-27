#ifndef ECAD_ECADSETTINGS_H
#define ECAD_ECADSETTINGS_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include <unordered_set>
#include <array>
namespace ecad {

struct EMetalFractionMappingSettings
{
    size_t threads = 1;
    std::string outFile;
    EValue regionExtTop = 0;
    EValue regionExtBot = 0;
    EValue regionExtLeft = 0;
    EValue regionExtRight = 0;
    std::array<size_t, 2> grid = {1, 1};
    std::unordered_set<ENetId> selectNets;
};

struct ELayoutPolygonMergeSettings
{
    size_t threads = 1;
    std::string outFile;
    bool includePadstackInst = true;
    bool includeDielectricLayer = true;
    bool skipTopBotDielectricLayers = false;
    std::unordered_set<ENetId> selectNets;
};

}//namespace ecad
#endif//ECAD_ECADSETTINGS_H