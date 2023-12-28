#pragma once
#include "ECadCommon.h"
#include "ETransform.h"
#include "ECadDef.h"
#include "Protocol.h"
#include <string>
namespace ecad {
class IText;
class EShape;
class IConnObj;
class IBondwire;
class IComponent;
class IGeometry2D;
class IPadstackDef;
class ECAD_API IPrimitive : public Clonable<IPrimitive>, public Printable
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IPrimitive() = default;
    virtual Ptr<IText> GetTextFromPrimitive() = 0;
    virtual Ptr<IConnObj> GetConnObjFromPrimitive() = 0;
    virtual Ptr<IBondwire> GetBondwireFromPrimitive() = 0;
    virtual Ptr<IGeometry2D> GetGeometry2DFromPrimitive() = 0;

    virtual void SetNet(ENetId net) = 0;
    virtual ENetId GetNet() const = 0;

    virtual void SetLayer(ELayerId layer) = 0;
    virtual ELayerId GetLayer() const = 0;

    virtual EPrimitiveType GetPrimitiveType() const = 0;
};

class IGeometry2D
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IGeometry2D() = default;
    virtual Ptr<EShape> GetShape() const = 0;
    virtual void Transform(const ETransform2D & transform) = 0;
};

class IBondwire
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IBondwire() = default;

    virtual const std::string & GetName() const = 0;
    
    virtual void SetNet(ENetId net) = 0;
    virtual ENetId GetNet() const = 0;
    
    virtual void SetRadius(EFloat r) = 0;
    virtual EFloat GetRadius() const = 0;

    virtual const EPoint2D & GetStartPt() const = 0;
    virtual const EPoint2D & GetEndPt() const = 0;

    virtual void SetStartLayer(ELayerId layerId, bool flipped) = 0;
    virtual ELayerId GetStartLayer(Ptr<bool> flipped = nullptr) const = 0;

    virtual void SetEndLayer(ELayerId layerId, bool flipped) = 0;
    virtual ELayerId GetEndLayer(Ptr<bool> flipped = nullptr) const = 0;

    virtual void SetMaterial(const std::string & material) = 0;
    virtual const std::string & GetMaterial() const = 0;

    virtual void SetBondwireType(EBondwireType type) = 0;
    virtual EBondwireType GetBondwireType() const = 0;

    virtual void SetHeight(EFloat height) = 0;
    virtual EFloat GetHeight() const = 0;

    virtual void SetStartComponent(CPtr<IComponent> comp) = 0;
    virtual CPtr<IComponent> GetStartComponent() const = 0;

    virtual void SetEndComponent(CPtr<IComponent> comp) = 0;
    virtual CPtr<IComponent> GetEndComponent() const = 0;
    
    virtual void SetSolderJoints(CPtr<IPadstackDef> s) = 0;
    virtual CPtr<IPadstackDef> GetSolderJoints() const = 0;

    virtual void SetCurrent(EFloat current) = 0;
    virtual EFloat GetCurrent() const = 0;

    virtual void Transform(const ETransform2D & transform) = 0;
};

class IText : public Transformable2D
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IText() = default;
    virtual const std::string & GetText() const = 0;
    virtual EPoint2D GetPosition() const = 0;
};

}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IPrimitive)
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IGeometry2D)
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IBondwrie)
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IText)
