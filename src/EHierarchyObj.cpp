#include "EHierarchyObj.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EHierarchyObj)

#include "interfaces/ILayer.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EHierarchyObj::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EHierarchyObj, IHierarchyObj>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("transform", m_transform);
}

template <typename Archive>
ECAD_INLINE void EHierarchyObj::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EHierarchyObj, IHierarchyObj>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("transform", m_transform);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EHierarchyObj)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EHierarchyObj::EHierarchyObj()
 : EHierarchyObj(std::string{})
{
}

ECAD_INLINE EHierarchyObj::EHierarchyObj(std::string name)
 : EObject(name)
{
}

ECAD_INLINE EHierarchyObj::~EHierarchyObj()
{
}

ECAD_INLINE EHierarchyObj::EHierarchyObj(const EHierarchyObj & other)
{
    *this = other;
}

ECAD_INLINE EHierarchyObj & EHierarchyObj::operator= (const EHierarchyObj & other)
{
    EObject::operator=(other);
    m_transform = other.m_transform;
    return *this;
}
}//namespace ecad