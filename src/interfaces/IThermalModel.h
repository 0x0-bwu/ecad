#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include <string>
namespace ecad {
class ECAD_API IThermalModel
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IThermalModel() = default;
    virtual EThermalModelType GetModelType() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IThermalModel)