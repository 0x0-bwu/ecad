#pragma once
#include "basic/ECadCommon.h"
namespace ecad {
class IDatabase;
class ECAD_API IDefinition : public Clonable<IDefinition>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IDefinition() = default;
    virtual const std::string & GetName() const = 0;
    virtual void SetDatabase(CPtr<IDatabase> database) = 0;
    virtual CPtr<IDatabase> GetDatabase() const = 0;
    virtual EDefinitionType GetDefinitionType() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IDefinition)