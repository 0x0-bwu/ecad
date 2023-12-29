#include "EMaterialDefCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialDefCollection)

#include "interfaces/IDefinitionCollection.h"
#include "interfaces/IMaterialDef.h"
#include "interfaces/IDefinition.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EMaterialDefCollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialDefCollection, IMaterialDefCollection>();
    boost::serialization::void_cast_register<EMaterialDefCollection, IDefinitionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
    ar & boost::serialization::make_nvp("id_lut", m_idLut);
}

template <typename Archive>
ECAD_INLINE void EMaterialDefCollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialDefCollection, IMaterialDefCollection>();
    boost::serialization::void_cast_register<EMaterialDefCollection, IDefinitionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
    ar & boost::serialization::make_nvp("id_lut", m_idLut);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EMaterialDefCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EMaterialDefCollection::EMaterialDefCollection()
{
    m_type = ECollectionType::MaterialDef;
}

ECAD_INLINE EMaterialDefCollection::~EMaterialDefCollection()
{
}

ECAD_INLINE EMaterialDefCollection::EMaterialDefCollection(const EMaterialDefCollection & other)
{
    *this = other;
}
    
ECAD_INLINE EMaterialDefCollection & EMaterialDefCollection::operator= (const EMaterialDefCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<IDefinitionCollection> EMaterialDefCollection::AddDefinitionCollection(EDefinitionType type)
{
    return nullptr;
}

ECAD_INLINE Ptr<IDefinitionCollection> EMaterialDefCollection::GetDefinitionCollection(EDefinitionType type) const
{
    if(type == EDefinitionType::MaterialDef) return const_cast<Ptr<EMaterialDefCollection> >(this);
    return nullptr;
}

ECAD_INLINE Ptr<IDefinition> EMaterialDefCollection::AddDefinition(const std::string & name, UPtr<IDefinition> definition)
{
    auto type = definition->GetDefinitionType();
    auto materialDef = dynamic_cast<Ptr<IMaterialDef> >(definition.get());
    if(nullptr == materialDef) return nullptr;
    Insert(name, UPtr<IMaterialDef>(materialDef));
    m_idLut.emplace(materialDef->GetMaterialId(), materialDef);
    definition.release();
    return GetDefinition(name, type);
}

ECAD_INLINE Ptr<IDefinition> EMaterialDefCollection::GetDefinition(const std::string & name, EDefinitionType type) const
{
    if(type == EDefinitionType::MaterialDef && BaseCollection::Count(name))
        return dynamic_cast<Ptr<IDefinition> >(BaseCollection::At(name).get());
    return nullptr;
}

ECAD_INLINE std::string EMaterialDefCollection::GetNextDefName(const std::string & base, EDefinitionType type) const
{
    if(type == EDefinitionType::MaterialDef) return NextKey(*this, base);
    return std::string{};
}

ECAD_INLINE Ptr<IMaterialDef> EMaterialDefCollection::FindMaterialDefById(EMaterialId id) const
{
    auto iter = m_idLut.find(id);
    if (iter != m_idLut.cend())
        return iter->second;
    return nullptr;
}

ECAD_INLINE MaterialDefIter EMaterialDefCollection::GetMaterialDefIter() const
{
    return MaterialDefIter(new EMaterialDefIterator(*this));
}

ECAD_INLINE size_t EMaterialDefCollection::Size() const
{
    return BaseCollection::Size();
}

ECAD_INLINE void EMaterialDefCollection::Clear()
{
    BaseCollection::Clear();
    m_idLut.clear();
}

ECAD_INLINE void EMaterialDefCollection::PrintImp(std::ostream & os) const
{
    BaseCollection::PrintImp(os);
    for (const auto & [name, def] : m_collection) {
        ECAD_UNUSED(def)
        os << "NAME: " << name << ECAD_EOL;
    }
}

}//namesapce ecad