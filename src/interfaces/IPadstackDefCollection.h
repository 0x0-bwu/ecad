#pragma once
#include "ECadCommon.h"
#include "IIterator.h"
#include "Protocol.h"
#include "IPadstackDef.h"
namespace ecad {

class ECAD_API IPadstackDefCollection : public Clonable<IPadstackDefCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IPadstackDefCollection() = default;
    virtual PadstackDefIter GetPadstackDefIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IPadstackDefCollection)