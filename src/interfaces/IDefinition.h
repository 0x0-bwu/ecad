#ifndef ECAD_IDEFINITION_H
#define ECAD_IDEFINITION_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
#include <string>
namespace ecad {
class ECAD_API IDefinition : public Clonable<IDefinition>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IDefinition() = default;
    virtual const std::string & GetName() const = 0;
    virtual EDefinitionType GetDefinitionType() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IDefinition)
#endif//ECAD_IDEFINITION_H