#include "EConnObjCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EConnObjCollection)

#include "interface/IPadstackInstCollection.h"
#include "interface/IPrimitiveCollection.h"
#include "interface/IPadstackInst.h"
#include "interface/IDefinition.h"
#include "interface/IPrimitive.h"
#include "interface/IConnObj.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void EConnObjCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EConnObjCollection, IConnObjCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EConnObjCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EConnObjCollection::EConnObjCollection()
{
    ECollectionCollection::Init();
}

EConnObjCollection::~EConnObjCollection()
{
}

EConnObjCollection::EConnObjCollection(const EConnObjCollection & other)
{
    *this = other;
}

EConnObjCollection & EConnObjCollection::operator= (const EConnObjCollection & other)
{
    ECollectionCollection::operator=(other);
    return *this;
}

Ptr<IPrimitiveCollection> EConnObjCollection::GetPrimitiveCollection() const
{
    return ECollectionCollection::PrimitiveCollection();
}

Ptr<IPadstackInstCollection> EConnObjCollection::GetPadstackInstCollection() const
{
    return ECollectionCollection::PadstackInstCollection();
}

ConnObjIter EConnObjCollection::GetConnObjIter() const
{
    return ConnObjIter(new EConnObjIterator(*this));
}

size_t EConnObjCollection::Size() const
{
    size_t count = 0;
    for(auto type : GetCollectionTypes()){
        count += GetCollection(type)->Size();
    }
    return count;
}

EConnObjIterator::EConnObjIterator(const EConnObjCollection & collection)
{
    m_primitiveIter = collection.GetPrimitiveCollection()->GetPrimitiveIter();
    m_padstackInstIter = collection.GetPadstackInstCollection()->GetPadstackInstIter();
}

EConnObjIterator::~EConnObjIterator()
{
}

EConnObjIterator::EConnObjIterator(const EConnObjIterator & other)
{
    *this = other;
}

EConnObjIterator & EConnObjIterator::operator= (const EConnObjIterator & other)
{
    m_primitiveIter = other.m_primitiveIter->Clone();
    m_padstackInstIter = other.m_padstackInstIter->Clone();
    return *this;
}

EConnObjIterator::EConnObjIterator(EConnObjIterator && other)
{
    Move(std::forward<EConnObjIterator>(other));
}

EConnObjIterator & EConnObjIterator::operator= (EConnObjIterator && other)
{ 
    Move(std::forward<EConnObjIterator>(other));
    return *this;
}

Ptr<IConnObj> EConnObjIterator::Next()
{
    if (auto p = dynamic_cast<Ptr<IConnObj> >(m_primitiveIter->Next()); p) return p;
    if (auto p = dynamic_cast<Ptr<IConnObj> >(m_padstackInstIter->Next()); p) return p;
    return nullptr;
}

Ptr<IConnObj> EConnObjIterator::Current()
{
    if (m_primitiveIter->Current()) return dynamic_cast<Ptr<IConnObj> >(m_primitiveIter->Current());
    if (m_padstackInstIter->Current()) return dynamic_cast<Ptr<IConnObj> >(m_padstackInstIter->Current());
    return nullptr;
}

ConnObjIter EConnObjIterator::Clone() const
{
    return ConnObjIter(new EConnObjIterator(*this));
}

void EConnObjIterator::Move(EConnObjIterator && other)
{
    m_primitiveIter = std::move(other.m_primitiveIter);
    m_padstackInstIter = std::move(other.m_padstackInstIter);

    other.m_primitiveIter = nullptr;
    other.m_padstackInstIter = nullptr;
}

}//namespace ecad