#ifndef ECAD_HEADER_ONLY
#include "ENet.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ENet)
#endif

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ENet::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ENet, INet>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("net_id", m_netId);
}

template <typename Archive>
ECAD_INLINE void ENet::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ENet, INet>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("net_id", m_netId);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ENet)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ENet::ENet()
 : ENet(std::string{})
{
}

ECAD_INLINE ENet::ENet(std::string name)
 : EObject(std::move(name))
{
}

ECAD_INLINE ENet::~ENet()
{
}

ECAD_INLINE ENet::ENet(const ENet & other)
{
    *this = other;
}

ECAD_INLINE ENet & ENet::operator= (const ENet & other)
{
    EObject::operator=(other);
    return *this;
}

ECAD_INLINE ENetId ENet::GetNetId() const
{
    return m_netId;
}

ECAD_INLINE void ENet::SetNetId(ENetId id)
{
    m_netId = id;
}

ECAD_INLINE Ptr<ENet> ENet::CloneImp() const
{
    return new ENet(*this);
}

}//namespace ecad