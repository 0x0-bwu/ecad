#pragma once
#include "ECadCommon.h"
#include "Protocol.h"
#include "ECadDef.h"
#include <string>
namespace ecad {
class ILayoutView;
class IComponentDef;
class ECAD_API IComponent : public Clonable<IComponent>, public Printable
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponent() = default;
    virtual void SetName(std::string name) = 0;
    virtual const std::string & GetName() const = 0;

    virtual CPtr<IComponentDef> GetComponentDef() const = 0;

    virtual CPtr<ILayoutView> GetRefLayoutView() const = 0;

    virtual void SetPlacementLayer(ELayerId layer) = 0;
    virtual ELayerId GetPlacementLayer() const = 0;

    virtual void AddTransform(const ETransform2D & trans) = 0;
    virtual void SetTransform(const ETransform2D & trans) = 0;
    virtual const ETransform2D & GetTransform() const = 0;

    virtual void SetLossPower(EFloat power) = 0;
    virtual EFloat GetLossPower() const = 0;

    virtual EBox2D GetBoundingBox() const = 0;

    virtual void SetFlipped(bool flipped) = 0;
    virtual bool isFlipped() const = 0;

    virtual EFloat GetHeight() const = 0;

    virtual bool GetPinLocation(const std::string & name, EPoint2D & loc) const = 0;

};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponent)