#pragma once
#include "basic/ECadCommon.h"
#include "interface/IDefinition.h"
#include "EObject.h"
namespace ecad {

class ECAD_API EDefinition : public EObject, public IDefinition
{
protected:
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EDefinition();
public:
    explicit EDefinition(std::string name, CPtr<IDatabase> database);
    virtual ~EDefinition();

    virtual EDefinitionType GetDefinitionType() const override;
    void SetDatabase(CPtr<IDatabase> database) override;
    CPtr<IDatabase> GetDatabase() const override;
    const std::string & GetName() const override;
    const EUuid & Uuid() const;
    std::string sUuid() const;
    
    void Print(std::ostream & os) const;
protected:
    ///Copy
    virtual Ptr<EDefinition> CloneImp() const override { return new EDefinition(*this); }

protected:
    CPtr<IDatabase> m_database;
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