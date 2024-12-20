#pragma once
#include "basic/ECadCommon.h"
#include "basic/Protocol.h"
#include "basic/ECadDef.h"
#include "interface/IIterator.h"
namespace ecad {
class IPadstackDef;
class IComponentDefPin;
class ECAD_API IComponentDefPinCollection : public Clonable<IComponentDefPinCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponentDefPinCollection() = default;
    virtual Ptr<IComponentDefPin> AddPin(UPtr<IComponentDefPin> pin) = 0;
    virtual Ptr<IComponentDefPin> CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef = nullptr, ELayerId lyr = noLayer) = 0;
    virtual Ptr<IComponentDefPin> FindPinByName(const std::string & name) const = 0;
    virtual ComponentDefPinIter GetComponentDefPinIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponentDefPinCollection)