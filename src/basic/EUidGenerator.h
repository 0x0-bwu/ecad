#pragma once
#include "ECadCommon.h"
#include <atomic>
namespace ecad {

using EUid = int32_t;
class ECAD_API EUidGenerator
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        EUid uid;
        ar & boost::serialization::make_nvp("uid", uid);
        m_uid.store(uid);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
public:
    EUid GetNextUid() { return m_uid++; }
    void Reset() { m_uid.store(0); }
private:
    std::atomic_int32_t m_uid = 0;
};

}//namespace ecad