#include "EPadstackDef.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPadstackDef)

#include "interfaces/IPadstackDefData.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EPadstackDef::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackDef, IPadstackDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("data", m_data);
}

template <typename Archive>
ECAD_INLINE void EPadstackDef::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackDef, IPadstackDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("data", m_data);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPadstackDef)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EPadstackDef::EPadstackDef()
 : EPadstackDef(std::string{}, nullptr)
{
}

ECAD_INLINE EPadstackDef::EPadstackDef(std::string name, CPtr<IDatabase> database)
 : EDefinition(std::move(name), database)
 , m_data(nullptr)
{
}

ECAD_INLINE EPadstackDef::~EPadstackDef()
{
}

ECAD_INLINE EPadstackDef::EPadstackDef(const EPadstackDef & other)
{
    *this = other;
}

ECAD_INLINE EPadstackDef & EPadstackDef::operator= (const EPadstackDef & other)
{
    EDefinition::operator=(other);
    m_data = CloneHelper(other.m_data);
    return *this;
}

ECAD_INLINE void EPadstackDef::SetPadstackDefData(UPtr<IPadstackDefData> data)
{
    m_data = std::move(data);
}

ECAD_INLINE Ptr<IPadstackDefData> EPadstackDef::GetPadstackDefData() const
{
    return m_data.get();
}

ECAD_INLINE EDefinitionType EPadstackDef::GetDefinitionType() const
{
    return EDefinitionType::PadstackDef;
}

}//namespace ecad