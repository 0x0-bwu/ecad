#include "EPadstackDef.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPadstackDef)

#include "interface/IPadstackDefData.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void EPadstackDef::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackDef, IPadstackDef>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("data", m_data);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPadstackDef)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EPadstackDef::EPadstackDef()
 : EPadstackDef(std::string{}, nullptr)
{
}

EPadstackDef::EPadstackDef(std::string name, CPtr<IDatabase> database)
 : EDefinition(std::move(name), database)
 , m_data(nullptr)
{
}

EPadstackDef::~EPadstackDef()
{
}

EPadstackDef::EPadstackDef(const EPadstackDef & other)
{
    *this = other;
}

EPadstackDef & EPadstackDef::operator= (const EPadstackDef & other)
{
    EDefinition::operator=(other);
    m_data = CloneHelper(other.m_data);
    return *this;
}

void EPadstackDef::SetDatabase(CPtr<IDatabase> database)
{
    EDefinition::SetDatabase(database);
}

void EPadstackDef::SetPadstackDefData(UPtr<IPadstackDefData> data)
{
    m_data = std::move(data);
}

Ptr<IPadstackDefData> EPadstackDef::GetPadstackDefData() const
{
    return m_data.get();
}

EDefinitionType EPadstackDef::GetDefinitionType() const
{
    return EDefinitionType::PadstackDef;
}

}//namespace ecad