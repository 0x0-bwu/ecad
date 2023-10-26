#include "EComponent.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponent)

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EComponent::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponent, IComponent>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("location", m_location);
    ar & boost::serialization::make_nvp("mount_layer", m_mountLyr);
}

template <typename Archive>
ECAD_INLINE void EComponent::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponent, IComponent>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("location", m_location);
    ar & boost::serialization::make_nvp("mount_layer", m_mountLyr);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponent)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EComponent::EComponent()
 : EComponent(std::string{}, nullptr)
{
}

ECAD_INLINE EComponent::EComponent(std::string name, CPtr<IComponentDef> compDef)
 : EObject(std::move(name)), m_compDef(compDef)
{
}

ECAD_INLINE EComponent::~EComponent()
{
}
    
ECAD_INLINE EComponent::EComponent(const EComponent & other)
{
    *this = other;
}

ECAD_INLINE EComponent & EComponent::operator= (const EComponent & other)
{
    EObject::operator==(other);
    m_compDef = other.m_compDef;
    m_location = other.m_location;
    m_mountLyr = other.m_mountLyr;
    return *this;
}

}//namespace ecad