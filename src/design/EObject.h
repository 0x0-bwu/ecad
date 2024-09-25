#pragma once
#include "basic/ECadCommon.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>

#ifdef ECAD_BOOST_SERIALIZATION_INCLUDE_UUID
#include <boost/uuid/uuid_serialize.hpp>
#endif//ECAD_BOOST_SERIALIZATION_INCLUDE_UUID
namespace ecad {

using EUuidHash = boost::hash<boost::uuids::uuid>;
class ECAD_API EObject
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EObject();
    explicit EObject(std::string name);
    virtual ~EObject();

    ///Copy
    EObject(const EObject & other);
    EObject & operator= (const EObject & other);

    ///Move
    EObject(EObject && other);
    EObject & operator= (EObject && other);

    bool operator== (const EObject & other) const;
    bool operator!= (const EObject & other) const;

    void SetName(std::string name);
    void Print(std::ostream & os) const;
    const std::string & GetName() const;
    const EUuid & Uuid() const;
    std::string sUuid() const;

protected:
    std::string m_name;
    EUuid m_uuid;    
};

ECAD_ALWAYS_INLINE void EObject::SetName(std::string name)
{
    m_name = std::move(name);
}

ECAD_ALWAYS_INLINE void EObject::Print(std::ostream & os) const
{
    os << "NAME: " << GetName() << ECAD_EOL;
    os << "UUID: " << sUuid()   << ECAD_EOL; 
}

ECAD_ALWAYS_INLINE const std::string & EObject::GetName() const
{
    return m_name;
} 

ECAD_ALWAYS_INLINE const EUuid & EObject::Uuid() const
{
    return m_uuid;
}

ECAD_ALWAYS_INLINE std::string EObject::sUuid() const
{
    return boost::uuids::to_string(m_uuid);
}

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EObject)