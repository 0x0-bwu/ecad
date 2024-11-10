#pragma once
#include "basic/ECadCommon.h"
namespace ecad {

class IDatabase;
class IDefinition;
class ECAD_API IDefinitionCollection : public Clonable<IDefinitionCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IDefinitionCollection() = default;
    virtual Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type) { ECAD_ASSERT(false); return nullptr; }
    virtual Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const { ECAD_ASSERT(false); return nullptr; }
    virtual Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition) = 0;
    virtual Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const = 0;
    virtual std::string GetNextDefName(const std::string & base, EDefinitionType type) const = 0;
    virtual void SetDatabase(CPtr<IDatabase> database) = 0;
    virtual size_t Size() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IDefinitionCollection)