#ifndef ECAD_EPADSTACKDEF_H
#define ECAD_EPADSTACKDEF_H
#include "interfaces/IPadstackDef.h"
#include "EDefinition.h"
#include "ECadDef.h"
namespace ecad {

class IPadstackDefData;
class ECAD_API EPadstackDef : public EDefinition, public IPadstackDef
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPadstackDef();
    EPadstackDef(std::string name);
    virtual ~EPadstackDef();

    ///Copy
    EPadstackDef(const EPadstackDef & other);
    EPadstackDef & operator= (const EPadstackDef & other);

    void SetPadstackDefData(UPtr<IPadstackDefData> data);
    Ptr<IPadstackDefData> GetPadstackDefData() const;

    EDefinitionType GetDefinitionType() const;
    const std::string & GetName() const;

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

#ifdef ECAD_HEADER_ONLY
#include "EPadstackDef.cpp"
#endif

#endif//ECAD_EPADSTACKDEF_H