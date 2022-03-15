#ifndef ECAD_ICOMPONENTDEFPINCOLLECTION_H
#define ECAD_ICOMPONENTDEFPINCOLLECTION_H
#include "ECadCommon.h"
#include "IIterator.h"
#include "Protocol.h"
namespace ecad {
class IComponentDefPin;
class ECAD_API IComponentDefPinCollection : public Clonable<IComponentDefPinCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponentDefPinCollection() = default;
    virtual ComponentDefPinIter GetComponentDefPinIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponentDefPinCollection)
#endif//ECAD_ICOMPONENTDEFPINCOLLECTION_H