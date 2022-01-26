#ifndef ECAD_EDEFINITION_HPP
#define ECAD_EDEFINITION_HPP
#include "interfaces/IDefinition.h"
#include "ECadCommon.h"
#include "EObject.h"
namespace ecad {

class ECAD_API EDefinition : public EObject, public IDefinition
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EDefinition();
    explicit EDefinition(std::string name);
    virtual ~EDefinition();

    ///Copy
    EDefinition(const EDefinition & other);
    EDefinition & operator= (const EDefinition & other);

    virtual EDefinitionType GetDefinitionType() const;
    void Print(std::ostream & os) const;
    const std::string & GetName() const;
    const EUuid & Uuid() const;
    std::string sUuid() const;

protected:
    ///Copy
    virtual Ptr<EDefinition> CloneImp() const override { return new EDefinition(*this); }
};

ECAD_ALWAYS_INLINE void EDefinition::Print(std::ostream & os) const
{
    EObject::Print(os);
    os << "DEFINITION TYPE: " << toString(GetDefinitionType()) << ECAD_EOL;
}

ECAD_ALWAYS_INLINE const std::string & EDefinition::GetName() const
{
    return EObject::GetName();
}

ECAD_ALWAYS_INLINE const EUuid & EDefinition::Uuid() const
{
    return EObject::Uuid();
}

ECAD_ALWAYS_INLINE std::string EDefinition::sUuid() const
{
    return EObject::sUuid();
}

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EDefinition)

#ifdef ECAD_HEADER_ONLY
#include "EDefinition.cpp"
#endif

#endif//ECAD_EDEFINITION_HPP