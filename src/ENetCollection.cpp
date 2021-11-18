#ifndef ECAD_HEADER_ONLY
#include "ENetCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ENetCollection)
#endif

#include "ENet.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ENetCollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ENetCollection, INetCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
    ar & boost::serialization::make_nvp("uid_gen", m_uidGen);
}

template <typename Archive>
ECAD_INLINE void ENetCollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ENetCollection, INetCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
    ar & boost::serialization::make_nvp("uid_gen", m_uidGen);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ENetCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ENetCollection::ENetCollection()
{
    m_type = ECollectionType::Net;
}

ECAD_INLINE ENetCollection::~ENetCollection()
{
}

ECAD_INLINE ENetCollection::ENetCollection(const ENetCollection & other)
{
    *this = other;
}

ECAD_INLINE ENetCollection & ENetCollection::operator= (const ENetCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE std::string ENetCollection::NextNetName(const std::string & name) const
{
    return NextKey(*this, name);
}

ECAD_INLINE Ptr<INet> ENetCollection::FindNetByName(const std::string & name)
{
    if(!Count(name)) return nullptr;
    return At(name).get();
}

ECAD_INLINE Ptr<INet> ENetCollection::CreateNet(const std::string & name)
{
    if(Count(name)) return nullptr;

    auto net = new ENet(name);
    net->SetNetId(static_cast<ENetId>(m_uidGen.GetNextUid()));
    Insert(name, UPtr<INet>(net));
    return net;
}

ECAD_INLINE Ptr<INet> ENetCollection::AddNet(UPtr<INet> net)
{
    auto name = NextNetName(net->GetName());
    net->SetName(name);
    net->SetNetId(static_cast<ENetId>(m_uidGen.GetNextUid()));
    Insert(name, std::move(net));
    return At(name).get();
}


ECAD_INLINE NetIter ENetCollection::GetNetIter() const
{
    return NetIter(new ENetIterator(*this));
}

ECAD_INLINE size_t ENetCollection::Size() const
{
    return BaseCollection::Size();
}

ECAD_INLINE void ENetCollection::Clear()
{
    m_uidGen.Reset();
    BaseCollection::Clear();
}

ECAD_INLINE Ptr<ENetCollection> ENetCollection::CloneImp() const
{
    return new ENetCollection(*this);
}

}//namesapce ecad