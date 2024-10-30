#include "ECadExtKiCadHandler.h"
#include "EDataMgr.h"

namespace ecad::ext::kicad {

using namespace generic;


namespace utils {

std::vector<FPoint2D> RoundRect2ShapeCoords(const FPoint2D & size, EFloat ratio)
{
    //todo
    return std::vector<FPoint2D>{};
}

std::vector<FPoint2D> Shape2Coords(const FPoint2D & size, const FPoint2D & pos, PadShape shape, EFloat a1, EFloat a2, EFloat ratio, size_t circleDiv)
{
    //todo
    return std::vector<FPoint2D>{};
}

}

ECAD_INLINE ECadExtKiCadHandler::ECadExtKiCadHandler(const std::string & kicadFile)
 : m_filename(kicadFile)
{
    m_functions.emplace("layers"   , std::bind(&ECadExtKiCadHandler::ExtractLayer   , this, std::placeholders::_1));
    m_functions.emplace("net"      , std::bind(&ECadExtKiCadHandler::ExtractNet     , this, std::placeholders::_1));
    m_functions.emplace("net_class", std::bind(&ECadExtKiCadHandler::ExtractNetClass, this, std::placeholders::_1));
    m_functions.emplace("module"   , std::bind(&ECadExtKiCadHandler::ExtractModule  , this, std::placeholders::_1));
    m_functions.emplace("gr_line"  , std::bind(&ECadExtKiCadHandler::ExtractGrLine  , this, std::placeholders::_1));
    m_functions.emplace("segment"  , std::bind(&ECadExtKiCadHandler::ExtractSegment , this, std::placeholders::_1));
    m_functions.emplace("via"      , std::bind(&ECadExtKiCadHandler::ExtractVia     , this, std::placeholders::_1));
}

ECAD_INLINE Ptr<IDatabase> ECadExtKiCadHandler::CreateDatabase(const std::string & name, Ptr<std::string> err)
{
    auto & eMgr = EDataMgr::Instance();
    if (eMgr.OpenDatabase(name)){
        if(err) *err = fmt::Fmt2Str("Error: database %1% is already exist.", name);
        return nullptr;
    }

    Tree tree;
    EKiCadParser parser;
    if (not parser(m_filename.data(), tree)) {
        if(err) *err = fmt::Fmt2Str("Error: failed to parse %1%.", m_filename);
        return nullptr; 
    }
    
    m_kicad.reset(new Database);
    for (auto & node : tree.branches)
        ExtractNode(node);

    m_database = eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    return m_database;
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractNode(const Tree & node)
{
    auto iter = m_functions.find(node.value);
    if (iter != m_functions.cend())
        iter->second(node);
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractLayer(const Tree & node)
{
    for (const auto & lyr : node.branches) {                
        auto name = lyr.branches[0].value;
        auto type = lyr.branches[1].value;
        if ("signal" == type || "power" == type) {
            m_layer2IndexMap.emplace(name, std::stoi(lyr.value));
            m_index2LayerMap.emplace(std::stoi(lyr.value), name);
        }
    }
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractNet(const Tree & node)
{
    EIndex netId{invalidIndex};
    auto & netName = node.branches.at(1).value;
    GetValue(node.branches.begin(), netId);
    m_net2IndexMap.emplace(netName, netId);
    m_index2NetMap.emplace(netId, netName);

    if (netName.back() == '+' || netName.back() == '-') {
        auto name = netName.substr(0, netName.size() - 1);
        auto iter = m_name2DiffPairNetMap.find(name);
        if (iter != m_name2DiffPairNetMap.cend())
            iter->second.second = netId;
        else m_name2DiffPairNetMap.emplace(std::move(name), std::make_pair(netId, invalidIndex));
    }
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractNetClass(const Tree & node)
{
    m_kicad->nets.resize(m_index2NetMap.size());
    auto netClassId = m_kicad->netClasses.size();
    auto netClassName = node.branches.front().value;
    EFloat clearance{0}, traceWidth{0}, viaDia{0}, viaDrill{0}, uViaDia{0}, uViaDrill{0};
    for (const auto & sub : node.branches) {
        auto iter = sub.branches.begin();
        if ("clearance" == sub.value)
            GetValue(iter, clearance);
        else if ("trace_width" == sub.value)
            GetValue(iter, traceWidth);
        else if ("via_dia" == sub.value)
            GetValue(iter, viaDia);
        else if ("via_drill" == sub.value)
            GetValue(iter, viaDrill);
        else if ("uvia_dia" == sub.value)
            GetValue(iter, uViaDia);
        else if ("uvia_drill" == sub.value)
            GetValue(iter, uViaDrill);
        else if ("add_net" == sub.value) {
            auto netId = invalidIndex;
            auto netName = sub.branches.front().value;
            auto diffPair = std::pair<EIndex, EIndex>(invalidIndex, invalidIndex);
            auto netIter = m_net2IndexMap.find(netName);
            if (netIter != m_net2IndexMap.cend()) {
                netId = netIter->second;
                GENERIC_ASSERT(netId < m_kicad->nets.size());
            }
            else {
                ECAD_ERROR("undefined net name \"%1%\" added in the netclass.", netName);
                continue;
            }
            auto dpIter = m_name2DiffPairNetMap.find(netName);
            if (dpIter != m_name2DiffPairNetMap.cend())
                diffPair = dpIter->second;
            m_kicad->nets[netId] = Net(netId, std::move(netName), netClassId, std::move(diffPair));
        }
    }
    m_kicad->netClasses.emplace_back(netClassId, netClassName, clearance, traceWidth, viaDia, viaDrill, uViaDia, uViaDrill);
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractModule(const Tree & node)
{
    auto & compName = node.branches.front().value;
    Instance instance;
    instance.id = m_kicad->instances.size();
    auto compIter = m_comp2IndexMap.find(compName);
    auto compId = compIter == m_comp2IndexMap.cend() ? 
                  m_kicad->components.size() : compIter->second;
    Component component(compName, compId);
    m_current.inst = &instance;
    m_current.comp = &component;

    for (const auto & sub : node.branches) {
        const auto & branches = sub.branches;
        
        if ("locked" == sub.value)
            instance.locked = true;
        else if ("layer" == sub.value) {
            instance.layerId = GetLayerId(sub.branches.front().value);
            if (0 != instance.layerId)
                component.flipped = true;
        }
        else if ("tedit" == sub.value) {
            //todo.
        }
        else if ("tstamp" == sub.value) {
            //todo.
        }
        else if ("at" == sub.value) {
            TryGetValue(branches.begin(), branches.end(), 
                        instance.x, instance.y, instance.angle);
        }
        else if ("fp_text" == sub.value) {
            //todo.
        }
        //create comp if need
        else if (compIter == m_comp2IndexMap.cend()) {

            if ("fp_line" == sub.value)
                ExtractLine(sub);
            else if ("fp_poly" == sub.value)
                ExtractPoly(sub);
            else if ("fp_circle" == sub.value)
                ExtractCircle(sub);
            else if ("fp_arc" == sub.value)
                ExtractArc(sub);
            else if ("pad" == sub.value)
                ExtractPadstack(sub);
        }
    }
    for (const auto & sub : node.branches)
        if ("pad" == sub.value)
            ExtractPad(sub);

    if (compIter == m_comp2IndexMap.cend()) {
        m_comp2IndexMap.emplace(compName, compId);
        m_kicad->components.emplace_back(std::move(component));
    }
    
    instance.compId = compId;
    m_inst2IndexMap.emplace(instance.name, instance.id);
    m_kicad->instances.emplace_back(std::move(instance));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractGrLine(const Tree & node)
{
    Line line;
    std::string layer;
    for (const auto & sub : node.branches) {
        if ("start" == sub.value)
            GetValue(sub.branches.begin(), line.start[0], line.start[1]);
        else if ("end" == sub.value)
            GetValue(sub.branches.begin(), line.end[0], line.end[1]);
        else if ("angle" == sub.value) 
            GetValue(sub.branches.begin(), line.angle);
        else if ("layer" == sub.value) {
            layer = sub.branches.front().value;
            if (ECAD_KICAD_PCB_LAYER_EDGE_CUT_STR.data() != layer)
                line.layer = invalidIndex;
            else 
                line.layer = ECAD_KICAD_PCB_LAYER_EDGE_CUT_ID;
        }
        else if ("width" == sub.value)
            GetValue(sub.branches.begin(), line.width);
    }
    if (ECAD_KICAD_PCB_LAYER_EDGE_CUT_ID == line.layer)
        m_kicad->boundaryLines.emplace_back(std::move(line));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractSegment(const Tree & node)
{
    Segment segment;
    GetValue(node.branches.at(0).branches.front().value, segment.start[0], segment.start[1]);
    GetValue(node.branches.at(1).branches.front().value, segment.end[0], segment.end[1]);
    GetValue(node.branches.at(2).branches.front().value, segment.width);
    segment.layer = node.branches.at(3).branches.front().value;
    GetValue(node.branches.at(4).branches.front().value, segment.netId);
    if (segment.netId < m_kicad->nets.size()) {
        segment.id = m_kicad->nets.at(segment.netId).segments.size();
        m_kicad->nets[segment.netId].segments.emplace_back(std::move(segment));
    }
    else {
        //todo err msg
    }
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractVia(const Tree & node)
{
    Via via;
    via.type = ViaType::THROUGH;
    for (const auto & sub : node.branches) {
        if ("at" == sub.value)
            GetValue(sub.branches.front().value, via.pos[0], via.pos[1]);
        else if ("size" == sub.value)
            GetValue(sub.branches.front().value, via.size);
        else if ("net" == sub.value)
            GetValue(sub.branches.front().value, via.netId);
        else if ("layers" == sub.value) {
            GetValue(sub.branches.at(0).value, via.startLayer);
            GetValue(sub.branches.at(1).value, via.endLayer);
        }
        else if ("micro" == sub.value)
            via.type = ViaType::MICRO;
        else if ("bline" == sub.value)
            via.type = ViaType::BLIND_BURIED;
    }
    if (via.netId < m_kicad->nets.size()) {
        via.id = m_kicad->nets.at(via.netId).vias.size();
        m_kicad->nets[via.netId].vias.emplace_back(std::move(via));
    }
    else {
        //todo, err msgs
    }
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractCircle(const Tree & node)
{
    Circle circle;
    GetValue(node.branches.at(0).branches.begin(), circle.center[0], circle.center[1]);
    GetValue(node.branches.at(1).branches.begin(), circle.end[0], circle.end[1]);
    GetValue(node.branches.at(3).branches.begin(), circle.width);
    m_current.comp->circles.emplace_back(std::move(circle));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractPoly(const Tree & node)
{
    Poly poly;
    for (const auto & sub : node.branches.at(0).branches) {
        EFloat x{0}, y{0};
        GetValue(sub.branches.begin(), x, y);
        poly.shape.emplace_back(x, y);
    }
    GetValue(node.branches.at(2).branches.begin(), poly.width);
    for (const auto & sub : node.branches) {

        if ("layer" == sub.value) {
            auto layer = sub.branches.front().value;
            if (ECAD_KICAD_PCB_LAYER_FRONT_CRTYD_STR.data() == layer)
                poly.layer = ECAD_KICAD_PCB_LAYER_FRONT_CRTYD_ID;
            else if (ECAD_KICAD_PCB_LAYER_BOTTOM_CRTYD_STR.data() == layer)
                poly.layer = ECAD_KICAD_PCB_LAYER_BOTTOM_CRTYD_ID;
        }
    }
    m_current.comp->polys.emplace_back(std::move(poly));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractArc(const Tree & node)
{
    Arc arc;
    GetValue(node.branches.at(0).branches.begin(), arc.start[0], arc.start[1]);
    GetValue(node.branches.at(1).branches.begin(), arc.end[0], arc.end[1]);
    GetValue(node.branches.at(2).branches.begin(), arc.angle);
    GetValue(node.branches.at(4).branches.begin(), arc.width);
    m_current.comp->arcs.emplace_back(std::move(arc));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractLine(const Tree & node)
{
    Line line;
    if (0 == m_current.inst->angle or 180 == m_current.inst->angle) {
        GetValue(node.branches.at(0).branches.begin(), line.start[0], line.start[1]);
        GetValue(node.branches.at(1).branches.begin(), line.end[0], line.end[1]);
    }
    else {
        GetValue(node.branches.at(0).branches.begin(), line.start[1], line.start[0]);
        GetValue(node.branches.at(1).branches.begin(), line.end[1], line.end[0]);
    }
    GetValue(node.branches.at(2).branches.begin(), line.width);
    m_current.comp->lines.emplace_back(std::move(line));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractPadstack(const Tree & node)
{
    Padstack padstack;
    padstack.id = m_current.comp->pads.size();
    padstack.rule = Rule{0.25, 0.25};
    padstack.name = node.branches.at(0).value;
    if ("\"\"" == padstack.name)
        padstack.name = "Unnammed" + std::to_string(m_current.noNamePadstackId++);

    padstack.SetType(node.branches.at(1).value);
    padstack.SetShape(node.branches.at(2).value);
    TryGetValue(node.branches.at(3).branches.begin(), node.branches.at(3).branches.end(),
                padstack.pos[0], padstack.pos[1], padstack.angle);
    if (m_current.comp->flipped) padstack.pos[1] *= -1;
    padstack.angle -= m_current.inst->angle;

    GetValue(node.branches.at(4).branches.begin(), padstack.size[0], padstack.size[1]);                        

    for (const auto & layerNode : node.branches.at(5).branches) {
        //todo. layers
    }

    if (PadShape::CIRCLE == padstack.shape)
        padstack.rule.radius = padstack.size[0] / 2;
    else if (PadShape::OVAL == padstack.shape) {
        padstack.rule.radius = padstack.size[0] / 4;
        padstack.shapeCoords.emplace_back(padstack.size[1] / -2, 0);
        padstack.shapeCoords.emplace_back(padstack.size[1] / 2, 0);
    }
    else if (PadShape::RECT == padstack.shape) {
        padstack.rule.radius = 0;
        auto x1 = padstack.size[0] / -2;
        auto y1 = padstack.size[1] / 2;
        auto x2 = padstack.size[0] / 2;
        auto y2 = padstack.size[1] / -2;
        padstack.shapeCoords.emplace_back(x1, y1);
        padstack.shapeCoords.emplace_back(x2, y1);
        padstack.shapeCoords.emplace_back(x2, y2);
        padstack.shapeCoords.emplace_back(x1, y2);
    }
    else if (PadShape::ROUNDRECT == padstack.shape) {
        padstack.rule.radius = 0;
        GetValue(node.branches.at(6).branches.begin(), padstack.roundRectRatio);
        padstack.shapeCoords = utils::RoundRect2ShapeCoords(padstack.size, padstack.roundRectRatio);
    }

    padstack.shapePolygon = utils::Shape2Coords(padstack.size, {0, 0}, 
                                                padstack.shape, m_current.inst->angle, 
                                                padstack.angle, padstack.roundRectRatio, 32);
    
    padstack.rule.clearance = 0;
    m_current.comp->pad2IndexMap.emplace(padstack.name, padstack.id); 
    m_current.comp->pads.emplace_back(std::move(padstack));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractPad(const Tree & node)
{
    auto pinName = node.branches.front().value;
    if ("\"\"" == pinName)
        pinName = "Unnamed" + std::to_string(m_current.noNamePinId++);

    bool connected{false};
    std::string netName{};
    EIndex netId{invalidIndex};
    for (const auto & sub : node.branches) {
        if ("net" == sub.value) {
            connected = true;
            netName = sub.branches.at(1).value;
            GetValue(sub.branches.begin(), netId);
            break;
        }
    }

    if (not m_kicad->isComponentId(m_current.comp->id)) {
        //todo err msg
        return;
    }

    auto padstackId = m_current.comp->GetPadstackId(pinName);
    if (invalidIndex == padstackId) {
        // todo err msg
        return;
    }

    Pin pin(padstackId, m_current.comp->id, m_current.inst->id);

    //todo. pin layers
    if (connected) {
        m_current.inst->pinNetMap.emplace(pinName, netId);
        if (auto * net = FindNetByName(netName); net)
            net->pins.emplace_back(std::move(pin));
    }
    else {
        m_current.inst->pinNetMap.emplace(pinName, invalidIndex);
        m_kicad->unconnectedPins.emplace_back(std::move(pin));
    }
}

ECAD_INLINE EIndex ECadExtKiCadHandler::GetNetId(const std::string & name) const
{
    auto iter = m_net2IndexMap.find(name);
    return iter == m_net2IndexMap.cend() ? invalidIndex : iter->second;
}

ECAD_INLINE EIndex ECadExtKiCadHandler::GetLayerId(const std::string & name) const
{
    auto iter = m_layer2IndexMap.find(name);
    return iter == m_layer2IndexMap.cend() ? invalidIndex : iter->second;    
}

ECAD_INLINE Net * ECadExtKiCadHandler::FindNetByName(const std::string & name) const
{
    auto id = GetNetId(name);
    if (id == invalidIndex) return nullptr;
    return &m_kicad->nets.at(id);
}

} // namespace ecad::ext::kicad