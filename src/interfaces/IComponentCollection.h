#pragma once
#include "ECadCommon.h"
#include "ETransform.h"
#include "ECadDef.h"
#include "Protocol.h"
#include "IIterator.h"
namespace ecad {

class IComponent;
class ECAD_API IComponentCollection : public Clonable<IComponentCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponentCollection() = default;
    virtual ComponentIter GetComponentIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponentCollection)