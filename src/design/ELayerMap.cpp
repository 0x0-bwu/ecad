#include "ELayerMap.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ELayerMap)

#include "interface/IDatabase.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void ELayerMap::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayerMap, ILayerMap>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("layer_map", m_layerIdMap);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ELayerMap)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ELayerMap::ELayerMap()
 : ELayerMap(std::string{}, nullptr)
{
}

ELayerMap::ELayerMap(std::string name, CPtr<IDatabase> database)
 : EDefinition(std::move(name), database)
{
    SetMapping(ELayerId::noLayer, ELayerId::noLayer);
    SetMapping(ELayerId::ComponentLayer, ELayerId::ComponentLayer);
}

ELayerMap::~ELayerMap()
{
}

void ELayerMap::SetDatabase(CPtr<IDatabase> database)
{
    return EDefinition::SetDatabase(database);
}

CPtr<IDatabase> ELayerMap::GetDatabase() const
{
    return EDefinition::GetDatabase();
}

EDefinitionType ELayerMap::GetDefinitionType() const
{
    return EDefinitionType::LayerMap;
}

void ELayerMap::SetMapping(ELayerId from, ELayerId to)
{
    m_layerIdMap.insert(LayerIdMap::value_type(from, to));
}

void ELayerMap::MappingLeft(CPtr<ILayerMap> layerMap)
{
    LayerIdMap layerIdMap;
    auto iter = m_layerIdMap.left.begin();
    for(; iter != m_layerIdMap.left.end(); ++iter)
        layerIdMap.insert(LayerIdMap::value_type(layerMap->GetMappingForward(iter->first), iter->second));
    m_layerIdMap = std::move(layerIdMap);
}

void ELayerMap::MappingRight(CPtr<ILayerMap> layerMap)
{
    LayerIdMap layerIdMap;
    auto iter = m_layerIdMap.left.begin();
    for(; iter != m_layerIdMap.left.end(); ++iter)
        layerIdMap.insert(LayerIdMap::value_type(iter->first, layerMap->GetMappingForward(iter->second)));
    m_layerIdMap = std::move(layerIdMap);
}

ELayerId ELayerMap::GetMappingForward(ELayerId from) const
{
    if(!m_layerIdMap.left.count(from)) return noLayer;
    return m_layerIdMap.left.at(from);
}

ELayerId ELayerMap::GetMappingBackward(ELayerId to) const
{
    if(!m_layerIdMap.right.count(to)) return noLayer;
    return m_layerIdMap.right.at(to);
}

}//namespace ecad