#include "ECellCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECellCollection)

#include "interface/IDefinition.h"
#include "interface/ICell.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ECellCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECellCollection, ICellCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECellCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ECellCollection::ECellCollection()
{
}

ECAD_INLINE ECellCollection::~ECellCollection()
{
}

ECAD_INLINE ECellCollection::ECellCollection(const ECellCollection & other)
{
    *this = other;
}

ECAD_INLINE ECellCollection & ECellCollection::operator= (const ECellCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<IDefinition> ECellCollection::AddDefinition(const std::string & name, UPtr<IDefinition> definition)
{
    auto type = definition->GetDefinitionType();
    auto cell = dynamic_cast<Ptr<ICell> >(definition.get());
    if(nullptr == cell) return nullptr;
    Insert(name, UPtr<ICell>(cell));
    definition.release();
    return GetDefinition(name, type);
}

ECAD_INLINE Ptr<IDefinition> ECellCollection::GetDefinition(const std::string & name, EDefinitionType type) const
{
    if(type == EDefinitionType::Cell && BaseCollection::Count(name))
        return dynamic_cast<Ptr<IDefinition> >(BaseCollection::At(name).get());
    return nullptr;
}

ECAD_INLINE std::string ECellCollection::GetNextDefName(const std::string & base, EDefinitionType type) const
{
    if(type == EDefinitionType::Cell) return NextKey(*this, base);
    return std::string{};
}

ECAD_INLINE void ECellCollection::SetDatabase(CPtr<IDatabase> database)
{
    auto cellIter = GetCellIter();
    while(auto cell = cellIter->Next()){
        cell->SetDatabase(database);
    }
}

ECAD_INLINE CellIter ECellCollection::GetCellIter() const
{
    return CellIter(new ECellIterator(*this));
}

ECAD_INLINE size_t ECellCollection::Size() const
{
    return BaseCollection::Size();
}

ECAD_INLINE void ECellCollection::Clear()
{
    BaseCollection::Clear();
}

}//namesapce ecad