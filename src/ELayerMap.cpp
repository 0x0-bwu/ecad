#ifndef ECAD_HEADER_ONLY
#include "ELayerMap.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ELayerMap)
#endif

#include "interfaces/IDatabase.h"
#include "ECadDef.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ELayerMap::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayerMap, ILayerMap>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("layer_map", m_layerIdMap);
    ar & boost::serialization::make_nvp("database", m_database);
}

template <typename Archive>
ECAD_INLINE void ELayerMap::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayerMap, ILayerMap>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("layer_map", m_layerIdMap);
    ar & boost::serialization::make_nvp("database", m_database);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ELayerMap)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ELayerMap::ELayerMap()
 : ELayerMap(std::string{}, nullptr)
{
}

ECAD_INLINE ELayerMap::ELayerMap(const std::string & name, Ptr<IDatabase> database)
 : EDefinition(name)
 , m_database(database)
{
    SetMapping(noLayer, noLayer);
}

ECAD_INLINE ELayerMap::~ELayerMap()
{
}
    
ECAD_INLINE ELayerMap::ELayerMap(const ELayerMap & other)
{
    *this = other;
}

ECAD_INLINE ELayerMap & ELayerMap::operator= (const ELayerMap & other)
{
    EDefinition::operator=(other);
    m_layerIdMap = other.m_layerIdMap;
    m_database = other.m_database;
    return *this;
}

ECAD_INLINE Ptr<IDatabase> ELayerMap::GetDatabase() const
{
    return m_database;
}

ECAD_INLINE EDefinitionType ELayerMap::GetDefinitionType() const
{
    return EDefinitionType::LayerMap;
}

ECAD_INLINE void ELayerMap::SetMapping(ELayerId from, ELayerId to)
{
    m_layerIdMap.insert(LayerIdMap::value_type(from, to));
}

ECAD_INLINE void ELayerMap::MappingLeft(CPtr<ILayerMap> layerMap)
{
    LayerIdMap layerIdMap;
    auto iter = m_layerIdMap.left.begin();
    for(; iter != m_layerIdMap.left.end(); ++iter)
        layerIdMap.insert(LayerIdMap::value_type(layerMap->GetMappingForward(iter->first), iter->second));
    m_layerIdMap = std::move(layerIdMap);
}

ECAD_INLINE void ELayerMap::MappingRight(CPtr<ILayerMap> layerMap)
{
    LayerIdMap layerIdMap;
    auto iter = m_layerIdMap.left.begin();
    for(; iter != m_layerIdMap.left.end(); ++iter)
        layerIdMap.insert(LayerIdMap::value_type(iter->first, layerMap->GetMappingForward(iter->second)));
    m_layerIdMap = std::move(layerIdMap);
}

ECAD_INLINE ELayerId ELayerMap::GetMappingForward(ELayerId from) const
{
    if(!m_layerIdMap.left.count(from)) return noLayer;
    return m_layerIdMap.left.at(from);
}

ECAD_INLINE ELayerId ELayerMap::GetMappingBackward(ELayerId to) const
{
    if(!m_layerIdMap.right.count(to)) return noLayer;
    return m_layerIdMap.right.at(to);
}

}//namespace ecad