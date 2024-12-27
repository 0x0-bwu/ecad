#include "ENet.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ENet)

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void ENet::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ENet, INet>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("net_id", m_netId);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ENet)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ENet::ENet()
 : ENet(std::string{})
{
}

ENet::ENet(std::string name)
 : EObject(std::move(name))
{
}

ENet::~ENet()
{
}

ENetId ENet::GetNetId() const
{
    return m_netId;
}

void ENet::SetNetId(ENetId id)
{
    m_netId = id;
}

Ptr<ENet> ENet::CloneImp() const
{
    return new ENet(*this);
}

}//namespace ecad