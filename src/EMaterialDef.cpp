#ifndef ECAD_HEADER_ONLY
#include "EMaterialDef.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialDef)
#endif

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EMaterialDef::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialDef, IMaterialDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("properties", m_properties);
}

template <typename Archive>
ECAD_INLINE void EMaterialDef::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialDef, IMaterialDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("properties", m_properties);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EMaterialDef)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EMaterialDef::EMaterialDef()
 : EMaterialDef(std::string{})
{
}

ECAD_INLINE EMaterialDef::EMaterialDef(std::string name)
 : EDefinition(std::move(name))
{
}

ECAD_INLINE EMaterialDef::~EMaterialDef()
{
}

ECAD_INLINE EMaterialDef::EMaterialDef(const EMaterialDef & other)
{
    *this = other;
}

ECAD_INLINE EMaterialDef & EMaterialDef::operator= (const EMaterialDef & other)
{
    EDefinition::operator=(other);
    m_properties.clear();
    for(const auto & property : other.m_properties){
        m_properties.insert(std::make_pair(property.first, CloneHelper(property.second)));
    }
    return *this;
}

ECAD_INLINE bool EMaterialDef::hasProperty(EMaterialPropId id) const
{
    return m_properties.count(id);
}

ECAD_INLINE void EMaterialDef::SetProperty(EMaterialPropId id, UPtr<IMaterialProp> prop)
{
    m_properties.insert(std::make_pair(id, std::move(prop)));
}

ECAD_INLINE CPtr<IMaterialProp> EMaterialDef::GetProperty(EMaterialPropId id) const
{
    auto iter = m_properties.find(id);
    if(iter == m_properties.end()) return nullptr;
    else return iter->second.get();
}

}//namespace ecad