#include "EKiCadObjects.h"

namespace ecad::ext::kicad {


void Stroke::SetType(const std::string & str)
{
    if ("solid" == str)
        type = Type::SOLID;
}

void Stroke::SetFill(const std::string & str)
{
    if ("solid" == str)
        fill = Fill::SOLID;
}

void Pad::SetType(const std::string & str)
{
    if ("smd" == str)
        type = Type::SMD;
    else if ("thru_hole" == str)
        type = Type::THRU_HOLE;
    else if ("connect" == str)
        type = Type::CONNECT;
    else if ("np_thru_hole" == str)
        type = Type::NP_THRU_HOLE;
    else
        type = Type::UNKNOWN;
}

void Pad::SetShape(const std::string & str)
{
    if ("rect" == str)
        shape = Shape::RECT;
    else if ("roundrect" == str)
        shape = Shape::ROUNDRECT;
    else if ("circle" == str)
        shape = Shape::CIRCLE;
    else if ("oval" == str)
        shape = Shape::OVAL;
    else if ("trapezoid" == str)
        shape = Shape::TRAPEZOID;
    else
        shape = Shape::UNKNOWN;
}

void Layer::SetType(const std::string & str)
{
    if ("Top Silk Screen" == str or "Bottom Silk Screen" == str)
        type = Type::SILK_SCREEN;
    else if ("Top Solder Paste" == str or "Bottom Solder Paste" == str)
        type = Type::SOLDER_PASTE;
    else if ("Top Solder Mask" == str  or "Bottom Solder Mask" == str)
        type = Type::SOLDER_MASK;
    else if ("copper" == str)
        type = Type::CONDUCTING;
    else if ("core" == str)
        type = Type::DIELECTRIC;
    else if ("mixed" == str)
        type = Type::MIXED;
}

void Layer::SetGroup(const std::string & str)
{
    if ("power" == str)
        group = Group::POWER;
    else if ("signal" == str)
        group = Group::SIGNAL;
    else if ("user" == str)
        group = Group::USER;
}

Layer & Database::AddLayer(EIndex id, std::string name)
{
    auto & layer = layers.emplace(name, Layer(id, name)).first->second;
    return layer;
}

Net & Database::AddNet(EIndex id, std::string name)
{
    auto & net = nets.emplace(id, Net(id, name)).first->second;
    netLut.emplace(net.name.c_str(), &net);
    return net;
}

Component & Database::AddComponent(std::string name)
{
    auto & comp = components.emplace(name, Component(std::move(name))).first->second;
    return comp;
}

Ptr<Net> Database::FindNet(EIndex id)
{
    auto iter = nets.find(id);
    if (iter == nets.end()) return nullptr;
    return &(iter->second);
}

Ptr<Net> Database::FindNet(const std::string & name)
{
    auto iter = netLut.find(name.c_str());
    if (iter == netLut.end()) return nullptr;
    return iter->second;
}

Ptr<Layer> Database::FindLayer(const std::string & name)
{
    auto iter = layers.find(name);
    if (iter == layers.end()) return nullptr;
    return &(iter->second);
}


} // namespace ecad::ext::kicad