#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
namespace ecad {
class EShape;
class ILayerMap;
class IPadstackDef;
class ECAD_API IPadstackInst : public Clonable<IPadstackInst>, public Transformable2D
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IPadstackInst() = default;
    
    virtual void SetNet(ENetId net) = 0;
    virtual ENetId GetNet() const = 0;
    
    virtual void SetLayerRange(ELayerId top, ELayerId bot) = 0;
    virtual void GetLayerRange(ELayerId & top, ELayerId & bot) const = 0;

    virtual void SetLayerMap(CPtr<ILayerMap> layerMap) = 0;
    virtual CPtr<ILayerMap> GetLayerMap() const = 0;

    virtual CPtr<IPadstackDef> GetPadstackDef() const = 0;

    virtual bool isLayoutPin() const = 0;
    virtual void SetIsLayoutPin(bool isPin) = 0;

    virtual UPtr<EShape> GetLayerShape(ELayerId lyr) const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IPadstackInst)