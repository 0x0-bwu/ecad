#include "EComponentDefPin.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentDefPin)

#include "interface/IPadstackDef.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void EComponentDefPin::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDefPin, IComponentDefPin>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("type", m_type);
    ar & boost::serialization::make_nvp("location", m_location);
    ar & boost::serialization::make_nvp("padstack_def", m_padstackDef);
    ar & boost::serialization::make_nvp("layer", m_layer);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponentDefPin)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EComponentDefPin::EComponentDefPin()
 : EObject(std::string{})
{
}

EComponentDefPin::EComponentDefPin(const std::string & name)
 : EObject(name)
{
}

EComponentDefPin::~EComponentDefPin()
{
}

void EComponentDefPin::SetIOType(EPinIOType type)
{
    m_type = type;
}

EPinIOType EComponentDefPin::GetIOType() const
{
    return m_type;
}

void EComponentDefPin::SetLocation(EPoint2D location)
{
    m_location = location;
}

const EPoint2D & EComponentDefPin::GetLocation() const
{
    return m_location;
}

void EComponentDefPin::SetPadstackDef(CPtr<IPadstackDef> def)
{
    m_padstackDef = def;
}
    
CPtr<IPadstackDef> EComponentDefPin::GetPadstackDef() const
{
    return m_padstackDef;
}

void EComponentDefPin::SetLayerId(ELayerId lyrId)
{
    m_layer = lyrId;
}
ELayerId EComponentDefPin::GetLayerId() const
{
    return m_layer;
}

}//namespace ecad