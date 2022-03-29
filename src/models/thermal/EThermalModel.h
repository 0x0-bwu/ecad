#ifndef ECAD_EMODEL_ETHERM_ETHERMALMODEL_HPP
#define ECAD_EMODEL_ETHERM_ETHERMALMODEL_HPP
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

#ifdef ECAD_HEADER_ONLY
#include "EThermalModel.cpp"
#endif

#endif//ECAD_EMODEL_ETHERM_ETHERMALMODEL_HPP