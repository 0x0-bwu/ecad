#include "EPadstackDefData.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPadstackDefData)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPad)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EVia)

#include "EShape.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void EVia::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    ar & boost::serialization::make_nvp("rotation", rotation);
    ar & boost::serialization::make_nvp("offset", offset);
    ar & boost::serialization::make_nvp("shape", shape);
}

template <typename Archive>
ECAD_INLINE void EVia::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ar & boost::serialization::make_nvp("rotation", rotation);
    ar & boost::serialization::make_nvp("offset", offset);
    ar & boost::serialization::make_nvp("shape", shape);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EVia)

template <typename Archive>
ECAD_INLINE void EPad::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    ar & boost::serialization::make_nvp("layer", lyr);
    ar & boost::serialization::make_nvp("rotation", rotation);
    ar & boost::serialization::make_nvp("offset", offset);
    ar & boost::serialization::make_nvp("shape", shape);
}

template <typename Archive>
ECAD_INLINE void EPad::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ar & boost::serialization::make_nvp("layer", lyr);
    ar & boost::serialization::make_nvp("rotation", rotation);
    ar & boost::serialization::make_nvp("offset", offset);
    ar & boost::serialization::make_nvp("shape", shape);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPad)

template <typename Archive>
ECAD_INLINE void EPadstackDefData::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackDefData, IPadstackDefData>();
    ar & boost::serialization::make_nvp("material", m_material);
    ar & boost::serialization::make_nvp("pads", m_pads);
    ar & boost::serialization::make_nvp("via", m_via);
}

template <typename Archive>
ECAD_INLINE void EPadstackDefData::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackDefData, IPadstackDefData>();
    ar & boost::serialization::make_nvp("material", m_material);
    ar & boost::serialization::make_nvp("pads", m_pads);
    ar & boost::serialization::make_nvp("via", m_via);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPadstackDefData)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EVia::EVia(const EVia & other)
{
    *this = other;
}

ECAD_INLINE EVia & EVia::operator= (const EVia & other)
{
    rotation = other.rotation;
    offset = other.offset;
    shape = CloneHelper(other.shape);
    return *this;
}

ECAD_INLINE EPad::EPad(const EPad & other)
{
    *this = other;
}

ECAD_INLINE EPad & EPad::operator= (const EPad & other)
{
    lyr = other.lyr;
    rotation = other.rotation;
    offset = other.offset;
    shape = CloneHelper(other.shape);
    return *this;
}

ECAD_INLINE EPadstackDefData::EPadstackDefData(const EPadstackDefData & other)
{
    *this = other;
}

ECAD_INLINE EPadstackDefData & EPadstackDefData::operator= (const EPadstackDefData & other)
{
    m_material = other.m_material;
    m_pads = other.m_pads;
    m_via = other.m_via;
    return *this;
}

ECAD_INLINE std::string EPadstackDefData::GetMaterial() const
{
    return m_material;
}
ECAD_INLINE void EPadstackDefData::SetMaterial(const std::string & material)
{
    m_material = material;
}

ECAD_INLINE void EPadstackDefData::SetLayers(const std::vector<std::string> & layers)
{
    m_pads.resize(layers.size());
    for(size_t i = 0; i < layers.size(); ++i)
        m_pads[i].lyr = layers[i];
}

ECAD_INLINE bool EPadstackDefData::SetPadParameters(ELayerId layerId, UPtr<EShape> shape, const EPoint2D & offset, EValue rotation)
{
    if(layerId == noLayer) return false;
    int size = static_cast<int>(m_pads.size());
    if(layerId >= size) return false;

    m_pads[layerId].shape = std::move(shape);
    m_pads[layerId].offset = offset;
    m_pads[layerId].rotation = rotation;
    return true;
}

ECAD_INLINE bool EPadstackDefData::GetPadParameters(ELayerId layerId, CPtr<EShape> & shape, EPoint2D & offset, EValue & rotation) const
{
    if(layerId == noLayer) return false;
    int size = static_cast<int>(m_pads.size());
    if(layerId >= size) return false;

    shape = m_pads[layerId].shape.get();
    offset = m_pads[layerId].offset;
    rotation = m_pads[layerId].rotation;
    return true;
}

ECAD_INLINE bool EPadstackDefData::SetPadParameters(const std::string & layer, UPtr<EShape> shape, const EPoint2D & offset, EValue rotation)
{
    return SetPadParameters(GetPadLayerId(layer), std::move(shape), offset, rotation);
}

ECAD_INLINE bool EPadstackDefData::GetPadParameters(const std::string & layer, CPtr<EShape> & shape, EPoint2D & offset, EValue & rotation) const
{
    return GetPadParameters(GetPadLayerId(layer), shape, offset, rotation);
}

ECAD_INLINE void EPadstackDefData::SetViaParameters(UPtr<EShape> shape, const EPoint2D & offset, EValue rotation)
{
    m_via.shape = std::move(shape);
    m_via.offset = offset;
    m_via.rotation = rotation;
}

ECAD_INLINE void EPadstackDefData::GetViaParameters(CPtr<EShape> & shape, EPoint2D & offset, EValue & rotation) const
{
    shape = m_via.shape.get();
    offset = m_via.offset;
    rotation = m_via.rotation;
}

ECAD_INLINE ELayerId EPadstackDefData::GetPadLayerId(const std::string & layer) const
{
    for(size_t i = 0; i < m_pads.size(); ++i){
        if(layer == m_pads[i].lyr)
            return static_cast<ELayerId>(i);
    }
    return noLayer;
}

}//namespace ecad