#ifndef ECAD_HEADER_ONLY
#include "EComponentDefPin.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentDefPin)
#endif

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EComponentDefPin::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDefPin, IComponentDefPin>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
}

template <typename Archive>
ECAD_INLINE void EComponentDefPin::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDefPin, IComponentDefPin>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponentDefPin)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EComponentDefPin::EComponentDefPin()
 : EObject(std::string{})
{
}

ECAD_INLINE EComponentDefPin::EComponentDefPin(const std::string & name)
 : EObject(name)
{
}

ECAD_INLINE EComponentDefPin::~EComponentDefPin()
{
}
    
ECAD_INLINE EComponentDefPin::EComponentDefPin(const EComponentDefPin & other)
{
    *this = other;
}

ECAD_INLINE EComponentDefPin & EComponentDefPin::operator= (const EComponentDefPin & other)
{
    EObject::operator=(other);
    return *this;
}

}//namespace ecad