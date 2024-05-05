#pragma once
#include "basic/ECadCommon.h"
#include "interface/IIterator.h"
namespace ecad {

class IPrimitiveCollection;
class IPadstackInstCollection;
class ECAD_API IConnObjCollection : public Clonable<IConnObjCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IConnObjCollection() = default;

    virtual Ptr<IPrimitiveCollection> GetPrimitiveCollection() const = 0;
    virtual Ptr<IPadstackInstCollection> GetPadstackInstCollection() const = 0;
    virtual ConnObjIter GetConnObjIter() const = 0;
    virtual size_t Size() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IConnObjCollection)