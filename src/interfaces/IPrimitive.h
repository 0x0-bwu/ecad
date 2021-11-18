#ifndef ECAD_IPRIMITIVE_H
#define ECAD_IPRIMITIVE_H
#include "ECadCommon.h"
#include "ETransform.h"
#include "ECadDef.h"
#include "Protocol.h"
#include <string>
namespace ecad {
class IText;
class EShape;
class IConnObj;
class IGeometry2D;
class ECAD_API IPrimitive : public Clonable<IPrimitive>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IPrimitive() = default;
    virtual Ptr<IText> GetTextFromPrimitive() = 0;
    virtual Ptr<IConnObj> GetConnObjFromPrimitive() = 0;
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
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IText)
#endif//ECAD_IPRIMITIVE_H