#ifndef ECAD_ECOMPONENTDEF_HPP
#define ECAD_ECOMPONENTDEF_HPP
#include "interfaces/IComponentDef.h"
#include "ECollectionCollection.h"
#include "EDefinition.h"
namespace ecad {

class ECAD_API EComponentDef : public EDefinition, public ECollectionCollection, public IComponentDef
{
    ECAD_ALWAYS_INLINE static constexpr std::array<ECollectionType, 1> m_collectionTypes = { ECollectionType::ComponentDefPin };

    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EComponentDef();
public:
    EComponentDef(const std::string & name);
    virtual ~EComponentDef();

    ///Copy
    EComponentDef(const EComponentDef & other);
    EComponentDef & operator= (const EComponentDef & other);

    EDefinitionType GetDefinitionType() const;

    const std::string & GetName() const;

protected:
    ///Copy
    virtual Ptr<EComponentDef> CloneImp() const override { return new EComponentDef(*this); }

protected:
    EComponentType m_type = EComponentType::Invalid;
};

ECAD_ALWAYS_INLINE const std::string & EComponentDef::GetName() const
{
    return EDefinition::GetName();
}

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponentDef)

#ifdef ECAD_HEADER_ONLY
#include "EComponentDef.cpp"
#endif

#endif//ECAD_ECOMPONENTDEF_HPP