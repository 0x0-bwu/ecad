#include "ENetCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ENetCollection)

#include "design/ENet.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void ENetCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ENetCollection, INetCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
    ar & boost::serialization::make_nvp("uid_gen", m_uidGen);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ENetCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ENetCollection::ENetCollection()
{
}

ENetCollection::~ENetCollection()
{
}

ENetCollection::ENetCollection(const ENetCollection & other)
{
    *this = other;
}

ENetCollection & ENetCollection::operator= (const ENetCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

std::string ENetCollection::NextNetName(const std::string & name) const
{
    return NextKey(*this, name);
}

Ptr<INet> ENetCollection::FindNetByName(const std::string & name) const
{
    if(!Count(name)) return nullptr;
    return At(name).get();
}

Ptr<INet> ENetCollection::FindNetByNetId(ENetId netId) const
{
    BuildNetIdLUT();
    auto iter = m_netIdNameMap->find(netId);
    if(iter == m_netIdNameMap->end()) return nullptr;
    return FindNetByName(iter->second);
}

Ptr<INet> ENetCollection::CreateNet(const std::string & name)
{
    if(Count(name)) return nullptr;

    auto net = new ENet(name);
    net->SetNetId(static_cast<ENetId>(m_uidGen.GetNextUid()));
    Insert(name, UPtr<INet>(net));
    ResetNetIdLUT();
    return net;
}

Ptr<INet> ENetCollection::AddNet(UPtr<INet> net)
{
    auto name = NextNetName(net->GetName());
    net->SetName(name);
    net->SetNetId(static_cast<ENetId>(m_uidGen.GetNextUid()));
    Insert(name, std::move(net));
    ResetNetIdLUT();
    return At(name).get();
}


NetIter ENetCollection::GetNetIter() const
{
    return NetIter(new ENetIterator(*this));
}

size_t ENetCollection::Size() const
{
    return BaseCollection::Size();
}

void ENetCollection::Clear()
{
    ResetNetIdLUT();
    m_uidGen.Reset();
    BaseCollection::Clear();
}

Ptr<ENetCollection> ENetCollection::CloneImp() const
{
    return new ENetCollection(*this);
}

void ENetCollection::BuildNetIdLUT() const
{
    if(nullptr == m_netIdNameMap) {
        m_netIdNameMap = std::make_unique<NetIdNameMap>();
        auto iter = GetNetIter();
        while(auto net = iter->Next()) {
            m_netIdNameMap->insert(std::make_pair(net->GetNetId(), net->GetName()));
        }
    }
}

void ENetCollection::ResetNetIdLUT()
{
    m_netIdNameMap.reset();
}

}//namespace ecad