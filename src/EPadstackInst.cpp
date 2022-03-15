#ifndef ECAD_HEADER_ONLY
#include "EPadstackInst.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPadstackInst)
#endif

#include "interfaces/IPadstackDefData.h"
#include "interfaces/IPadstackDef.h"
#include "interfaces/ILayerMap.h"
#include "interfaces/ILayer.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void EPadstackInst::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackInst, IPadstackInst>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EConnObj);
    ar & boost::serialization::make_nvp("padstack_def", m_def);
    ar & boost::serialization::make_nvp("layer_map", m_layerMap);
    ar & boost::serialization::make_nvp("top_layer", m_topLyr);
    ar & boost::serialization::make_nvp("bot_layer", m_botLyr);
    ar & boost::serialization::make_nvp("transform", m_transform);
    ar & boost::serialization::make_nvp("is_layout_pin", m_isLayoutPin);
}

template <typename Archive>
ECAD_INLINE void EPadstackInst::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackInst, IPadstackInst>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EConnObj);
    ar & boost::serialization::make_nvp("padstack_def", m_def);
    ar & boost::serialization::make_nvp("layer_map", m_layerMap);
    ar & boost::serialization::make_nvp("top_layer", m_topLyr);
    ar & boost::serialization::make_nvp("bot_layer", m_botLyr);
    ar & boost::serialization::make_nvp("transform", m_transform);
    ar & boost::serialization::make_nvp("is_layout_pin", m_isLayoutPin);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPadstackInst)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EPadstackInst::EPadstackInst()
 : EPadstackInst(std::string{}, nullptr, noNet)
{
}

ECAD_INLINE EPadstackInst::EPadstackInst(std::string name, CPtr<IPadstackDef> def, ENetId net)
 : EConnObj(std::move(name), net)
 , m_def(def)
{
}

ECAD_INLINE EPadstackInst::~EPadstackInst()
{
}

ECAD_INLINE EPadstackInst::EPadstackInst(const EPadstackInst & other)
{
    *this = other;
}

ECAD_INLINE EPadstackInst & EPadstackInst::operator= (const EPadstackInst & other)
{
    m_def = other.m_def;
    m_layerMap = other.m_layerMap;
    m_topLyr = other.m_topLyr;
    m_botLyr = other.m_botLyr;
    m_transform = other.m_transform;
    return *this;
}

ECAD_INLINE void EPadstackInst::SetNet(ENetId net)
{
    EConnObj::SetNet(net);
}

ECAD_INLINE ENetId EPadstackInst::GetNet() const
{
    return EConnObj::GetNet();
}

ECAD_INLINE void EPadstackInst::SetLayerRange(ELayerId top, ELayerId bot)
{
    m_topLyr = top; m_botLyr = bot;
}

ECAD_INLINE void EPadstackInst::SetLayerMap(CPtr<ILayerMap> layerMap)
{
    m_layerMap = layerMap;
}

ECAD_INLINE CPtr<ILayerMap> EPadstackInst::GetLayerMap() const
{
    return m_layerMap;
}

ECAD_INLINE void EPadstackInst::GetLayerRange(ELayerId & top, ELayerId & bot) const
{
    top = m_topLyr; bot = m_botLyr;
}

ECAD_INLINE CPtr<IPadstackDef> EPadstackInst::GetPadstackDef() const
{
    return m_def;
}

ECAD_INLINE bool EPadstackInst::isLayoutPin() const
{
    return m_isLayoutPin;
}

ECAD_INLINE void EPadstackInst::SetIsLayoutPin(bool isPin)
{
    m_isLayoutPin = isPin;
}

ECAD_INLINE UPtr<EShape> EPadstackInst::GetLayerShape(ELayerId lyr) const
{
    auto min = std::min(m_topLyr, m_botLyr);
    auto max = std::max(m_topLyr, m_botLyr);
    if(lyr < min || lyr > max) return nullptr;

    auto layerMap = GetLayerMap();
    if(nullptr == layerMap) return nullptr;

    auto padstackDef = GetPadstackDef();
    if(nullptr == padstackDef) return nullptr;

    auto padstackDefData = padstackDef->GetPadstackDefData();
    if(nullptr == padstackDefData) return nullptr;

    auto toLyr = layerMap->GetMappingForward(lyr);

    CPtr<EShape> shape = nullptr;
    EValue rotation(0);
    EPoint2D offset(0, 0);

    bool res = padstackDefData->GetPadParameters(toLyr, shape, offset, rotation);
    if(!res || nullptr == shape)
        padstackDefData->GetViaParameters(shape, offset, rotation);
    
    if(nullptr == shape) return nullptr;

    auto lyrShape = shape->Clone();
    auto transform = GetTransform();
    transform.Append(makeETransform2D(1.0, rotation, offset));
    lyrShape->Transform(transform);

    return std::move(lyrShape);    
}

}//namespace ecad