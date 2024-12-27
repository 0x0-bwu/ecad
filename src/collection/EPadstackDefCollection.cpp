#include "EPadstackDefCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPadstackDefCollection)

#include "interface/IPadstackDef.h"
#include "interface/IDefinition.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void EPadstackDefCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackDefCollection, IPadstackDefCollection>();
    boost::serialization::void_cast_register<EPadstackDefCollection, IDefinitionCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPadstackDefCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EPadstackDefCollection::EPadstackDefCollection()
{
}

EPadstackDefCollection::~EPadstackDefCollection()
{
}

EPadstackDefCollection::EPadstackDefCollection(const EPadstackDefCollection & other)
{
    *this = other;
}

EPadstackDefCollection & EPadstackDefCollection::operator= (const EPadstackDefCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

Ptr<IDefinition> EPadstackDefCollection::AddDefinition(const std::string & name, UPtr<IDefinition> definition)
{
    auto type = definition->GetDefinitionType();
    auto padstackDef = dynamic_cast<Ptr<IPadstackDef> >(definition.get());
    if(nullptr == padstackDef) return nullptr;
    Insert(name, UPtr<IPadstackDef>(padstackDef));
    definition.release();
    return GetDefinition(name, type);
}

Ptr<IDefinition> EPadstackDefCollection::GetDefinition(const std::string & name, EDefinitionType type) const
{
    if(type == EDefinitionType::PadstackDef && BaseCollection::Count(name))
        return dynamic_cast<Ptr<IDefinition> >(BaseCollection::At(name).get());
    return nullptr;
}

std::string EPadstackDefCollection::GetNextDefName(const std::string & base, EDefinitionType type) const
{
    if(type == EDefinitionType::PadstackDef) return NextKey(*this, base);
    return std::string{};
}

void EPadstackDefCollection::SetDatabase(CPtr<IDatabase> database)
{
    auto psIter = GetPadstackDefIter();
    while(auto ps = psIter->Next()){
        ps->SetDatabase(database);
    }
}

PadstackDefIter EPadstackDefCollection::GetPadstackDefIter() const
{
    return PadstackDefIter(new EPadstackDefIterator(*this));
}

size_t EPadstackDefCollection::Size() const
{
    return BaseCollection::Size();
}

void EPadstackDefCollection::Clear()
{
    BaseCollection::Clear();
}

}//namespace ecad