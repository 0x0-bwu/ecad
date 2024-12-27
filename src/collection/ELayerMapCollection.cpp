#include "ELayerMapCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ELayerMapCollection)

#include "interface/IDefinitionCollection.h"
#include "interface/IDefinition.h"
#include "interface/ILayerMap.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void ELayerMapCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayerMapCollection, ILayerMapCollection>();
    boost::serialization::void_cast_register<ELayerMapCollection, IDefinitionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ELayerMapCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ELayerMapCollection::ELayerMapCollection()
{
}

ELayerMapCollection::~ELayerMapCollection()
{
}

ELayerMapCollection::ELayerMapCollection(const ELayerMapCollection & other)
{
    *this = other;
}
    
ELayerMapCollection & ELayerMapCollection::operator= (const ELayerMapCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

Ptr<IDefinition> ELayerMapCollection::AddDefinition(const std::string & name, UPtr<IDefinition> definition)
{
    auto type = definition->GetDefinitionType();
    auto layerMap = dynamic_cast<Ptr<ILayerMap> >(definition.get());
    if(nullptr == layerMap) return nullptr;
    Insert(name, UPtr<ILayerMap>(layerMap));
    definition.release();
    return GetDefinition(name, type);
}

Ptr<IDefinition> ELayerMapCollection::GetDefinition(const std::string & name, EDefinitionType type) const
{
    if(type == EDefinitionType::LayerMap && BaseCollection::Count(name))
        return dynamic_cast<Ptr<IDefinition> >(BaseCollection::At(name).get());
    return nullptr;
}

std::string ELayerMapCollection::GetNextDefName(const std::string & base, EDefinitionType type) const
{
    if(type == EDefinitionType::LayerMap) return NextKey(*this, base);
    return std::string{};
}

void ELayerMapCollection::SetDatabase(CPtr<IDatabase> database)
{
    auto lmIter = GetLayerMapIter();
    while (auto lm = lmIter->Next()) {
        lm->SetDatabase(database);
    }
}

LayerMapIter ELayerMapCollection::GetLayerMapIter() const
{
    return LayerMapIter(new ELayerMapIterator(*this));
}

size_t ELayerMapCollection::Size() const
{
    return BaseCollection::Size();
}

void ELayerMapCollection::Clear()
{
    BaseCollection::Clear();
}

void ELayerMapCollection::PrintImp(std::ostream & os) const
{
    BaseCollection::PrintImp(os);
    for (const auto & [name, def] : m_collection) {
        ECAD_UNUSED(def)
        os << "NAME: " << name << ECAD_EOL;
    }
}

}//namespace ecad