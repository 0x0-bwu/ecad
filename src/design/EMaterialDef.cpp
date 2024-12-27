#include "EMaterialDef.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialDef)

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void EMaterialDef::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialDef, IMaterialDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("id", m_id);
    ar & boost::serialization::make_nvp("type", m_type);
    ar & boost::serialization::make_nvp("properties", m_properties);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EMaterialDef)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EMaterialDef::EMaterialDef()
 : EMaterialDef(std::string{}, nullptr, EMaterialId::noMaterial)
{
}

EMaterialDef::EMaterialDef(std::string name, CPtr<IDatabase> database, EMaterialId id)
 : EDefinition(std::move(name), database), m_id(id)
{
}

EMaterialDef::~EMaterialDef()
{
}

EMaterialDef::EMaterialDef(const EMaterialDef & other)
{
    *this = other;
}

EMaterialDef & EMaterialDef::operator= (const EMaterialDef & other)
{
    EDefinition::operator=(other);
    m_id = other.m_id;
    m_type = other.m_type;
    m_properties.clear();
    for(const auto & property : other.m_properties){
        m_properties.insert(std::make_pair(property.first, CloneHelper(property.second)));
    }
    return *this;
}

void EMaterialDef::SetDatabase(CPtr<IDatabase> database)
{
    EDefinition::SetDatabase(database);
}

EMaterialId EMaterialDef::GetMaterialId() const
{
    return m_id;
}

bool EMaterialDef::hasProperty(EMaterialPropId id) const
{
    return m_properties.count(id);
}

void EMaterialDef::SetProperty(EMaterialPropId id, UPtr<IMaterialProp> prop)
{
    m_properties.emplace(std::make_pair(id, std::move(prop)));
}

CPtr<IMaterialProp> EMaterialDef::GetProperty(EMaterialPropId id) const
{
    auto iter = m_properties.find(id);
    if (iter == m_properties.cend()) return nullptr;
    else return iter->second.get();
}

void EMaterialDef::SetMaterialType(EMaterialType type)
{
    m_type = type;
}

EMaterialType EMaterialDef::GetMaterialType() const
{
    return m_type;
}

EDefinitionType EMaterialDef::GetDefinitionType() const
{
    return EDefinitionType::MaterialDef;
}

}//namespace ecad