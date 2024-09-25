#pragma once
#include "basic/ECadCommon.h"
#include "IIterator.h"
namespace ecad {
class IComponentDef;
class ECAD_API IComponentDefCollection : public Clonable<IComponentDefCollection>, public Printable
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponentDefCollection() = default;
    virtual ComponentDefIter GetComponentDefIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponentDefCollection)