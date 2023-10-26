#pragma once
#include "ECadCommon.h"
#include "Protocol.h"
namespace ecad {
class ECAD_API ICollection : public Clonable<ICollection>, public Printable
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ICollection() = default;
    virtual size_t Size() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ICollection)