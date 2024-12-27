#include "EMaterialDefCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialDefCollection)

#include "interface/IDefinitionCollection.h"
#include "interface/IMaterialDef.h"
#include "interface/IDefinition.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void EMaterialDefCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialDefCollection, IMaterialDefCollection>();
    boost::serialization::void_cast_register<EMaterialDefCollection, IDefinitionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
    ar & boost::serialization::make_nvp("id_lut", m_idLut);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EMaterialDefCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EMaterialDefCollection::EMaterialDefCollection()
{
}

EMaterialDefCollection::~EMaterialDefCollection()
{
}

EMaterialDefCollection::EMaterialDefCollection(const EMaterialDefCollection & other)
{
    *this = other;
}
    
EMaterialDefCollection & EMaterialDefCollection::operator= (const EMaterialDefCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

Ptr<IDefinition> EMaterialDefCollection::AddDefinition(const std::string & name, UPtr<IDefinition> definition)
{
    auto type = definition->GetDefinitionType();
    auto materialDef = dynamic_cast<Ptr<IMaterialDef> >(definition.get());
    if(nullptr == materialDef) return nullptr;
    Insert(name, UPtr<IMaterialDef>(materialDef));
    m_idLut.emplace(materialDef->GetMaterialId(), materialDef);
    definition.release();
    return GetDefinition(name, type);
}

Ptr<IDefinition> EMaterialDefCollection::GetDefinition(const std::string & name, EDefinitionType type) const
{
    if(type == EDefinitionType::MaterialDef && BaseCollection::Count(name))
        return dynamic_cast<Ptr<IDefinition> >(BaseCollection::At(name).get());
    return nullptr;
}

std::string EMaterialDefCollection::GetNextDefName(const std::string & base, EDefinitionType type) const
{
    if(type == EDefinitionType::MaterialDef) return NextKey(*this, base);
    return std::string{};
}

Ptr<IMaterialDef> EMaterialDefCollection::FindMaterialDefById(EMaterialId id) const
{
    auto iter = m_idLut.find(id);
    if (iter != m_idLut.cend())
        return iter->second;
    return nullptr;
}

void EMaterialDefCollection::SetDatabase(CPtr<IDatabase> database)
{
    auto matIter = GetMaterialDefIter();
    while(auto mat = matIter->Next()) {
        mat->SetDatabase(database);
    }
}


MaterialDefIter EMaterialDefCollection::GetMaterialDefIter() const
{
    return MaterialDefIter(new EMaterialDefIterator(*this));
}

size_t EMaterialDefCollection::Size() const
{
    return BaseCollection::Size();
}

void EMaterialDefCollection::Clear()
{
    BaseCollection::Clear();
    m_idLut.clear();
}

void EMaterialDefCollection::PrintImp(std::ostream & os) const
{
    BaseCollection::PrintImp(os);
    for (const auto & [name, def] : m_collection) {
        ECAD_UNUSED(def)
        os << "NAME: " << name << ECAD_EOL;
    }
}

}//namespace ecad