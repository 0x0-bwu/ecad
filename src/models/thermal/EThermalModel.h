#pragma once
#include "utilities/EMetalFractionMapping.h"
#include "generic/math/Interpolation.hpp"
#include "ECadCommon.h"
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