#include "EComponentCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentCollection)

#include "EComponent.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void EComponentCollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentCollection, IComponentCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

template <typename Archive>
ECAD_INLINE void EComponentCollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentCollection, EComponentCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponentCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EComponentCollection::EComponentCollection()
{
    m_type = ECollectionType::Component;
}

ECAD_INLINE EComponentCollection::~EComponentCollection()
{
}

ECAD_INLINE EComponentCollection::EComponentCollection(const EComponentCollection & other)
{
    *this = other;
}

ECAD_INLINE EComponentCollection & EComponentCollection::operator= (const EComponentCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<IComponent> EComponentCollection::AddComponent(UPtr<IComponent> component)
{
    auto name = component->GetName();
    if (Count(name)) {
        name = NextKey(*this, name);
        dynamic_cast<Ptr<EObject>>(component.get())->SetName(name);
    }
    if (Insert(name, std::move(component)))
        return At(name).get();
    return nullptr;
}


ECAD_INLINE Ptr<IComponent> EComponentCollection::CreateComponent(const std::string & name, CPtr<IComponentDef> compDef, ELayerId layer, const ETransform2D & transform)
{
    auto component = new EComponent(name, compDef);
    component->SetPlacementLayer(layer);
    component->SetTransform(transform);
    if (Insert(name, UPtr<IComponent>(component)))
        return At(name).get();
    return nullptr;
}

ECAD_INLINE ComponentIter EComponentCollection::GetComponentIter() const
{
    return ComponentIter(new EComponentIterator(*this));
}

ECAD_INLINE size_t EComponentCollection::Size() const
{
    return BaseCollection::Size();
}

ECAD_INLINE void EComponentCollection::Clear()
{
    return BaseCollection::Clear();
}

}//namesapce ecad