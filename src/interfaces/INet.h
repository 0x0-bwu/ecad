#ifndef ECAD_INET_H
#define ECAD_INET_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
#include <string>
namespace ecad {
class ECAD_API INet : public Clonable<INet>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~INet() = default;
    virtual void SetNetId(ENetId id) = 0;
    virtual ENetId GetNetId() const = 0;
    virtual void SetName(std::string name) = 0;
    virtual const std::string & GetName() const = 0;
    virtual const EUuid & Uuid() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::INet)
#endif//ECAD_INET_H