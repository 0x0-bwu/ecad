#pragma once
#include "ECadCommon.h"
#include "ETransform.h"
#include "ECadDef.h"
#include "Protocol.h"
#include "IIterator.h"
namespace ecad {

class IText;
class EShape;
class ILayerMap;
class IBondwire;
class IPrimitive;
class ECAD_API IPrimitiveCollection : public Clonable<IPrimitiveCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IPrimitiveCollection() = default;
    virtual Ptr<IPrimitive> GetPrimitive(size_t index) const = 0;
    virtual Ptr<IPrimitive> AddPrimitive(UPtr<IPrimitive> primitive) = 0;
    virtual bool SetPrimitive(UPtr<IPrimitive> primitive, size_t index) = 0;

    virtual Ptr<IPrimitive> CreateGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape) = 0;
    virtual Ptr<IBondwire> CreateBondwire(std::string name, ENetId net, EPoint2D start, EPoint2D end, FCoord radius) = 0;
    virtual Ptr<IText> CreateText(ELayerId layer, const ETransform2D & transform, const std::string & text) = 0;
    virtual void Map(CPtr<ILayerMap> lyrMap) = 0;
    virtual PrimitiveIter GetPrimitiveIter() const = 0;
    virtual UPtr<IPrimitive> PopBack() = 0;
    virtual size_t Size() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IPrimitiveCollection)