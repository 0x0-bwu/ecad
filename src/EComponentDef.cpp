#include "EComponentDef.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentDef)

#include "interfaces/IComponentDefPinCollection.h"
#include "generic/geometry/GeometryIO.hpp"

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
    ar & boost::serialization::make_nvp("bounding_box", m_bondingBox);
    ar & boost::serialization::make_nvp("height", m_height);
    ar & boost::serialization::make_nvp("material", m_material);
}

template <typename Archive>
ECAD_INLINE void EComponentDef::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDef, IComponentDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("type", m_type);
    ar & boost::serialization::make_nvp("bounding_box", m_bondingBox);
    ar & boost::serialization::make_nvp("height", m_height);
    ar & boost::serialization::make_nvp("material", m_material);
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
    m_type = other.m_type;
    m_bondingBox = other.m_bondingBox;
    m_height = other.m_height;
    m_material = other.m_material;
    return *this;
}

ECAD_INLINE EDefinitionType EComponentDef::GetDefinitionType() const
{
    return EDefinitionType::ComponentDef;
}

ECAD_INLINE void EComponentDef::SetComponentType(EComponentType type)
{
    m_type = type;
}
    
ECAD_INLINE EComponentType EComponentDef::GetComponentType() const
{
    return m_type;
}

ECAD_INLINE void EComponentDef::SetBondingBox(const EBox2D & bbox)
{
    m_bondingBox = bbox;
}

ECAD_INLINE const EBox2D & EComponentDef::GetBondingBox() const
{
    return m_bondingBox;
}

ECAD_INLINE void EComponentDef::SetMaterial(const std::string & name)
{
    m_material = name;
}

ECAD_INLINE const std::string & EComponentDef::GetMaterial() const
{
    return m_material;
}

ECAD_INLINE void EComponentDef::SetHeight(FCoord height)
{
    m_height = height;
}

ECAD_INLINE FCoord EComponentDef::GetHeight() const
{
    return m_height;
}

ECAD_INLINE Ptr<IComponentDefPin> EComponentDef::CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef, ELayerId lyr)
{
    return ComponentDefPinCollection()->CreatePin(name, loc, type, psDef, lyr);
}

ECAD_INLINE void EComponentDef::PrintImp(std::ostream & os) const
{
    os << "COMPONENT DEFINE: " << m_name << ECAD_EOL;
    os << "TYPE: " << toString(m_type) << ECAD_EOL;
    os << "BBOX: " << m_bondingBox << ECAD_EOL;
    os << "HEIGHT: " << m_height << ECAD_EOL;
    os << "MATERIAL: " << m_material << ECAD_EOL; 
}



}//namespace ecad