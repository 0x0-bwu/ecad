#pragma once
#include "ECadCommon.h"
#include "Protocol.h"
#include "ECadDef.h"
#include <string>
namespace ecad {
class ICell;
class ECAD_API IComponentDefPin : public Clonable<IComponentDefPin>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponentDefPin() = default;
    virtual const std::string & GetName() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponentDefPin)