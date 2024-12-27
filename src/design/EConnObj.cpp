#include "EConnObj.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EConnObj)

#include "interface/INet.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void EConnObj::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EConnObj, IConnObj>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("net", m_net);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EConnObj)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EConnObj::EConnObj()
 : EConnObj(std::string{})
{
}

EConnObj::EConnObj(std::string name, ENetId net)
 : EObject(std::move(name))
 , m_net(net)
{
}

EConnObj::~EConnObj()
{
}

}//namespace ecad