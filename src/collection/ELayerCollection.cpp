#include "ELayerCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ELayerCollection)

#include "interface/ILayer.h"
#include "design/ELayerMap.h"
#include "EDataMgr.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void ELayerCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayerCollection, ILayerCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ELayerCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ELayerCollection::ELayerCollection()
{
}

ELayerCollection::~ELayerCollection()
{
}

ELayerCollection::ELayerCollection(const ELayerCollection & other)
{
    *this = other;
}

ELayerCollection & ELayerCollection::operator= (const ELayerCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ELayerId ELayerCollection::AppendLayer(UPtr<ILayer> layer)
{
    auto id = static_cast<ELayerId>(Size());
    layer->SetLayerId(id);
    Append(std::move(layer));
    return id;
}

std::vector<ELayerId> ELayerCollection::AppendLayers(std::vector<UPtr<ILayer> > layers)
{
    std::vector<ELayerId> ids(layers.size());
    for(size_t i = 0; i < layers.size(); ++i)
        ids[i] = AppendLayer(std::move(layers[i]));
    return ids;
}

UPtr<ILayerMap> ELayerCollection::AddDefaultDielectricLayers()
{
    auto curr = m_collection.begin();
    auto next = curr; next++;
    while(next != m_collection.end()){
        if((*curr)->GetLayerType() == ELayerType::ConductingLayer && 
           (*next)->GetLayerType() == ELayerType::ConductingLayer){
            auto dielectric = EDataMgr::Instance().CreateStackupLayer(GetNextLayerName("Dielectric"), ELayerType::DielectricLayer, 0, 0);
            curr = m_collection.insert(next, std::move(dielectric));
            next = curr; next++;
        }
        curr++; next++;
    }
    return MakeLayerIdOrdered();
}

UPtr<ILayerMap> ELayerCollection::GetDefaultLayerMap() const
{
    auto lyrMap = UPtr<ILayerMap>(new ELayerMap);
    for(const auto & layer : m_collection){
        auto id = layer->GetLayerId();
        lyrMap->SetMapping(id, id);
    }
    return lyrMap;
}

void ELayerCollection::GetStackupLayers(std::vector<Ptr<IStackupLayer> > & layers) const
{
    layers.clear();
    for(const auto & layer : m_collection) {
        auto stackupLayer = layer->GetStackupLayerFromLayer();
        if (stackupLayer) layers.emplace_back(stackupLayer);
    }
}

void ELayerCollection::GetStackupLayers(std::vector<CPtr<IStackupLayer> > & layers) const
{
    layers.clear();
    for(const auto & layer : m_collection) {
        auto stackupLayer = layer->GetStackupLayerFromLayer();
        if (stackupLayer) layers.push_back(stackupLayer);
    }
}

LayerIter ELayerCollection::GetLayerIter() const
{
    return LayerIter(new ELayerIterator(*this));
}

size_t ELayerCollection::Size() const
{
    return BaseCollection::Size();
}

Ptr<ILayer> ELayerCollection::FindLayerByLayerId(ELayerId lyrId) const
{
    size_t index = static_cast<size_t>(lyrId);
    if(index >= Size()) return nullptr;
    return At(index).get();
}

Ptr<ILayer> ELayerCollection::FindLayerByName(const std::string & name) const
{
    for(const auto & layer : m_collection){
        if(name == layer->GetName())
            return layer.get();
    }
    return nullptr;
}

std::string ELayerCollection::GetNextLayerName(const std::string & base) const
{
    if(nullptr == FindLayerByName(base)) return base;
    size_t index = 1;
    while(true){
        std::string name = base + std::to_string(index);
        if(nullptr == FindLayerByName(name)) return name;
        index++;
    }
    return "";
}

Ptr<ELayerCollection> ELayerCollection::CloneImp() const
{
    return new ELayerCollection(*this);
}

UPtr<ILayerMap> ELayerCollection::MakeLayerIdOrdered()
{
    auto lyrMap = UPtr<ILayerMap>(new ELayerMap);
    for(size_t i = 0; i < m_collection.size(); ++i){
        auto origin = m_collection[i]->GetLayerId();
        auto ordered  = static_cast<ELayerId>(i);
        m_collection[i]->SetLayerId(ordered);
        if(origin != ELayerId::noLayer){
            lyrMap->SetMapping(origin, ordered);
        }
    }
    return lyrMap;
}

}//namespace ecad