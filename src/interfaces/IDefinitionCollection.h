#ifndef ECAD_IDEFINITIONCOLLECTION_H
#define ECAD_IDEFINITIONCOLLECTION_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
namespace ecad {

class IDefinition;
class ECAD_API IDefinitionCollection : public Clonable<IDefinitionCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IDefinitionCollection() = default;
    virtual Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type) = 0;
    virtual Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const = 0;
    virtual Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition) = 0;
    virtual Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const = 0;
    virtual std::string GetNextDefName(const std::string & base, EDefinitionType type) const = 0;
    virtual size_t Size() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IDefinitionCollection)
#endif//ECAD_IDEFINITIONCOLLECTION_H