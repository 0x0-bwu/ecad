#include "ELayerMap.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ELayerMap)

#include "interface/IDatabase.h"
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

ECAD_INLINE ELayerMap::ELayerMap(std::string name, CPtr<IDatabase> database)
 : EDefinition(std::move(name), database)
{
    SetMapping(ELayerId::noLayer, ELayerId::noLayer);
    SetMapping(ELayerId::ComponentLayer, ELayerId::ComponentLayer);
}

ECAD_INLINE ELayerMap::~ELayerMap()
{
}

ECAD_INLINE CPtr<IDatabase> ELayerMap::GetDatabase() const
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