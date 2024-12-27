#include "EComponentDef.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentDef)

#include "interface/IComponentDefPinCollection.h"
#include "generic/geometry/GeometryIO.hpp"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void EComponentDef::serialize(Archive & ar, const unsigned int version)
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

EComponentDef::EComponentDef()
 : EComponentDef(std::string{}, nullptr)
{
}

EComponentDef::EComponentDef(std::string name, CPtr<IDatabase> database)
 : EDefinition(std::move(name), database)
{
    ECollectionCollection::Init();
}

EComponentDef::EComponentDef(const EComponentDef & other)
{
    *this = other;
}

EComponentDef & EComponentDef::operator= (const EComponentDef & other)
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

EComponentDef::~EComponentDef()
{
}

void EComponentDef::SetDatabase(CPtr<IDatabase> database)
{
    return EDefinition::SetDatabase(database);
}

CPtr<IDatabase> EComponentDef::GetDatabase() const
{
    return EDefinition::GetDatabase();
}

EDefinitionType EComponentDef::GetDefinitionType() const
{
    return EDefinitionType::ComponentDef;
}

void EComponentDef::SetComponentType(EComponentType type)
{
    m_type = type;
}
    
EComponentType EComponentDef::GetComponentType() const
{
    return m_type;
}

void EComponentDef::SetBoundary(UPtr<EShape> boundary)
{
    m_boundary = std::move(boundary);
}

CPtr<EShape> EComponentDef::GetBoundary() const
{
    return m_boundary.get();
}

void EComponentDef::SetMaterial(const std::string & name)
{
    m_material = name;
}

const std::string & EComponentDef::GetMaterial() const
{
    return m_material;
}

void EComponentDef::SetHeight(EFloat height)
{
    m_height = height;
}

EFloat EComponentDef::GetHeight() const
{
    return m_height;
}

void EComponentDef::SetSolderBallBumpHeight(EFloat height)
{
    m_solderHeight = height;
}

EFloat EComponentDef::GetSolderBallBumpHeight() const
{
    return m_solderHeight;
}

void EComponentDef::SetSolderFillingMaterial(const std::string & name)
{
    m_solderFillingMaterial = name;
}

const std::string & EComponentDef::GetSolderFillingMaterial() const
{
    return m_solderFillingMaterial;
}

Ptr<IComponentDefPin> EComponentDef::CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef, ELayerId lyr)
{
    return ComponentDefPinCollection()->CreatePin(name, loc, type, psDef, lyr);
}

Ptr<IComponentDefPin> EComponentDef::FindPinByName(const std::string & name) const
{
    return ComponentDefPinCollection()->FindPinByName(name);
}

void EComponentDef::PrintImp(std::ostream & os) const
{
    os << "COMPONENT DEFINE: " << m_name << ECAD_EOL;
    os << "TYPE: " << toString(m_type) << ECAD_EOL;
    os << "HEIGHT: " << m_height << ECAD_EOL;
    os << "MATERIAL: " << m_material << ECAD_EOL; 
    if (m_boundary)
        os << "BBOX: " << m_boundary->GetBBox() << ECAD_EOL;
}



}//namespace ecad