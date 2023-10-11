#pragma once
#include "ECadCommon.h"
#include "IIterator.h"
#include "Protocol.h"
#include "ECadDef.h"
#include <string>
namespace ecad {
class INet;
class ECAD_API INetCollection : public Clonable<INetCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~INetCollection() = default;
    virtual std::string NextNetName(const std::string & name) const = 0;
    virtual Ptr<INet> FindNetByName(const std::string & name) const = 0;
    virtual Ptr<INet> FindNetByNetId(ENetId netId) const = 0;
    virtual Ptr<INet> CreateNet(const std::string & name) = 0;
    virtual Ptr<INet> AddNet(UPtr<INet> net) = 0;
    virtual NetIter GetNetIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::INetCollection)