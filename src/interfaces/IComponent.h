#pragma once
#include "ECadCommon.h"
#include "Protocol.h"
#include <string>
namespace ecad {
class IComponentDef;
class ECAD_API IComponent : public Clonable<IComponent>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponent() = default;
    virtual const std::string & GetName() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponent)