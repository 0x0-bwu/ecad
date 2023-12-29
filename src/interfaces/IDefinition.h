#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
#include <string>
namespace ecad {
class IDatabase;
class ECAD_API IDefinition : public Clonable<IDefinition>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IDefinition() = default;
    virtual const std::string & GetName() const = 0;
    virtual CPtr<IDatabase> GetDatabase() const = 0;
    virtual EDefinitionType GetDefinitionType() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IDefinition)