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
#include "ECellCollection.h"
#include "ENetCollection.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ECollectionCollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

template <typename Archive>
ECAD_INLINE void ECollectionCollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECollectionCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ECollectionCollection::ECollectionCollection()
{
    m_type = ECollectionType::Collection;
}

ECAD_INLINE ECollectionCollection::~ECollectionCollection()
{
}

ECAD_INLINE ECollectionCollection::ECollectionCollection(const ECollectionCollection & other)
{
    *this = other;
}

ECAD_INLINE ECollectionCollection & ECollectionCollection::operator= (const ECollectionCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<ICollection> ECollectionCollection::AddCollection(ECollectionType type)
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

ECAD_INLINE Ptr<ICollection> ECollectionCollection::GetCollection(ECollectionType type) const
{
    if(!Count(type)) return nullptr;
    return At(type).get();
}

ECAD_INLINE Ptr<INetCollection> ECollectionCollection::NetCollection() const
{
    auto res = GetCollection(ECollectionType::Net);
    return dynamic_cast<Ptr<INetCollection> >(res);
}

ECAD_INLINE Ptr<ICellCollection> ECollectionCollection::CellCollection() const
{
    auto res = GetCollection(ECollectionType::Cell);
    return dynamic_cast<Ptr<ICellCollection> >(res);
}

ECAD_INLINE Ptr<ILayerCollection> ECollectionCollection::LayerCollection() const
{
    auto res = GetCollection(ECollectionType::Layer);
    return dynamic_cast<Ptr<ILayerCollection> >(res);
}

ECAD_INLINE Ptr<IConnObjCollection> ECollectionCollection::ConnObjCollection() const
{
    auto res = GetCollection(ECollectionType::ConnObj);
    return dynamic_cast<Ptr<IConnObjCollection> >(res);
}

ECAD_INLINE Ptr<ICellInstCollection> ECollectionCollection::CellInstCollection() const
{
    auto res = GetCollection(ECollectionType::CellInst);
    return dynamic_cast<Ptr<ICellInstCollection> >(res);   
}

ECAD_INLINE Ptr<ILayerMapCollection> ECollectionCollection::LayerMapCollection() const
{
    auto res = GetCollection(ECollectionType::LayerMap);
    return dynamic_cast<Ptr<ILayerMapCollection> >(res);   
}

ECAD_INLINE Ptr<IComponentCollection> ECollectionCollection::ComponentCollection() const
{
    auto res = GetCollection(ECollectionType::Component);
    return dynamic_cast<Ptr<IComponentCollection> >(res);
}

ECAD_INLINE Ptr<IPrimitiveCollection> ECollectionCollection::PrimitiveCollection() const
{
    auto res = GetCollection(ECollectionType::Primitive);
    return dynamic_cast<Ptr<IPrimitiveCollection> >(res);    
}

ECAD_INLINE Ptr<IDefinitionCollection> ECollectionCollection::DefinitionCollection() const
{
    auto res = GetCollection(ECollectionType::Definition);
    return dynamic_cast<Ptr<IDefinitionCollection> >(res);
}

ECAD_INLINE Ptr<IMaterialDefCollection> ECollectionCollection::MaterialDefCollection() const
{
    auto res = GetCollection(ECollectionType::MaterialDef);
    return dynamic_cast<Ptr<IMaterialDefCollection> >(res);
}

ECAD_INLINE Ptr<IPadstackDefCollection> ECollectionCollection::PadstackDefCollection() const
{
    auto res = GetCollection(ECollectionType::PadstackDef);
    return dynamic_cast<Ptr<IPadstackDefCollection> >(res);
}

ECAD_INLINE Ptr<IComponentDefCollection> ECollectionCollection::ComponentDefCollection() const
{
    auto res = GetCollection(ECollectionType::ComponentDef);
    return dynamic_cast<Ptr<IComponentDefCollection> >(res);
}

ECAD_INLINE Ptr<IHierarchyObjCollection> ECollectionCollection::HierarchyObjCollection() const
{
    auto res = GetCollection(ECollectionType::HierarchyObj);
    return dynamic_cast<Ptr<IHierarchyObjCollection> >(res);   
}

ECAD_INLINE Ptr<IPadstackInstCollection> ECollectionCollection::PadstackInstCollection() const
{
    auto res = GetCollection(ECollectionType::PadstackInst);
    return dynamic_cast<Ptr<IPadstackInstCollection> >(res);
}

ECAD_INLINE Ptr<IComponentDefPinCollection> ECollectionCollection::ComponentDefPinCollection() const
{
    auto res = GetCollection(ECollectionType::ComponentDefPin);
    return dynamic_cast<Ptr<IComponentDefPinCollection> >(res);
}

ECAD_INLINE size_t ECollectionCollection::Size() const
{
    size_t count = 0;
    auto iter = m_collection.begin();
    for(; iter != m_collection.end(); ++iter)
        count += (iter->second)->Size();
    return count;
}

}//namespace ecad