#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include <string>
namespace ecad {
class ECAD_API IModel
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IModel() = default;
    virtual EModelType GetModelType() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IModel)