#include "EPadstackInst.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPadstackInst)

#include "interface/IPadstackDefData.h"
#include "interface/IPadstackDef.h"
#include "interface/ILayerMap.h"
#include "interface/ILayer.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void EPadstackInst::serialize(Archive & ar, const unsigned int version)
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

EPadstackInst::EPadstackInst()
 : EPadstackInst(std::string{}, nullptr, noNet)
{
}

EPadstackInst::EPadstackInst(std::string name, CPtr<IPadstackDef> def, ENetId net)
 : EConnObj(std::move(name), net)
 , m_def(def)
{
}

EPadstackInst::~EPadstackInst()
{
}

void EPadstackInst::SetNet(ENetId net)
{
    EConnObj::SetNet(net);
}

ENetId EPadstackInst::GetNet() const
{
    return EConnObj::GetNet();
}

void EPadstackInst::SetLayerRange(ELayerId top, ELayerId bot)
{
    m_topLyr = top; m_botLyr = bot;
}

void EPadstackInst::SetLayerMap(CPtr<ILayerMap> layerMap)
{
    m_layerMap = layerMap;
}

CPtr<ILayerMap> EPadstackInst::GetLayerMap() const
{
    return m_layerMap;
}

void EPadstackInst::GetLayerRange(ELayerId & top, ELayerId & bot) const
{
    top = m_topLyr; bot = m_botLyr;
}

CPtr<IPadstackDef> EPadstackInst::GetPadstackDef() const
{
    return m_def;
}

bool EPadstackInst::isLayoutPin() const
{
    return m_isLayoutPin;
}

void EPadstackInst::SetIsLayoutPin(bool isPin)
{
    m_isLayoutPin = isPin;
}

UPtr<EShape> EPadstackInst::GetLayerShape(ELayerId lyr) const
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
    EFloat rotation(0);
    EPoint2D offset(0, 0);

    bool res = padstackDefData->GetPadParameters(toLyr, shape, offset, rotation);
    if(!res || nullptr == shape)
        padstackDefData->GetViaParameters(shape, offset, rotation);
    
    if(nullptr == shape) return nullptr;

    auto lyrShape = shape->Clone();
    auto transform = GetTransform();
    transform.Append(makeETransform2D(1.0, rotation, offset));
    lyrShape->Transform(transform);

    return lyrShape;   
}

}//namespace ecad