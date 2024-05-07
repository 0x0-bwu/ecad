#include "ELayerMapCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ELayerMapCollection)

#include "interface/IDefinitionCollection.h"
#include "interface/IDefinition.h"
#include "interface/ILayerMap.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void ELayerMapCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayerMapCollection, ILayerMapCollection>();
    boost::serialization::void_cast_register<ELayerMapCollection, IDefinitionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ELayerMapCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ELayerMapCollection::ELayerMapCollection()
{
    m_type = ECollectionType::LayerMap;
}

ECAD_INLINE ELayerMapCollection::~ELayerMapCollection()
{
}

ECAD_INLINE ELayerMapCollection::ELayerMapCollection(const ELayerMapCollection & other)
{
    *this = other;
}
    
ECAD_INLINE ELayerMapCollection & ELayerMapCollection::operator= (const ELayerMapCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<IDefinitionCollection> ELayerMapCollection::AddDefinitionCollection(EDefinitionType type)
{
    return nullptr;
}

ECAD_INLINE Ptr<IDefinitionCollection> ELayerMapCollection::GetDefinitionCollection(EDefinitionType type) const
{
    if(type == EDefinitionType::LayerMap) return const_cast<Ptr<ELayerMapCollection> >(this);
    return nullptr;
}

ECAD_INLINE Ptr<IDefinition> ELayerMapCollection::AddDefinition(const std::string & name, UPtr<IDefinition> definition)
{
    auto type = definition->GetDefinitionType();
    auto layerMap = dynamic_cast<Ptr<ILayerMap> >(definition.get());
    if(nullptr == layerMap) return nullptr;
    Insert(name, UPtr<ILayerMap>(layerMap));
    definition.release();
    return GetDefinition(name, type);
}

ECAD_INLINE Ptr<IDefinition> ELayerMapCollection::GetDefinition(const std::string & name, EDefinitionType type) const
{
    if(type == EDefinitionType::LayerMap && BaseCollection::Count(name))
        return dynamic_cast<Ptr<IDefinition> >(BaseCollection::At(name).get());
    return nullptr;
}

ECAD_INLINE std::string ELayerMapCollection::GetNextDefName(const std::string & base, EDefinitionType type) const
{
    if(type == EDefinitionType::LayerMap) return NextKey(*this, base);
    return std::string{};
}

ECAD_INLINE LayerMapIter ELayerMapCollection::GetLayerMapIter() const
{
    return LayerMapIter(new ELayerMapIterator(*this));
}

ECAD_INLINE size_t ELayerMapCollection::Size() const
{
    return BaseCollection::Size();
}

ECAD_INLINE void ELayerMapCollection::Clear()
{
    BaseCollection::Clear();
}

ECAD_INLINE void ELayerMapCollection::PrintImp(std::ostream & os) const
{
    BaseCollection::PrintImp(os);
    for (const auto & [name, def] : m_collection) {
        ECAD_UNUSED(def)
        os << "NAME: " << name << ECAD_EOL;
    }
}

}//namesapce ecad