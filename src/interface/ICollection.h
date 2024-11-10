#pragma once
#include "basic/ECadCommon.h"
namespace ecad {
class ECAD_API ICollection : public Clonable<ICollection>, public Printable
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ICollection() = default;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;

protected:
    virtual ECollectionType GetType() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ICollection)