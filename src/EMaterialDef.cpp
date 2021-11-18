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
}

template <typename Archive>
ECAD_INLINE void EMaterialDef::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialDef, IMaterialDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
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
    return *this;
}

}//namespace ecad