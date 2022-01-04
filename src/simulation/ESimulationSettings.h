#ifndef ECAD_ESIM_ESIMULATIONSETTINGS_H
#define ECAD_ESIM_ESIMULATIONSETTINGS_H

#include "ECadCommon.h"
#include <unordered_set>
#include <array>
namespace ecad {
namespace esim {

struct ECAD_API EMetalFractionMappingSettings
{
    size_t threads = 1;
    std::string outFile;
    ECoordUnits coordUnits;
    EValue regionExtTop = 0;
    EValue regionExtBot = 0;
    EValue regionExtLeft = 0;
    EValue regionExtRight = 0;
    std::array<size_t, 2> grid;
    std::unordered_set<int> selectNets;
};


}//namespace esim
}//namespace ecad

#endif//ECAD_ESIM_ESIMULATIONSETTINGS_H