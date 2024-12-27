#include "EComponentDefCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentDefCollection)

#include "interface/IDefinitionCollection.h"
#include "interface/IComponentDef.h"
#include "interface/IDefinition.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void EComponentDefCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDefCollection, IComponentDefCollection>();
    boost::serialization::void_cast_register<EComponentDefCollection, IDefinitionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponentDefCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EComponentDefCollection::EComponentDefCollection()
{
}

EComponentDefCollection::~EComponentDefCollection()
{
}

EComponentDefCollection::EComponentDefCollection(const EComponentDefCollection & other)
{
    *this = other;
}
    
EComponentDefCollection & EComponentDefCollection::operator= (const EComponentDefCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

Ptr<IDefinition> EComponentDefCollection::AddDefinition(const std::string & name, UPtr<IDefinition> definition)
{
    auto type = definition->GetDefinitionType();
    auto componentDef = dynamic_cast<Ptr<IComponentDef> >(definition.get());
    if(nullptr == componentDef) return nullptr;
    Insert(name, UPtr<IComponentDef>(componentDef));
    definition.release();
    return GetDefinition(name, type);
}

Ptr<IDefinition> EComponentDefCollection::GetDefinition(const std::string & name, EDefinitionType type) const
{
    if(type == EDefinitionType::ComponentDef && BaseCollection::Count(name))
        return dynamic_cast<Ptr<IDefinition> >(BaseCollection::At(name).get());
    return nullptr;
}

std::string EComponentDefCollection::GetNextDefName(const std::string & base, EDefinitionType type) const
{
    if(type == EDefinitionType::ComponentDef) return NextKey(*this, base);
    return std::string{};
}

void EComponentDefCollection::SetDatabase(CPtr<IDatabase> database)
{
    auto compDefIter = GetComponentDefIter();
    while(auto compDef = compDefIter->Next()) {
        compDef->SetDatabase(database);
    }
}

ComponentDefIter EComponentDefCollection::GetComponentDefIter() const
{
    return ComponentDefIter(new EComponentDefIterator(*this));
}

size_t EComponentDefCollection::Size() const
{
    return BaseCollection::Size();
}

void EComponentDefCollection::Clear()
{
    BaseCollection::Clear();
}

void EComponentDefCollection::PrintImp(std::ostream & os) const
{
    BaseCollection::PrintImp(os);
    for (const auto & [name, def] : m_collection) {
        ECAD_UNUSED(def)
        os << "NAME: " << name << ECAD_EOL;
    }
}


}//namespace ecad