#include "ECollectionCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECollectionCollection)

#include "EComponentDefPinCollection.h"
#include "EPadstackInstCollection.h"
#include "EHierarchyObjCollection.h"
#include "EComponentDefCollection.h"
#include "EPadstackDefCollection.h"
#include "EMaterialDefCollection.h"
#include "EDefinitionCollection.h"
#include "EComponentCollection.h"
#include "EPrimitiveCollection.h"
#include "ELayerMapCollection.h"
#include "ECellInstCollection.h"
#include "EConnObjCollection.h"
#include "ELayerCollection.h"
#include "EModelCollection.h"
#include "ECellCollection.h"
#include "ENetCollection.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void ECollectionCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECollectionCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECollectionCollection::ECollectionCollection()
{
}

ECollectionCollection::~ECollectionCollection()
{
}

ECollectionCollection::ECollectionCollection(const ECollectionCollection & other)
{
    *this = other;
}

ECollectionCollection & ECollectionCollection::operator= (const ECollectionCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

Ptr<ICollection> ECollectionCollection::AddCollection(ECollectionType type)
{
    if(Count(type)) return nullptr;
    Ptr<ICollection> collection = nullptr;
    switch(type)
    {
        case ECollectionType::ComponentDefPin : {
            collection = new EComponentDefPinCollection;
            break;
        }
        case ECollectionType::PadstackInst : {
            collection = new EPadstackInstCollection;
            break;
        }
        case ECollectionType::HierarchyObj : {
            collection = new EHierarchyObjCollection;
            break;
        }
        case ECollectionType::ComponentDef : {
            collection = new EComponentDefCollection;
            break;
        }
        case ECollectionType::PadstackDef : {
            collection = new EPadstackDefCollection;
            break;
        }
        case ECollectionType::MaterialDef : {
            collection = new EMaterialDefCollection;
            break;
        }
        case ECollectionType::Definition : {
            collection = new EDefinitionCollection;
            break;
        }
        case ECollectionType::Component : {
            collection = new EComponentCollection;
            break;
        }
        case ECollectionType::Primitive : {
            collection = new EPrimitiveCollection;
            break;
        }
        case ECollectionType::LayerMap : {
            collection = new ELayerMapCollection;
            break;
        }
        case ECollectionType::CellInst : {
            collection = new ECellInstCollection;
            break;
        }
        case ECollectionType::ConnObj : {
            collection = new EConnObjCollection;
            break;
        }
        case ECollectionType::Model : {
            collection = new EModelCollection;
            break;
        }
        case ECollectionType::Layer : {
            collection = new ELayerCollection;
            break;
        }
        case ECollectionType::Cell : {
            collection = new ECellCollection;
            break;
        }
        case ECollectionType::Net : {
            collection = new ENetCollection;
            break;
        }
        default : {
            ECAD_ASSERT(false)
            break;
        }
    }
    Insert(type, UPtr<ICollection>(collection));
    return m_collection[type].get();
}

Ptr<ICollection> ECollectionCollection::GetCollection(ECollectionType type) const
{
    if (not Count(type)) return nullptr;
    return At(type).get();
}

Ptr<INetCollection> ECollectionCollection::NetCollection() const
{
    auto res = GetCollection(ECollectionType::Net);
    return dynamic_cast<Ptr<INetCollection> >(res);
}

Ptr<ICellCollection> ECollectionCollection::CellCollection() const
{
    auto res = GetCollection(ECollectionType::Cell);
    return dynamic_cast<Ptr<ICellCollection> >(res);
}

Ptr<ILayerCollection> ECollectionCollection::LayerCollection() const
{
    auto res = GetCollection(ECollectionType::Layer);
    return dynamic_cast<Ptr<ILayerCollection> >(res);
}

Ptr<IModelCollection> ECollectionCollection::ModelCollection() const
{
    auto res = GetCollection(ECollectionType::Model);
    return dynamic_cast<Ptr<IModelCollection> >(res);   
}

Ptr<IConnObjCollection> ECollectionCollection::ConnObjCollection() const
{
    auto res = GetCollection(ECollectionType::ConnObj);
    return dynamic_cast<Ptr<IConnObjCollection> >(res);
}

Ptr<ICellInstCollection> ECollectionCollection::CellInstCollection() const
{
    auto res = GetCollection(ECollectionType::CellInst);
    return dynamic_cast<Ptr<ICellInstCollection> >(res);   
}

Ptr<ILayerMapCollection> ECollectionCollection::LayerMapCollection() const
{
    auto res = GetCollection(ECollectionType::LayerMap);
    return dynamic_cast<Ptr<ILayerMapCollection> >(res);   
}

Ptr<IComponentCollection> ECollectionCollection::ComponentCollection() const
{
    auto res = GetCollection(ECollectionType::Component);
    return dynamic_cast<Ptr<IComponentCollection> >(res);
}

Ptr<IPrimitiveCollection> ECollectionCollection::PrimitiveCollection() const
{
    auto res = GetCollection(ECollectionType::Primitive);
    return dynamic_cast<Ptr<IPrimitiveCollection> >(res);    
}

Ptr<IDefinitionCollection> ECollectionCollection::DefinitionCollection() const
{
    auto res = GetCollection(ECollectionType::Definition);
    return dynamic_cast<Ptr<IDefinitionCollection> >(res);
}

Ptr<IMaterialDefCollection> ECollectionCollection::MaterialDefCollection() const
{
    auto res = GetCollection(ECollectionType::MaterialDef);
    return dynamic_cast<Ptr<IMaterialDefCollection> >(res);
}

Ptr<IPadstackDefCollection> ECollectionCollection::PadstackDefCollection() const
{
    auto res = GetCollection(ECollectionType::PadstackDef);
    return dynamic_cast<Ptr<IPadstackDefCollection> >(res);
}

Ptr<IComponentDefCollection> ECollectionCollection::ComponentDefCollection() const
{
    auto res = GetCollection(ECollectionType::ComponentDef);
    return dynamic_cast<Ptr<IComponentDefCollection> >(res);
}

Ptr<IHierarchyObjCollection> ECollectionCollection::HierarchyObjCollection() const
{
    auto res = GetCollection(ECollectionType::HierarchyObj);
    return dynamic_cast<Ptr<IHierarchyObjCollection> >(res);   
}

Ptr<IPadstackInstCollection> ECollectionCollection::PadstackInstCollection() const
{
    auto res = GetCollection(ECollectionType::PadstackInst);
    return dynamic_cast<Ptr<IPadstackInstCollection> >(res);
}

Ptr<IComponentDefPinCollection> ECollectionCollection::ComponentDefPinCollection() const
{
    auto res = GetCollection(ECollectionType::ComponentDefPin);
    return dynamic_cast<Ptr<IComponentDefPinCollection> >(res);
}

size_t ECollectionCollection::Size() const
{
    size_t count = 0;
    auto iter = m_collection.begin();
    for(; iter != m_collection.end(); ++iter)
        count += (iter->second)->Size();
    return count;
}

void ECollectionCollection::Clear()
{
    for (auto & collection : m_collection)
        collection.second->Clear();
}
}//namespace ecad