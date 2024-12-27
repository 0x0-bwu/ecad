#include "EHierarchyObjCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EHierarchyObjCollection)

#include "interface/IComponentCollection.h"
#include "interface/ICellInstCollection.h"
#include "interface/IHierarchyObj.h"
#include "interface/IComponent.h"
#include "interface/ICellInst.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void EHierarchyObjCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EHierarchyObjCollection, IHierarchyObjCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EHierarchyObjCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EHierarchyObjCollection::EHierarchyObjCollection()
{
    ECollectionCollection::Init();
}

EHierarchyObjCollection::~EHierarchyObjCollection()
{
}

EHierarchyObjCollection::EHierarchyObjCollection(const EHierarchyObjCollection & other)
{
    *this = other;
}

EHierarchyObjCollection & EHierarchyObjCollection::operator= (const EHierarchyObjCollection & other)
{
    ECollectionCollection::operator=(other);
    return *this;
}

Ptr<ICellInstCollection> EHierarchyObjCollection::GetCellInstCollection() const
{
    return ECollectionCollection::CellInstCollection();
}

Ptr<IComponentCollection> EHierarchyObjCollection::GetComponentCollection() const
{
    return ECollectionCollection::ComponentCollection();
}

HierarchyObjIter EHierarchyObjCollection::GetHierarchyObjIter() const
{
    return HierarchyObjIter(new EHierarchyObjIterator(*this));
}

size_t EHierarchyObjCollection::Size() const
{
    size_t count = 0;
    for (auto type : GetCollectionTypes()){
        count+= GetCollection(type)->Size();
    }
    return count;
}

EHierarchyObjIterator::EHierarchyObjIterator(const EHierarchyObjCollection & collection)
{
    m_cellInstIter = collection.GetCellInstCollection()->GetCellInstIter();
    m_componentIter = collection.GetComponentCollection()->GetComponentIter();
}

EHierarchyObjIterator::~EHierarchyObjIterator()
{
}

EHierarchyObjIterator::EHierarchyObjIterator(const EHierarchyObjIterator & other)
{
    *this = other;
}

EHierarchyObjIterator & EHierarchyObjIterator::operator= (const EHierarchyObjIterator & other)
{
    m_cellInstIter = other.m_cellInstIter->Clone();
    m_componentIter = other.m_componentIter->Clone();
    return *this;
}

EHierarchyObjIterator::EHierarchyObjIterator(EHierarchyObjIterator && other)
{
    Move(std::forward<EHierarchyObjIterator>(other));
}

EHierarchyObjIterator & EHierarchyObjIterator::operator= (EHierarchyObjIterator && other)
{ 
    Move(std::forward<EHierarchyObjIterator>(other));
    return *this;
}

Ptr<IHierarchyObj> EHierarchyObjIterator::Next()
{
    if(auto p = dynamic_cast<Ptr<IHierarchyObj> >(m_cellInstIter->Next()); p) return p;
    if(auto p = dynamic_cast<Ptr<IHierarchyObj> >(m_componentIter->Next()); p) return p;
    return nullptr;
}

Ptr<IHierarchyObj> EHierarchyObjIterator::Current()
{
    if(m_cellInstIter->Current()) return dynamic_cast<Ptr<IHierarchyObj> >(m_cellInstIter->Current());
    
    return nullptr;
}

HierarchyObjIter EHierarchyObjIterator::Clone() const
{
    return HierarchyObjIter(new EHierarchyObjIterator(*this));
}

void EHierarchyObjIterator::Move(EHierarchyObjIterator && other)
{
    m_cellInstIter = std::move(other.m_cellInstIter);
    m_componentIter = std::move(other.m_componentIter);

    other.m_cellInstIter = nullptr;
    other.m_componentIter = nullptr;
}

}//namespace ecad