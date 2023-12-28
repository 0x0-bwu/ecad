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
ECAD_INLINE void EBump::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    ar & boost::serialization::make_nvp("thickness", thickness);
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("material", material);
}

template <typename Archive>
ECAD_INLINE void EBump::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ar & boost::serialization::make_nvp("thickness", thickness);
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("material", material);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EBump)

template <typename Archive>
ECAD_INLINE void EPadstackDefData::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackDefData, IPadstackDefData>();
    ar & boost::serialization::make_nvp("material", m_material);
    ar & boost::serialization::make_nvp("bump_ball", m_solderBumpBall);
    ar & boost::serialization::make_nvp("pads", m_pads);
    ar & boost::serialization::make_nvp("via", m_via);
}

template <typename Archive>
ECAD_INLINE void EPadstackDefData::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackDefData, IPadstackDefData>();
    ar & boost::serialization::make_nvp("material", m_material);
    ar & boost::serialization::make_nvp("bump_ball", m_solderBumpBall);
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

ECAD_INLINE EBump::EBump(const EBump & other)
{
    *this = other;
}

ECAD_INLINE EBump & EBump::operator= (const EBump & other)
{
    thickness = other.thickness;
    shape = CloneHelper(other.shape);
    material = other.material;
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

ECAD_INLINE bool EPadstackDefData::SetPadParameters(ELayerId layerId, UPtr<EShape> shape, const EPoint2D & offset, EFloat rotation)
{
    if(layerId == noLayer) return false;
    int size = static_cast<int>(m_pads.size());
    if(layerId >= size) return false;

    m_pads[layerId].shape = std::move(shape);
    m_pads[layerId].offset = offset;
    m_pads[layerId].rotation = rotation;
    return true;
}

ECAD_INLINE bool EPadstackDefData::GetPadParameters(ELayerId layerId, CPtr<EShape> & shape, EPoint2D & offset, EFloat & rotation) const
{
    if(layerId == noLayer) return false;
    int size = static_cast<int>(m_pads.size());
    if(layerId >= size) return false;

    shape = m_pads[layerId].shape.get();
    offset = m_pads[layerId].offset;
    rotation = m_pads[layerId].rotation;
    return true;
}

ECAD_INLINE bool EPadstackDefData::SetPadParameters(const std::string & layer, UPtr<EShape> shape, const EPoint2D & offset, EFloat rotation)
{
    return SetPadParameters(GetPadLayerId(layer), std::move(shape), offset, rotation);
}

ECAD_INLINE bool EPadstackDefData::GetPadParameters(const std::string & layer, CPtr<EShape> & shape, EPoint2D & offset, EFloat & rotation) const
{
    return GetPadParameters(GetPadLayerId(layer), shape, offset, rotation);
}

ECAD_INLINE void EPadstackDefData::SetViaParameters(UPtr<EShape> shape, const EPoint2D & offset, EFloat rotation)
{
    m_via.shape = std::move(shape);
    m_via.offset = offset;
    m_via.rotation = rotation;
}

ECAD_INLINE void EPadstackDefData::GetViaParameters(CPtr<EShape> & shape, EPoint2D & offset, EFloat & rotation) const
{
    shape = m_via.shape.get();
    offset = m_via.offset;
    rotation = m_via.rotation;
}

ECAD_INLINE void EPadstackDefData::SetTopSolderBumpParameters(UPtr<EShape> shape, EFloat thickness)
{
    auto & topBump = m_solderBumpBall.first;
    topBump.shape = std::move(shape);
    topBump.thickness = thickness;
}

ECAD_INLINE bool EPadstackDefData::GetTopSolderBumpParameters(CPtr<EShape> & shape, EFloat & thickness) const
{
    if (not hasTopSolderBump()) return false;
    const auto & topBump = m_solderBumpBall.first;
    shape = topBump.shape.get();
    thickness = topBump.thickness;
    return true;
}

ECAD_INLINE void EPadstackDefData::SetBotSolderBallParameters(UPtr<EShape> shape, EFloat thickness)
{
    auto & botBall = m_solderBumpBall.second;
    botBall.shape = std::move(shape);
    botBall.thickness = thickness;
}

ECAD_INLINE bool EPadstackDefData::GetBotSolderBallParameters(CPtr<EShape> & shape, EFloat & thickness) const
{
    if (not hasBotSolderBall()) return false;
    const auto & botBall = m_solderBumpBall.second;
    shape = botBall.shape.get();
    thickness = botBall.thickness;
    return true;  
}

ECAD_INLINE void EPadstackDefData::SetTopSolderBumpMaterial(const std::string & material)
{
    m_solderBumpBall.first.material = material;
}

ECAD_INLINE const std::string & EPadstackDefData::GetTopSolderBumpMaterial() const
{
    return m_solderBumpBall.first.material;
}

ECAD_INLINE void EPadstackDefData::SetBotSolderBallMaterial(const std::string & material)
{
    m_solderBumpBall.second.material = material;
}

ECAD_INLINE const std::string & EPadstackDefData::GetBotSolderBallMaterial() const
{
    return m_solderBumpBall.second.material;
}

ECAD_INLINE bool EPadstackDefData::hasTopSolderBump() const
{
    const auto & topBump = m_solderBumpBall.first;
    if (nullptr == topBump.shape) return false;
    return true;
}

ECAD_INLINE bool EPadstackDefData::hasBotSolderBall() const
{
    const auto & botBall = m_solderBumpBall.second;
    if (nullptr == botBall.shape) return false;
    return true;
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