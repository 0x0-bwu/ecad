#include "EObject.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EObject)

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void EObject::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ar & boost::serialization::make_nvp("name", m_name);
#ifdef ECAD_BOOST_SERIALIZATION_INCLUDE_UUID
    ar & boost::serialization::make_nvp("uuid", m_uuid);
#endif//ECAD_BOOST_SERIALIZATION_INCLUDE_UUID
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EObject)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EObject::EObject()
 : EObject(std::string{})
{
}

ECAD_INLINE EObject::EObject(std::string name)
 : m_name(std::move(name))
 , m_uuid(boost::uuids::random_generator()())
{
}


ECAD_INLINE EObject::~EObject()
{
}

ECAD_INLINE EObject::EObject(const EObject & other)
{
    *this = other;
}

ECAD_INLINE EObject & EObject::operator= (const EObject & other)
{
    m_name = other.m_name;
    return *this;
}

ECAD_INLINE EObject::EObject(EObject && other)
 : m_uuid(std::move(other.m_uuid))
{
}

ECAD_INLINE EObject & EObject::operator= (EObject && other)
{
    m_uuid = std::move(other.m_uuid);
    return *this;
}

ECAD_INLINE bool EObject::operator== (const EObject & other) const
{
    return m_uuid == other.m_uuid;
}

ECAD_INLINE bool EObject::operator!= (const EObject & other) const
{
    return !(*this == other);
}

}//namespace ecad