#ifndef ECAD_HEADER_ONLY
#include "EConnObjCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EConnObjCollection)
#endif

#include "ECollectionCollection.h"
#include "Interface.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EConnObjCollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EConnObjCollection, IConnObjCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
}

template <typename Archive>
ECAD_INLINE void EConnObjCollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EConnObjCollection, IConnObjCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EConnObjCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EConnObjCollection::EConnObjCollection()
{
    m_type = ECollectionType::ConnObj;
    for(auto type : m_collectionTypes) 
        AddCollection(type);}

ECAD_INLINE EConnObjCollection::~EConnObjCollection()
{
}

ECAD_INLINE EConnObjCollection::EConnObjCollection(const EConnObjCollection & other)
{
    *this = other;
}

ECAD_INLINE EConnObjCollection & EConnObjCollection::operator= (const EConnObjCollection & other)
{
    ECollectionCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<IPrimitiveCollection> EConnObjCollection::GetPrimitiveCollection() const
{
    return ECollectionCollection::GetPrimitiveCollection();
}

ECAD_INLINE Ptr<IPadstackInstCollection> EConnObjCollection::GetPadstackInstCollection() const
{
    return ECollectionCollection::GetPadstackInstCollection();
}

ECAD_INLINE ConnObjIter EConnObjCollection::GetConnObjIter() const
{
    return ConnObjIter(new EConnObjIterator(*this));
}

ECAD_INLINE size_t EConnObjCollection::Size() const
{
    size_t count = 0;
    for(auto type : m_collectionTypes){
        count += GetCollection(type)->Size();
    }
    return count;
}

ECAD_INLINE EConnObjIterator::EConnObjIterator(const EConnObjCollection & collection)
{
    m_primitiveIter = collection.GetPrimitiveCollection()->GetPrimitiveIter();
    m_padstackInstIter = collection.GetPadstackInstCollection()->GetPadstackInstIter();
}

ECAD_INLINE EConnObjIterator::~EConnObjIterator()
{
}

ECAD_INLINE EConnObjIterator::EConnObjIterator(const EConnObjIterator & other)
{
    *this = other;
}

ECAD_INLINE EConnObjIterator & EConnObjIterator::operator= (const EConnObjIterator & other)
{
    m_primitiveIter = std::move(other.m_primitiveIter->Clone());
    m_padstackInstIter = std::move(other.m_padstackInstIter->Clone());
    return *this;
}

ECAD_INLINE EConnObjIterator::EConnObjIterator(EConnObjIterator && other)
{
    Move(std::forward<EConnObjIterator>(other));
}

ECAD_INLINE EConnObjIterator & EConnObjIterator::operator= (EConnObjIterator && other)
{ 
    Move(std::forward<EConnObjIterator>(other));
    return *this;
}

ECAD_INLINE Ptr<IConnObj> EConnObjIterator::Next()
{
    Ptr<IConnObj> p = nullptr;
    if(p = dynamic_cast<Ptr<IConnObj> >(m_primitiveIter->Next())) return p;
    if(p = dynamic_cast<Ptr<IConnObj> >(m_padstackInstIter->Next())) return p;
    return p;
}

ECAD_INLINE Ptr<IConnObj> EConnObjIterator::Current()
{
    if(m_primitiveIter->Current()) return dynamic_cast<Ptr<IConnObj> >(m_primitiveIter->Current());
    if(m_padstackInstIter->Current()) return dynamic_cast<Ptr<IConnObj> >(m_padstackInstIter->Current());
    return nullptr;
}

ECAD_INLINE ConnObjIter EConnObjIterator::Clone() const
{
    return ConnObjIter(new EConnObjIterator(*this));
}

ECAD_INLINE void EConnObjIterator::Move(EConnObjIterator && other)
{
    m_primitiveIter = std::move(other.m_primitiveIter);
    m_padstackInstIter = std::move(other.m_padstackInstIter);

    other.m_primitiveIter = nullptr;
    other.m_padstackInstIter = nullptr;
}

}//namesapce ecad