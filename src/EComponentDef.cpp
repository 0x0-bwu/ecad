#ifndef ECAD_HEADER_ONLY
#include "EComponentDef.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentDef)
#endif

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EComponentDef::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDef, IComponentDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("type", m_type);
}

template <typename Archive>
ECAD_INLINE void EComponentDef::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDef, IComponentDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("type", m_type);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponentDef)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EComponentDef::EComponentDef()
 : EComponentDef(std::string{})
{
}

ECAD_INLINE EComponentDef::EComponentDef(const std::string & name)
 : EDefinition(name)
{
    for(auto type : m_collectionTypes) 
        AddCollection(type);
}

ECAD_INLINE EComponentDef::~EComponentDef()
{
}
    
ECAD_INLINE EComponentDef::EComponentDef(const EComponentDef & other)
{
    *this = other;
}

ECAD_INLINE EComponentDef & EComponentDef::operator= (const EComponentDef & other)
{
    EDefinition::operator=(other);
    return *this;
}

ECAD_INLINE EDefinitionType EComponentDef::GetDefinitionType() const
{
    return EDefinitionType::ComponentDef;
}

}//namespace ecad