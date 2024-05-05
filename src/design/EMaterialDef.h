#pragma once
#include "interface/IMaterialProp.h"
#include "interface/IMaterialDef.h"
#include "EDefinition.h"
namespace ecad {

class ECAD_API EMaterialDef : public EDefinition, public IMaterialDef
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EMaterialDef();
public:
    EMaterialDef(std::string name, CPtr<IDatabase> database, EMaterialId id);
    virtual ~EMaterialDef();

    ///Copy
    EMaterialDef(const EMaterialDef & other);
    EMaterialDef & operator= (const EMaterialDef & other);

    EMaterialId GetMaterialId() const override;
    bool hasProperty(EMaterialPropId id) const override;
    void SetProperty(EMaterialPropId id, UPtr<IMaterialProp> prop) override;
    CPtr<IMaterialProp> GetProperty(EMaterialPropId id) const override;
    void SetMaterialType(EMaterialType type) override;
    EMaterialType GetMaterialType() const override;
    EDefinitionType GetDefinitionType() const override;
    const std::string & GetName() const override;
    const EUuid & Uuid() const override;

protected:
    ///Copy
    virtual Ptr<EMaterialDef> CloneImp() const override { return new EMaterialDef(*this); }
private:
    EMaterialId m_id;
    EMaterialType m_type{EMaterialType::Rigid};
    std::unordered_map<EMaterialPropId, UPtr<IMaterialProp> > m_properties;
};

ECAD_ALWAYS_INLINE const std::string & EMaterialDef::GetName() const
{
    return EDefinition::GetName();
}

ECAD_ALWAYS_INLINE const EUuid & EMaterialDef::Uuid() const
{
    return EDefinition::Uuid();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialDef)