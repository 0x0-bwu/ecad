#pragma once
#include "ECadCommon.h"
#include "Protocol.h"
#include "ECadDef.h"
#include <string>
namespace ecad {
class IComponentDef;
class ECAD_API IComponent : public Clonable<IComponent>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponent() = default;
    virtual const std::string & GetName() const = 0;

    virtual void SetPlacementLayer(ELayerId layer) = 0;
    virtual ELayerId GetPlacementLayer() const = 0;

    virtual void SetTransform(const ETransform2D & trans) = 0;
    virtual const ETransform2D & GetTransform() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponent)