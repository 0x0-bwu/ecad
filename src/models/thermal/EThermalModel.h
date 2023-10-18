#pragma once
#include "ECadCommon.h"
#include "utilities/EMetalFractionMapping.h"
#include "generic/math/Interpolation.hpp"
namespace ecad {
namespace emodel {
namespace etherm {

class ECAD_API EThermalModel
{
public:
    virtual ~EThermalModel();
};

}//namespace etherm
}//namesapce emodel
}//namespace ecad