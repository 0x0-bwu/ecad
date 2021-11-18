#ifndef ECAD_HEADER_ONLY
#include "ECollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECollection)
#endif

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ECollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECollection, ICollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("type", m_type);
}

template <typename Archive>
ECAD_INLINE void ECollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECollection, ICollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("type", m_type);
}
    
ECAD_SERIALIZATION_FUNCTIONS_IMP(ECollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ECollection::ECollection()
 : ECollection(std::string{})
{ 
}

ECAD_INLINE ECollection::ECollection(std::string name)
 : EObject(std::move(name))
 , m_type(ECollectionType::Invalid)
{
}

ECAD_INLINE ECollection::~ECollection()
{
}

ECAD_INLINE ECollection::ECollection(const ECollection & other)
{
    *this = other;
}

ECAD_INLINE ECollection & ECollection::operator= (const ECollection & other)
{
    EObject::operator=(other);
    m_type = other.m_type;
    return *this;
}

ECAD_INLINE size_t ECollection::Size() const
{
    return 0;
}
}//namespace ecad