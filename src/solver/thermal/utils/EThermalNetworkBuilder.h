#pragma once
#include "basic/ECadCommon.h"
namespace ecad {
namespace solver {

struct EThermalNetworkBuildSummary
{
    size_t totalNodes = 0;
    size_t fixedTNodes = 0;
    size_t boundaryNodes = 0;
    double iHeatFlow = 0, oHeatFlow = 0, jouleHeat = 0;
    void Reset() { *this = EThermalNetworkBuildSummary{}; }
};

class ECAD_API EThermalNetworkBuilder
{
public:
    mutable EThermalNetworkBuildSummary summary;
    virtual ~EThermalNetworkBuilder() = default;
};

}//namesapce solver
}//namespace ecad