#include "ENetCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ENetCollection)

#include "design/ENet.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void ENetCollection::serialize(Archive & ar, const unsigned int version)
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

ECAD_INLINE Ptr<INet> ENetCollection::FindNetByName(const std::string & name) const
{
    if(!Count(name)) return nullptr;
    return At(name).get();
}

ECAD_INLINE Ptr<INet> ENetCollection::FindNetByNetId(ENetId netId) const
{
    BuildNetIdLUT();
    auto iter = m_netIdNameMap->find(netId);
    if(iter == m_netIdNameMap->end()) return nullptr;
    return FindNetByName(iter->second);
}

ECAD_INLINE Ptr<INet> ENetCollection::CreateNet(const std::string & name)
{
    if(Count(name)) return nullptr;

    auto net = new ENet(name);
    net->SetNetId(static_cast<ENetId>(m_uidGen.GetNextUid()));
    Insert(name, UPtr<INet>(net));
    ResetNetIdLUT();
    return net;
}

ECAD_INLINE Ptr<INet> ENetCollection::AddNet(UPtr<INet> net)
{
    auto name = NextNetName(net->GetName());
    net->SetName(name);
    net->SetNetId(static_cast<ENetId>(m_uidGen.GetNextUid()));
    Insert(name, std::move(net));
    ResetNetIdLUT();
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
    ResetNetIdLUT();
    m_uidGen.Reset();
    BaseCollection::Clear();
}

ECAD_INLINE Ptr<ENetCollection> ENetCollection::CloneImp() const
{
    return new ENetCollection(*this);
}

ECAD_INLINE void ENetCollection::BuildNetIdLUT() const
{
    if(nullptr == m_netIdNameMap) {
        m_netIdNameMap = std::make_unique<NetIdNameMap>();
        auto iter = GetNetIter();
        while(auto net = iter->Next()) {
            m_netIdNameMap->insert(std::make_pair(net->GetNetId(), net->GetName()));
        }
    }
}

ECAD_INLINE void ENetCollection::ResetNetIdLUT()
{
    m_netIdNameMap.reset();
}

}//namesapce ecad