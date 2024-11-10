#include "EDefinitionCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EDefinitionCollection)

#include "interface/IDefinition.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void EDefinitionCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EDefinitionCollection, IDefinitionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EDefinitionCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EDefinitionCollection::EDefinitionCollection()
{
    ECollectionCollection::Init();
}

ECAD_INLINE EDefinitionCollection::EDefinitionCollection(const EDefinitionCollection & other)
{
    *this = other;
}

ECAD_INLINE EDefinitionCollection & EDefinitionCollection::operator= (const EDefinitionCollection & other)
{
    ECollectionCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<IDefinitionCollection> EDefinitionCollection::AddDefinitionCollection(EDefinitionType type)
{
    switch(type)
    {
        case EDefinitionType::Cell : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::AddCollection(ECollectionType::Cell));
        }
        case EDefinitionType::LayerMap : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::AddCollection(ECollectionType::LayerMap));
        }
        case EDefinitionType::MaterialDef : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::AddCollection(ECollectionType::MaterialDef));
        }
        case EDefinitionType::PadstackDef : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::AddCollection(ECollectionType::PadstackDef));
        }
        case EDefinitionType::ComponentDef : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::AddCollection(ECollectionType::ComponentDef));
        }
        default : return nullptr;
    }
}

ECAD_INLINE Ptr<IDefinitionCollection> EDefinitionCollection::GetDefinitionCollection(EDefinitionType type) const
{
    switch(type)
    {
        case EDefinitionType::Cell : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::GetCollection(ECollectionType::Cell));
        }
        case EDefinitionType::LayerMap : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::GetCollection(ECollectionType::LayerMap));
        }
        case EDefinitionType::MaterialDef : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::GetCollection(ECollectionType::MaterialDef));
        }
        case EDefinitionType::PadstackDef : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::GetCollection(ECollectionType::PadstackDef));
        }
        case EDefinitionType::ComponentDef : {
            return dynamic_cast<Ptr<IDefinitionCollection> >(ECollectionCollection::GetCollection(ECollectionType::ComponentDef));
        }
        default : return nullptr;
    } 
}

ECAD_INLINE Ptr<IDefinition> EDefinitionCollection::AddDefinition(const std::string & name, UPtr<IDefinition> definition)
{
    auto dc = GetDefinitionCollection(definition->GetDefinitionType());
    if(dc) return dc->AddDefinition(name, std::move(definition));
    return nullptr;
}

ECAD_INLINE Ptr<IDefinition> EDefinitionCollection::GetDefinition(const std::string & name, EDefinitionType type) const
{
    auto dc = GetDefinitionCollection(type);
    if(dc) return dc->GetDefinition(name, type);
    return nullptr;
}

ECAD_INLINE std::string EDefinitionCollection::GetNextDefName(const std::string & name, EDefinitionType type) const
{
    auto dc = GetDefinitionCollection(type);
    if(dc) return dc->GetNextDefName(name, type);
    return std::string{};  
}

ECAD_INLINE void EDefinitionCollection::SetDatabase(CPtr<IDatabase> database)
{
    for (auto & collection : m_collection)
        dynamic_cast<Ptr<IDefinitionCollection>>(collection.second.get())->SetDatabase(database);
}

ECAD_INLINE size_t EDefinitionCollection::Size() const
{
    return ECollectionCollection::Size();
}

}//namespace ecad