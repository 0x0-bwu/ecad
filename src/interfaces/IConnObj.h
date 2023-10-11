#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
namespace ecad {
class ECAD_API IConnObj : public Clonable<IConnObj>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IConnObj() = default;
    virtual void SetNet(ENetId net) = 0;
    virtual ENetId GetNet() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IConnObj)