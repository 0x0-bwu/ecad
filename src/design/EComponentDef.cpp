#include "EComponentDef.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentDef)

#include "interface/IComponentDefPinCollection.h"
#include "generic/geometry/GeometryIO.hpp"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EComponentDef::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDef, IComponentDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("type", m_type);
    ar & boost::serialization::make_nvp("boundary", m_boundary);
    ar & boost::serialization::make_nvp("height", m_height);
    ar & boost::serialization::make_nvp("solder_height", m_solderHeight);
    ar & boost::serialization::make_nvp("material", m_material);
    ar & boost::serialization::make_nvp("solder_filling_material", m_solderFillingMaterial);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponentDef)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EComponentDef::EComponentDef()
 : EComponentDef(std::string{}, nullptr)
{
}

ECAD_INLINE EComponentDef::EComponentDef(std::string name, CPtr<IDatabase> database)
 : EDefinition(std::move(name), database)
{
    ECollectionCollection::Init();
}

ECAD_INLINE EComponentDef::EComponentDef(const EComponentDef & other)
{
    *this = other;
}

ECAD_INLINE EComponentDef & EComponentDef::operator= (const EComponentDef & other)
{
    EDefinition::operator=(other);
    ECollectionCollection::operator=(other);
    
    m_type = other.m_type;
    m_boundary = CloneHelper(other.m_boundary);
    m_height = other.m_height;
    m_solderHeight = other.m_solderHeight;
    m_material = other.m_material;
    m_solderFillingMaterial = other.m_solderFillingMaterial;
    return *this;
}

ECAD_INLINE EComponentDef::~EComponentDef()
{
}

ECAD_INLINE void EComponentDef::SetDatabase(CPtr<IDatabase> database)
{
    return EDefinition::SetDatabase(database);
}

ECAD_INLINE CPtr<IDatabase> EComponentDef::GetDatabase() const
{
    return EDefinition::GetDatabase();
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

ECAD_INLINE void EComponentDef::SetBoundary(UPtr<EShape> boundary)
{
    m_boundary = std::move(boundary);
}

ECAD_INLINE CPtr<EShape> EComponentDef::GetBoundary() const
{
    return m_boundary.get();
}

ECAD_INLINE void EComponentDef::SetMaterial(const std::string & name)
{
    m_material = name;
}

ECAD_INLINE const std::string & EComponentDef::GetMaterial() const
{
    return m_material;
}

ECAD_INLINE void EComponentDef::SetHeight(EFloat height)
{
    m_height = height;
}

ECAD_INLINE EFloat EComponentDef::GetHeight() const
{
    return m_height;
}

ECAD_INLINE void EComponentDef::SetSolderBallBumpHeight(EFloat height)
{
    m_solderHeight = height;
}

ECAD_INLINE EFloat EComponentDef::GetSolderBallBumpHeight() const
{
    return m_solderHeight;
}

ECAD_INLINE void EComponentDef::SetSolderFillingMaterial(const std::string & name)
{
    m_solderFillingMaterial = name;
}

ECAD_INLINE const std::string & EComponentDef::GetSolderFillingMaterial() const
{
    return m_solderFillingMaterial;
}

ECAD_INLINE Ptr<IComponentDefPin> EComponentDef::CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef, ELayerId lyr)
{
    return ComponentDefPinCollection()->CreatePin(name, loc, type, psDef, lyr);
}

ECAD_INLINE Ptr<IComponentDefPin> EComponentDef::FindPinByName(const std::string & name) const
{
    return ComponentDefPinCollection()->FindPinByName(name);
}

ECAD_INLINE void EComponentDef::PrintImp(std::ostream & os) const
{
    os << "COMPONENT DEFINE: " << m_name << ECAD_EOL;
    os << "TYPE: " << toString(m_type) << ECAD_EOL;
    os << "HEIGHT: " << m_height << ECAD_EOL;
    os << "MATERIAL: " << m_material << ECAD_EOL; 
    if (m_boundary)
        os << "BBOX: " << m_boundary->GetBBox() << ECAD_EOL;
}



}//namespace ecad