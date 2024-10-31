#include "EKiCadObjects.h"

namespace ecad::ext::kicad {

ECAD_INLINE void Padstack::SetType(const std::string & str)
{
    if ("smd" == str)
        type = PadType::SMD;
    else if ("thru_hole" == str)
        type = PadType::THRU_HOLE;
    else if ("connect" == str)
        type = PadType::CONNECT;
    else if ("np_thru_hole" == str)
        type = PadType::NP_THRU_HOLE;
    else
        type = PadType::UNKNOWN;
}

ECAD_INLINE void Padstack::SetShape(const std::string & str)
{
    if ("rect" == str)
        shape = PadShape::RECT;
    else if ("roundrect" == str)
        shape = PadShape::ROUNDRECT;
    else if ("circle" == str)
        shape = PadShape::CIRCLE;
    else if ("oval" == str)
        shape = PadShape::OVAL;
    else if ("trapezoid" == str)
        shape = PadShape::TRAPEZOID;
    else
        shape = PadShape::UNKNOWN;
}

ECAD_INLINE void Layer::SetType(const std::string & str)
{
    if ("Top Silk Screen" == str or "Bottom Silk Screen" == str)
        type = LayerType::SILK_SCREEN;
    else if ("Top Solder Paste" == str or "Bottom Solder Paste" == str)
        type = LayerType::SOLDER_PASTE;
    else if ("Top Solder Mask" == str  or "Bottom Solder Mask" == str)
        type = LayerType::SOLDER_MASK;
    else if ("copper" == str)
        type = LayerType::CONDUCTING;
    else if ("core" == str)
        type = LayerType::DIELECTRIC;
}

ECAD_INLINE void Layer::SetGroup(const std::string & str)
{
    if ("power" == str)
        group = LayerGroup::POWER;
    else if ("signal" == str)
        group = LayerGroup::SIGNAL;
    else if ("user" == str)
        group = LayerGroup::USER;
}

ECAD_INLINE Layer & Database::AddLayer(EIndex id, std::string name)
{
    auto & layer = layers.emplace(name, Layer(id, name)).first->second;
    return layer;
}

ECAD_INLINE Net & Database::AddNet(EIndex id, std::string name)
{
    auto & net = nets.emplace(id, Net(id, name)).first->second;
    netLut.emplace(net.name.c_str(), &net);
    return net;
}

ECAD_INLINE Component & Database::AddComponent(std::string name)
{
    auto & comp = components.emplace(name, Component(std::move(name))).first->second;
    return comp;
}

ECAD_INLINE Ptr<Net> Database::FindNet(EIndex id)
{
    auto iter = nets.find(id);
    if (iter == nets.end()) return nullptr;
    return &(iter->second);
}

ECAD_INLINE Ptr<Net> Database::FindNet(const std::string & name)
{
    auto iter = netLut.find(name.c_str());
    if (iter == netLut.end()) return nullptr;
    return iter->second;
}

ECAD_INLINE Ptr<Layer> Database::FindLayer(const std::string & name)
{
    auto iter = layers.find(name);
    if (iter == layers.end()) return nullptr;
    return &(iter->second);
}


} // namespace ecad::ext::kicad