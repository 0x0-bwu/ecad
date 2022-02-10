#ifndef ECAD_EMATERIALDEF_H
#define ECAD_EMATERIALDEF_H
#include "interfaces/IMaterialDef.h"
#include "EDefinition.h"
namespace ecad {

class ECAD_API EMaterialDef : public EDefinition, public IMaterialDef
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EMaterialDef();
public:
    EMaterialDef(std::string name);
    virtual ~EMaterialDef();

    ///Copy
    EMaterialDef(const EMaterialDef & other);
    EMaterialDef & operator= (const EMaterialDef & other);

    const std::string & GetName() const;
    const EUuid & Uuid() const;

protected:
    ///Copy
    virtual Ptr<EMaterialDef> CloneImp() const override { return new EMaterialDef(*this); }
private:
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

#ifdef ECAD_HEADER_ONLY
#include "EMaterialDef.cpp"
#endif

#endif//ECAD_EMATERIALDEF_H