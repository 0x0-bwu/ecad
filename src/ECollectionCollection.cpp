#include "ECollectionCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECollectionCollection)

#include "EComponentDefPinCollection.h"
#include "EPadstackInstCollection.h"
#include "EHierarchyObjCollection.h"
#include "EComponentDefCollection.h"
#include "EPadstackDefCollection.h"
#include "EMaterialDefCollection.h"
#include "EDefinitionCollection.h"
#include "EPrimitiveCollection.h"
#include "ELayerMapCollection.h"
#include "ECellInstCollection.h"
#include "EConnObjCollection.h"
#include "ELayerCollection.h"
#include "ECellCollection.h"
#include "ENetCollection.h"
#include "ECadDef.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ECollectionCollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECollectionCollection, ICollectionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

template <typename Archive>
ECAD_INLINE void ECollectionCollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECollectionCollection, ICollectionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECollectionCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ECollectionCollection::ECollectionCollection()
 : ECollectionCollection(std::string{})
{
}

ECAD_INLINE ECollectionCollection::ECollectionCollection(std::string name)
 : BaseCollection(std::move(name))
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
        default : break;
    }
    Insert(type, UPtr<ICollection>(collection));
    return m_collection[type].get();
}

ECAD_INLINE Ptr<ICollection> ECollectionCollection::GetCollection(ECollectionType type) const
{
    if(!Count(type)) return nullptr;
    return At(type).get();
}

ECAD_INLINE Ptr<INetCollection> ECollectionCollection::GetNetCollection() const
{
    auto res = GetCollection(ECollectionType::Net);
    return dynamic_cast<Ptr<INetCollection> >(res);
}

ECAD_INLINE Ptr<ICellCollection> ECollectionCollection::GetCellCollection() const
{
    auto res = GetCollection(ECollectionType::Cell);
    return dynamic_cast<Ptr<ICellCollection> >(res);
}

ECAD_INLINE Ptr<ILayerCollection> ECollectionCollection::GetLayerCollection() const
{
    auto res = GetCollection(ECollectionType::Layer);
    return dynamic_cast<Ptr<ILayerCollection> >(res);
}

ECAD_INLINE Ptr<IConnObjCollection> ECollectionCollection::GetConnObjCollection() const
{
    auto res = GetCollection(ECollectionType::ConnObj);
    return dynamic_cast<Ptr<IConnObjCollection> >(res);
}

ECAD_INLINE Ptr<ICellInstCollection> ECollectionCollection::GetCellInstCollection() const
{
    auto res = GetCollection(ECollectionType::CellInst);
    return dynamic_cast<Ptr<ICellInstCollection> >(res);   
}

ECAD_INLINE Ptr<ILayerMapCollection> ECollectionCollection::GetLayerMapCollection() const
{
    auto res = GetCollection(ECollectionType::LayerMap);
    return dynamic_cast<Ptr<ILayerMapCollection> >(res);   
}

ECAD_INLINE Ptr<IPrimitiveCollection> ECollectionCollection::GetPrimitiveCollection() const
{
    auto res = GetCollection(ECollectionType::Primitive);
    return dynamic_cast<Ptr<IPrimitiveCollection> >(res);    
}

ECAD_INLINE Ptr<IDefinitionCollection> ECollectionCollection::GetDefinitionCollection() const
{
    auto res = GetCollection(ECollectionType::Definition);
    return dynamic_cast<Ptr<IDefinitionCollection> >(res);
}

ECAD_INLINE Ptr<IMaterialDefCollection> ECollectionCollection::GetMaterialDefCollection() const
{
    auto res = GetCollection(ECollectionType::MaterialDef);
    return dynamic_cast<Ptr<IMaterialDefCollection> >(res);
}

ECAD_INLINE Ptr<IPadstackDefCollection> ECollectionCollection::GetPadstackDefCollection() const
{
    auto res = GetCollection(ECollectionType::PadstackDef);
    return dynamic_cast<Ptr<IPadstackDefCollection> >(res);
}

ECAD_INLINE Ptr<IComponentDefCollection> ECollectionCollection::GetComponentDefCollection() const
{
    auto res = GetCollection(ECollectionType::ComponentDef);
    return dynamic_cast<Ptr<IComponentDefCollection> >(res);
}

ECAD_INLINE Ptr<IHierarchyObjCollection> ECollectionCollection::GetHierarchyObjCollection() const
{
    auto res = GetCollection(ECollectionType::HierarchyObj);
    return dynamic_cast<Ptr<IHierarchyObjCollection> >(res);   
}

ECAD_INLINE Ptr<IPadstackInstCollection> ECollectionCollection::GetPadstackInstCollection() const
{
    auto res = GetCollection(ECollectionType::PadstackInst);
    return dynamic_cast<Ptr<IPadstackInstCollection> >(res);
}

ECAD_INLINE Ptr<IComponentDefPinCollection> ECollectionCollection::GetComponentDefPinCollection() const
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