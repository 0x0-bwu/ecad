#include "ECadExtKiCadHandler.h"
#include "EDataMgr.h"

namespace ecad::ext::kicad {

using namespace generic;

ECAD_INLINE ECadExtKiCadHandler::ECadExtKiCadHandler(const std::string & kicadFile)
 : m_filename(kicadFile)
{
    m_functions.emplace("layers"   , std::bind(&ECadExtKiCadHandler::ExtractLayer    , this, std::placeholders::_1));
    m_functions.emplace("setup"    , std::bind(&ECadExtKiCadHandler::ExtractSetup    , this, std::placeholders::_1));
    m_functions.emplace("stackup"  , std::bind(&ECadExtKiCadHandler::ExtractStackup  , this, std::placeholders::_1));
    m_functions.emplace("net"      , std::bind(&ECadExtKiCadHandler::ExtractNet      , this, std::placeholders::_1));
    m_functions.emplace("footprint", std::bind(&ECadExtKiCadHandler::ExtractFootprint, this, std::placeholders::_1));
    m_functions.emplace("segment"  , std::bind(&ECadExtKiCadHandler::ExtractSegment  , this, std::placeholders::_1));
    m_functions.emplace("zone"     , std::bind(&ECadExtKiCadHandler::ExtractZone     , this, std::placeholders::_1));
    m_functions.emplace("via"      , std::bind(&ECadExtKiCadHandler::ExtractVia      , this, std::placeholders::_1));
    m_functions.emplace("pad"      , std::bind(&ECadExtKiCadHandler::ExtractPad      , this, std::placeholders::_1));
    m_functions.emplace("fp_arc"   , std::bind(&ECadExtKiCadHandler::ExtractArc      , this, std::placeholders::_1));
    m_functions.emplace("gr_arc"   , std::bind(&ECadExtKiCadHandler::ExtractArc      , this, std::placeholders::_1));
    m_functions.emplace("fp_line"  , std::bind(&ECadExtKiCadHandler::ExtractLine     , this, std::placeholders::_1));
    m_functions.emplace("gr_line"  , std::bind(&ECadExtKiCadHandler::ExtractLine     , this, std::placeholders::_1));
    m_functions.emplace("fp_poly"  , std::bind(&ECadExtKiCadHandler::ExtractPoly     , this, std::placeholders::_1));
    m_functions.emplace("fp_circle", std::bind(&ECadExtKiCadHandler::ExtractCircle   , this, std::placeholders::_1));
    m_functions.emplace("gr_circle", std::bind(&ECadExtKiCadHandler::ExtractCircle   , this, std::placeholders::_1));
}

ECAD_INLINE Ptr<IDatabase> ECadExtKiCadHandler::CreateDatabase(const std::string & name, Ptr<std::string> err)
{
    auto & eMgr = EDataMgr::Instance();
    m_database = eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    if (not (ExtractKiCadObjects(err))) return nullptr;
    if (not (CreateEcadObjects(err))) return nullptr;

    return m_database;
}

ECAD_INLINE bool ECadExtKiCadHandler::ExtractKiCadObjects(Ptr<std::string> err)
{
    Tree tree;
    EKiCadParser parser;
    if (not parser(m_filename.data(), tree)) {
        if(err) *err = fmt::Fmt2Str("Error: failed to parse %1%.", m_filename);
        return false; 
    }
    
    m_kicad.reset(new Database);
    m_current.comp = m_kicad.get();
    for (auto & node : tree.branches)
        ExtractNode(node);
    return true;
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractNode(const Tree & node)
{
    auto iter = m_functions.find(node.value);
    if (iter != m_functions.cend())
        iter->second(node);
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractLayer(const Tree & node)
{
    for (const auto & sub : node.branches) {   
        EIndex id = std::stoi(sub.value);         
        auto name = sub.branches.at(0).value;
        auto & layer = m_kicad->AddLayer(id, name);
        layer.SetGroup(sub.branches.at(1).value);
        if (sub.branches.size() > 2)
            layer.attr = sub.branches.at(2).value;
    }
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractSetup(const Tree & node)
{
    for (const auto & sub : node.branches)
        ExtractNode(sub);
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractStackup(const Tree & node)
{
    for (const auto & sub : node.branches) {
        if ("layer" == sub.value) {
            auto iter = sub.branches.begin();
            if (auto layer = m_kicad->FindLayer(iter->value); layer) {
                for (iter = std::next(iter); iter != sub.branches.end(); ++iter) {
                    if ("type" == iter->value)
                        layer->SetType(iter->branches.front().value);
                    else if ("thickness" == iter->value)
                        GetValue(iter->branches, layer->thickness);
                    else if ("material" == iter->value)
                        GetValue(iter->branches, layer->material);
                    else if ("epsilon_r" == iter->value)
                        GetValue(iter->branches, layer->epsilonR);
                    else if ("loss_tangent" == iter->value)
                        GetValue(iter->branches, layer->lossTangent);
                }
            }
        }
    }
}


ECAD_INLINE void ECadExtKiCadHandler::ExtractNet(const Tree & node)
{
    EIndex netId{invalidIndex};
    auto & netName = node.branches.at(1).value;
    GetValue(node.branches, netId);
    m_kicad->AddNet(netId, netName);
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractFootprint(const Tree & node)
{
    auto iter = node.branches.begin();
    auto & comp = m_kicad->AddComponent(iter->value);
    m_current.comp = &comp;
    for (iter = std::next(iter); iter != node.branches.end(); ++iter) {
        const auto & branches = iter->branches;
        if ("layer" == iter->value) {
            auto layer = m_kicad->FindLayer(branches.front().value);
            comp.layerId = layer ? layer->id : invalidIndex;
            comp.flipped = (0 != comp.layerId);
        }
        else if ("at" == iter->value)
            TryGetValue(branches, comp.location[0], comp.location[1], comp.angle);
        else if ("pad" == iter->value)
            ExtractPad(*iter);
        else
            ExtractNode(*iter);         
    }
    m_current.comp = m_kicad.get();
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractSegment(const Tree & node)
{
    Segment segment;
    for (const auto & sub : node.branches) {
        if ("start" == sub.value)
            GetValue(sub.branches, segment.start[0], segment.start[1]);
        else if ("end" == sub.value)
            GetValue(sub.branches, segment.end[0], segment.end[1]);
        else if ("width" == sub.value)
            GetValue(sub.branches, segment.width);
        else if ("net" == sub.value)
            GetValue(sub.branches, segment.netId);
        else if ("layer" == sub.value) {
            if (auto layer = m_kicad->FindLayer(sub.branches.front().value); layer)
                segment.layerId = layer->id;
        }
    }
    m_current.comp->segments.emplace_back(std::move(segment));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractZone(const Tree & node)
{
    Zone zone;
    for (const auto & sub : node.branches) {
        if ("net" == sub.value)
            GetValue(sub.branches, zone.netId);
        else if ("layer" == sub.value) {
            if (auto layer = m_kicad->FindLayer(sub.branches.front().value); layer)
                zone.layerId = layer->id;
        }
        else if ("polygon" == sub.value) {
            for (const auto & polygon : sub.branches) {
                if ("pts" == polygon.value)
                    ExtractPoints(polygon, zone.polygon);
            }
        }
        else if ("filled_polygon" == sub.value) {
           for (const auto & polygon : sub.branches) {
                if ("pts" == polygon.value)
                    ExtractPoints(polygon, zone.filledPolygons.emplace_back(Points{}));
            }
        }
    }
    m_current.comp->zones.emplace_back(std::move(zone));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractVia(const Tree & node)
{
    Via via;
    via.type = Via::Type::THROUGH;
    for (const auto & sub : node.branches) {
        if ("at" == sub.value)
            GetValue(sub.branches, via.pos[0], via.pos[1]);
        else if ("size" == sub.value)
            GetValue(sub.branches, via.size);
        else if ("net" == sub.value)
            GetValue(sub.branches, via.netId);
        else if ("layers" == sub.value) {
            for (size_t i = 0; i < via.layers.size(); ++i) {
                if (auto layer = m_kicad->FindLayer(sub.branches.at(i).value); layer)
                    via.layers[i] = layer->id;
            }
        }
        else if ("micro" == sub.value)
            via.type = Via::Type::MICRO;
        else if ("bline" == sub.value)
            via.type = Via::Type::BLIND_BURIED;
    }
    m_current.comp->vias.emplace_back(std::move(via));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractCircle(const Tree & node)
{
    Circle circle;
    for (const auto & sub : node.branches) {
        if ("center" == sub.value)
            GetValue(sub.branches, circle.center[0], circle.center[1]);
        else if ("end" == sub.value)
            GetValue(sub.branches, circle.end[0], circle.end[1]);
        else if ("stroke" == sub.value)
            ExtractStroke(sub, circle);
    }
    m_current.comp->circles.emplace_back(std::move(circle));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractPoly(const Tree & node)
{
    Poly poly;
    for (const auto & sub : node.branches) {
        if ("pts" == sub.value)
            ExtractPoints(sub, poly.shape);
        else if ("stroke" == sub.value)
            ExtractStroke(sub, poly);
    }
    m_current.comp->polys.emplace_back(std::move(poly));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractArc(const Tree & node)
{
    Arc arc;
    GetValue(node.branches.at(0).branches, arc.start[0], arc.start[1]);
    GetValue(node.branches.at(1).branches, arc.end[0], arc.end[1]);
    GetValue(node.branches.at(2).branches, arc.angle);
    GetValue(node.branches.at(4).branches, arc.width);
    m_current.comp->arcs.emplace_back(std::move(arc));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractLine(const Tree & node)
{
    Line line;
    for (const auto & sub : node.branches) {
        if ("start" == sub.value)
            GetValue(sub.branches, line.start[0], line.start[1]);
        else if ("end" == sub.value)
            GetValue(sub.branches, line.end[0], line.end[1]);
        else if ("stroke" == sub.value)
            ExtractStroke(sub, line);
    }
    m_current.comp->lines.emplace_back(std::move(line));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractPad(const Tree & node)
{
    Pad pad;
    auto iter = node.branches.begin();
    pad.name = iter->value; iter++;
    if (pad.name.empty())
        pad.name = "Unnamed" + std::to_string(m_current.noNamePadId++);
    pad.SetType(iter->value); iter++;
    pad.SetShape(iter->value); iter++;
    for (; iter != node.branches.end(); ++iter) {
        if ("at" == iter->value)
            TryGetValue(iter->branches, pad.pos[0], pad.pos[1], pad.angle);
        else if ("size" == iter->value)
            GetValue(iter->branches, pad.size[0], pad.size[1]);
        else if ("layers" == iter->value) {
            for (const auto & lyrNode : iter->branches) {
                if (auto layer = m_kicad->FindLayer(lyrNode.value); layer)
                    pad.layers.emplace_back(layer->id);
            }
        }
        else if ("roundrect_rratio" == iter->value)
            GetValue(iter->branches, pad.roundrectRatio);
        else if ("net" == iter->value)
            GetValue(iter->branches, pad.netId);
        else if ("primitives" == iter->value) {
            for (const auto & primNode : iter->branches) {
                if ("gr_poly" == primNode.value) {
                    ExtractPoints(primNode.branches.front(), pad.shapePolygon);
                }
            }
        }
    }
    if (Pad::Shape::OVAL == pad.shape) {
        pad.shapePolygon.emplace_back(pad.size[1] / -2, 0);
        pad.shapePolygon.emplace_back(pad.size[1] / 2, 0);
    }
    else if (Pad::Shape::RECT == pad.shape) {
        auto x1 = pad.size[0] / -2;
        auto y1 = pad.size[1] / 2;
        auto x2 = pad.size[0] / 2;
        auto y2 = pad.size[1] / -2;
        pad.shapePolygon.emplace_back(x1, y1);
        pad.shapePolygon.emplace_back(x2, y1);
        pad.shapePolygon.emplace_back(x2, y2);
        pad.shapePolygon.emplace_back(x1, y2);
    }
    m_current.comp->pads.emplace_back(std::move(pad));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractPoints(const Tree & node, std::vector<FPoint2D> & points)
{
    points.clear();
    points.reserve(node.branches.size());
    for (const auto & sub : node.branches) {
        EFloat x{0}, y{0};
        GetValue(sub.branches, x, y);
        points.emplace_back(x, y);
    }
    if (points.back() == points.front())
        points.pop_back();
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractStroke(const Tree & node, Stroke & stroke)
{
    for (const auto & sub : node.branches) {
        if ("width" == sub.value)
            GetValue(sub.branches, stroke.width);
        else if ("type" == sub.value)
            stroke.SetType(sub.branches.front().value);
        else if ("fill" == sub.value)
            stroke.SetFill(sub.branches.front().value);
        else if ("layer" == sub.value) {
            if (auto layer = m_kicad->FindLayer(sub.branches.front().value); layer)
                stroke.layer = layer->id;
        }
    }
}

ECAD_INLINE bool ECadExtKiCadHandler::CreateEcadObjects(Ptr<std::string> err)
{
    ECoordUnits coordUnits(ECoordUnits::Unit::Millimeter);
    m_database->SetCoordUnits(coordUnits);

    // top call
    auto & eMgr = EDataMgr::Instance();
    auto cell = eMgr.CreateCircuitCell(m_database, m_database->GetName());
    auto layout = cell->GetLayoutView();

    CreateEcadNets(layout);
    CreateEcadLayers(layout);

    return true;
}

ECAD_INLINE void ECadExtKiCadHandler::CreateEcadLayers(Ptr<ILayoutView> layout)
{
    auto & eMgr = EDataMgr::Instance();

    EFloat elevation{0};
    std::vector<UPtr<ILayer> > layers;
    for (const auto & [kName, kLayer] : m_kicad->layers) {
        if (kLayer.id > ECAD_KICAD_PCB_LAYER_BOTTOM_ADHES_ID)
            continue;
        ELayerType type = kLayer.type == Layer::Type::CONDUCTING ?
                          ELayerType::ConductingLayer : ELayerType::DielectricLayer;
        auto conductingMat = kLayer.material.empty() ? sDefaultConductingMat : kLayer.material;
        auto dielectricMat = kLayer.material.empty() ? sDefaultDielectricMat : kLayer.material;
        auto layer = eMgr.CreateStackupLayer(kName, type, elevation, kLayer.thickness, conductingMat, dielectricMat);
        if (nullptr == layer) {
            //todo, error
            continue;
        }
        m_lut.layer.emplace(kLayer.id, layer.get());
        layers.emplace_back(std::move(layer));
        elevation -= kLayer.thickness;
    }
    layout->AppendLayers(std::move(layers));
}

ECAD_INLINE void ECadExtKiCadHandler::CreateEcadNets(Ptr<ILayoutView> layout)
{
    auto & eMgr = EDataMgr::Instance();
    for (const auto & [kNetId, kNet] : m_kicad->nets) {
        auto net = eMgr.CreateNet(layout, kNet.name);
        if (nullptr == net) {
            //todo, error
            continue;
        }
        m_lut.net.emplace(kNetId, net);
    }
}

ECAD_INLINE void ECadExtKiCadHandler::CreateLayoutBoundary(Ptr<ILayoutView> layout)
{
    auto & eMgr = EDataMgr::Instance();
    EPolygonWithHolesData pwh;
    auto shape = eMgr.CreateShapePolygonWithHoles(pwh);
    layout->SetBoundary(std::move(shape));
}

} // namespace ecad::ext::kicad