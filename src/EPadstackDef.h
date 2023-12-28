#pragma once
#include "interfaces/IPadstackDef.h"
#include "EDefinition.h"
#include "ECadDef.h"
namespace ecad {

class IPadstackDefData;
class ECAD_API EPadstackDef : public EDefinition, public IPadstackDef
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EPadstackDef();
public:
    EPadstackDef(std::string name, CPtr<IDatabase> database);
    virtual ~EPadstackDef();

    ///Copy
    EPadstackDef(const EPadstackDef & other);
    EPadstackDef & operator= (const EPadstackDef & other);

    void SetPadstackDefData(UPtr<IPadstackDefData> data) override;
    Ptr<IPadstackDefData> GetPadstackDefData() const override;

    EDefinitionType GetDefinitionType() const override;
    const std::string & GetName() const override;

protected:
    ///Copy
    virtual Ptr<EPadstackDef> CloneImp() const override { return new EPadstackDef(*this); }

protected:
    UPtr<IPadstackDefData> m_data;
};

ECAD_ALWAYS_INLINE const std::string & EPadstackDef::GetName() const
{
    return EDefinition::GetName();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPadstackDef)