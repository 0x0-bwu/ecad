#include "EComponentDefPinCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentDefPinCollection)

#include "design/EComponentDefPin.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EComponentDefPinCollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDefPinCollection, IComponentDefPinCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

template <typename Archive>
ECAD_INLINE void EComponentDefPinCollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDefPinCollection, IComponentDefPinCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponentDefPinCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EComponentDefPinCollection::EComponentDefPinCollection()
{
    m_type = ECollectionType::ComponentDefPin;
}

ECAD_INLINE EComponentDefPinCollection::~EComponentDefPinCollection()
{
}

ECAD_INLINE EComponentDefPinCollection::EComponentDefPinCollection(const EComponentDefPinCollection & other)
{
    *this = other;
}

ECAD_INLINE EComponentDefPinCollection & EComponentDefPinCollection::operator= (const EComponentDefPinCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<IComponentDefPin> EComponentDefPinCollection::AddPin(UPtr<IComponentDefPin> pin)
{
    Append(std::move(pin));
    return Back().get();
}
    
ECAD_INLINE Ptr<IComponentDefPin> EComponentDefPinCollection::CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef, ELayerId lyr)
{
    auto pin = new EComponentDefPin(name);
    pin->SetLocation(loc);
    pin->SetIOType(type);
    pin->SetPadstackDef(psDef);
    pin->SetLayerId(lyr);
    return AddPin(UPtr<IComponentDefPin>(pin));
}

ECAD_INLINE Ptr<IComponentDefPin> EComponentDefPinCollection::FindPinByName(const std::string & name) const
{
    auto pinIter = GetComponentDefPinIter();
    while (auto * pin = pinIter->Next()) {
        if (name == pin->GetName()) return pin;
    }
    return nullptr;
}

ECAD_INLINE ComponentDefPinIter EComponentDefPinCollection::GetComponentDefPinIter() const
{
    return ComponentDefPinIter(new EComponentDefPinIterator(*this));
}

ECAD_INLINE size_t EComponentDefPinCollection::Size() const
{
    return BaseCollection::Size();
}

ECAD_INLINE void EComponentDefPinCollection::Clear()
{
    return BaseCollection::Clear();
}

ECAD_INLINE Ptr<EComponentDefPinCollection> EComponentDefPinCollection::CloneImp() const
{
    return new EComponentDefPinCollection(*this);
}

}//namesapce ecad