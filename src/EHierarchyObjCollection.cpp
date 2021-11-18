#ifndef ECAD_HEADER_ONLY
#include "EHierarchyObjCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EHierarchyObjCollection)
#endif

#include "ECollectionCollection.h"
#include "Interface.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
template <typename Archive>
ECAD_INLINE void EHierarchyObjCollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EHierarchyObjCollection, IHierarchyObjCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
}

template <typename Archive>
ECAD_INLINE void EHierarchyObjCollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EHierarchyObjCollection, IHierarchyObjCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EHierarchyObjCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EHierarchyObjCollection::EHierarchyObjCollection()
{
    m_type = ECollectionType::HierarchyObj;
    for(auto type : m_collectionTypes) 
        AddCollection(type);
}

ECAD_INLINE EHierarchyObjCollection::~EHierarchyObjCollection()
{
}

ECAD_INLINE EHierarchyObjCollection::EHierarchyObjCollection(const EHierarchyObjCollection & other)
{
    *this = other;
}

ECAD_INLINE EHierarchyObjCollection & EHierarchyObjCollection::operator= (const EHierarchyObjCollection & other)
{
    ECollectionCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<ICellInstCollection> EHierarchyObjCollection::GetCellInstCollection() const
{
    return ECollectionCollection::GetCellInstCollection();
}

ECAD_INLINE HierarchyObjIter EHierarchyObjCollection::GetHierarchyObjIter() const
{
    return HierarchyObjIter(new EHierarchyObjIterator(*this));
}

ECAD_INLINE size_t EHierarchyObjCollection::Size() const
{
    size_t count = 0;
    for(auto type : m_collectionTypes){
        count+= GetCollection(type)->Size();
    }
    return count;
}

ECAD_INLINE EHierarchyObjIterator::EHierarchyObjIterator(const EHierarchyObjCollection & collection)
{
    m_cellInstIter = collection.GetCellInstCollection()->GetCellInstIter();
}

ECAD_INLINE EHierarchyObjIterator::~EHierarchyObjIterator()
{
}

ECAD_INLINE EHierarchyObjIterator::EHierarchyObjIterator(const EHierarchyObjIterator & other)
{
    *this = other;
}

ECAD_INLINE EHierarchyObjIterator & EHierarchyObjIterator::operator= (const EHierarchyObjIterator & other)
{
    m_cellInstIter = std::move(other.m_cellInstIter->Clone());
    return *this;
}

ECAD_INLINE EHierarchyObjIterator::EHierarchyObjIterator(EHierarchyObjIterator && other)
{
    Move(std::forward<EHierarchyObjIterator>(other));
}

ECAD_INLINE EHierarchyObjIterator & EHierarchyObjIterator::operator= (EHierarchyObjIterator && other)
{ 
    Move(std::forward<EHierarchyObjIterator>(other));
    return *this;
}

ECAD_INLINE Ptr<IHierarchyObj> EHierarchyObjIterator::Next()
{
    Ptr<IHierarchyObj> p = nullptr;
    if(p = dynamic_cast<Ptr<IHierarchyObj> >(m_cellInstIter->Next())) return p;
    return p;
}

ECAD_INLINE Ptr<IHierarchyObj> EHierarchyObjIterator::Current()
{
    if(m_cellInstIter->Current()) return dynamic_cast<Ptr<IHierarchyObj> >(m_cellInstIter->Current());
    return nullptr;
}

ECAD_INLINE HierarchyObjIter EHierarchyObjIterator::Clone() const
{
    return HierarchyObjIter(new EHierarchyObjIterator(*this));
}

ECAD_INLINE void EHierarchyObjIterator::Move(EHierarchyObjIterator && other)
{
    m_cellInstIter = std::move(other.m_cellInstIter);

    other.m_cellInstIter = nullptr;
}

}//namesapce ecad