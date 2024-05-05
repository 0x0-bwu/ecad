#include "EComponentDefPin.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponentDefPin)

#include "interface/IPadstackDef.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EComponentDefPin::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponentDefPin, IComponentDefPin>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("type", m_type);
    ar & boost::serialization::make_nvp("location", m_location);
    ar & boost::serialization::make_nvp("padstack_def", m_padstackDef);
    ar & boost::serialization::make_nvp("layer", m_layer);
}

template <typename Archive>
ECAD_INLINE void EComponentDefPin::load(Archive & ar, const unsigned int version)
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

ECAD_INLINE EComponentDefPin::EComponentDefPin()
 : EObject(std::string{})
{
}

ECAD_INLINE EComponentDefPin::EComponentDefPin(const std::string & name)
 : EObject(name)
{
}

ECAD_INLINE EComponentDefPin::~EComponentDefPin()
{
}

ECAD_INLINE void EComponentDefPin::SetIOType(EPinIOType type)
{
    m_type = type;
}

ECAD_INLINE EPinIOType EComponentDefPin::GetIOType() const
{
    return m_type;
}

ECAD_INLINE void EComponentDefPin::SetLocation(EPoint2D location)
{
    m_location = location;
}

ECAD_INLINE const EPoint2D & EComponentDefPin::GetLocation() const
{
    return m_location;
}

ECAD_INLINE void EComponentDefPin::SetPadstackDef(CPtr<IPadstackDef> def)
{
    m_padstackDef = def;
}
    
ECAD_INLINE CPtr<IPadstackDef> EComponentDefPin::GetPadstackDef() const
{
    return m_padstackDef;
}

ECAD_INLINE void EComponentDefPin::SetLayerId(ELayerId lyrId)
{
    m_layer = lyrId;
}
ECAD_INLINE ELayerId EComponentDefPin::GetLayerId() const
{
    return m_layer;
}

}//namespace ecad