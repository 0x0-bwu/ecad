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
    
    m_kicadDatabase.reset(new Database);
    for (const auto & node : tree.branches) {
        if ("layers" == node.value)
            ExtractLayer(node);
        else if ("net" == node.value)
            ExtractNet(node);
        else if ("net_class" ==  node.value)
            ExtractNetClass(node);
        else if ("module" == node.value)
            ExtractModule(node);
        else if ("gr_line" == node.value)
            ExtractGrLine(node);
        else if ("segment" == node.value)
            ExtractSegment(node);
        else if ("via" == node.value)
            ExtractVia(node);
        else if ("zone" == node.value) {
            //todo
        }
    }

    m_database= eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    return m_database;
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
    auto & db = *m_kicadDatabase;
    db.nets.resize(m_index2NetMap.size());
    auto netClassId = db.netClasses.size();
    auto netClassName = node.branches.front().value;
    EFloat clearance{0}, traceWidth{0}, viaDia{0}, viaDrill{0}, uViaDia{0}, uViaDril{0};

    for (const auto & netClass : node.branches) {
        auto iter = netClass.branches.begin();
        if (node.value == "clearance")
            GetValue(iter, clearance);
        else if (node.value == "trace_width")
            GetValue(iter, traceWidth);
        else if (node.value == "via_dia")
            GetValue(iter, viaDia);
        else if (node.value == "via_drill")
            GetValue(iter, viaDrill);
        else if (node.value == "uvia_dia")
            GetValue(iter, uViaDia);
        else if (node.value == "uvia_drill")
            GetValue(iter, uViaDril);
        else if (node.value == "add_net") {
            auto netName = node.branches.front().value;
            auto netId = invalidIndex;
            auto name = netName.substr(0, netName.size() - 1);
            auto diffPair = std::pair<EIndex, EIndex>(invalidIndex, invalidIndex);
            auto netIter = m_net2IndexMap.find(netName);
            if (netIter != m_net2IndexMap.cend()) {
                netId = netIter->second;
                GENERIC_ASSERT(netId < db.nets.size());
            }
            else {
                ECAD_ERROR("undefined net name \"%1%\" added in the netclass.", netName);
                continue;
            }
            auto dpIter = m_name2DiffPairNetMap.find(name);
            if (dpIter != m_name2DiffPairNetMap.cend())
                diffPair = dpIter->second;
            db.nets[netId] = Net(netId, std::move(netName), netClassId, std::move(diffPair));
        }
    }
    db.netClasses.emplace_back(netClassId, netClassName, clearance, traceWidth, viaDia, viaDrill, uViaDia, uViaDril);
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractModule(const Tree & node)
{
    auto & db = *m_kicadDatabase;
    auto & compName = node.branches.front().value;

    Instance instance;
    instance.id = db.instances.size();
    auto compIter = m_comp2IndexMap.find(compName);
    auto compId = compIter == m_comp2IndexMap.cend() ? invalidIndex : compIter->second;
    Component component;
    component.id = compId;
    component.name = compName;

    EIndex noNameId{0};
    bool isBottomComp{false};
    for (const auto & sub : node.branches) {
        const auto & branchs = sub.branches;
        
        if ("locked" == sub.value)
            instance.locked = true;
        
        else if ("layer" == sub.value) {
            instance.layerId = GetLayerId(node.branches.front().value);
            if (0 != instance.layerId)
                isBottomComp = true;
        }

        else if ("at" == sub.value) {
            TryGetValue(branchs.begin(), branchs.end(), 
                        instance.x, instance.y, instance.angle);
        }
        else if ("fp_text" == sub.value) {
            //todo.
        }
        //create comp if need
        else if (compIter == m_comp2IndexMap.cend()) {

            if ("fp_line" == sub.value)
                ExtractLine(sub, component, instance);
            else if ("fp_poly" == sub.value)
                ExtractPoly(sub, component);
            else if ("fp_circle" == sub.value)
                ExtractCircle(sub, component);
            else if ("fp_arc" == sub.value)
                ExtractArc(sub, component);
            else if ("pad" == sub.value)
                ExtractPadstack(sub, component, instance, noNameId, isBottomComp);
        }
    }

    if (compIter == m_comp2IndexMap.cend()) {
        m_comp2IndexMap.emplace(compName, compId);
        db.components.emplace_back(std::move(component));
    }

    noNameId = 0;
    for (const auto & sub : node.branches)
        if ("pad" == sub.value)
            ExtractPad(sub, component, instance, noNameId);

    instance.compId = compId;
    m_inst2IndexMap.emplace(instance.name, instance.id);
    db.instances.emplace_back(std::move(instance));
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
        m_kicadDatabase->boundaryLines.emplace_back(std::move(line));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractSegment(const Tree & node)
{
    Segment segment;
    auto & db = *m_kicadDatabase;
    GetValue(node.branches.at(0).branches.front().value, segment.start[0], segment.start[1]);
    GetValue(node.branches.at(1).branches.front().value, segment.end[0], segment.end[1]);
    GetValue(node.branches.at(2).branches.front().value, segment.width);
    segment.layer = node.branches.at(3).branches.front().value;
    GetValue(node.branches.at(4).branches.front().value, segment.netId);
    if (segment.netId < db.nets.size()) {
        segment.id = db.nets.at(segment.netId).segments.size();
        db.nets[segment.netId].segments.emplace_back(std::move(segment));
    }
    else {
        //todo err msg
    }
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractVia(const Tree & node)
{
    Via via;
    via.type = ViaType::THROUGH;
    auto & db = *m_kicadDatabase;
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
    if (via.netId < db.nets.size()) {
        via.id = db.nets.at(via.netId).vias.size();
        db.nets[via.netId].vias.emplace_back(std::move(via));
    }
    else {
        //todo, err msgs
    }
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractCircle(const Tree & node, Component & comp)
{
    Circle circle;
    GetValue(node.branches.at(0).branches.begin(), circle.center[0], circle.center[1]);
    GetValue(node.branches.at(1).branches.begin(), circle.end[0], circle.end[1]);
    GetValue(node.branches.at(3).branches.begin(), circle.width);
    comp.circles.emplace_back(std::move(circle));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractPoly(const Tree & node, Component & comp)
{
    Poly poly;
    for (const auto & coordNode : node.branches.at(0).branches) {
        EFloat x{0}, y{0};
        GetValue(coordNode.branches.begin(), x, y);
        poly.shape.emplace_back(x, y);
    }
    GetValue(node.branches.at(2).branches.begin(), poly.width);
    for (const auto & polyNode : node.branches) {

        if ("layer" == polyNode.value) {
            auto layer = polyNode.branches.front().value;
            if (ECAD_KICAD_PCB_LAYER_FRONT_CRTYD_STR.data() == layer)
                poly.layer = ECAD_KICAD_PCB_LAYER_FRONT_CRTYD_ID;
            else if (ECAD_KICAD_PCB_LAYER_BOTTOM_CRTYD_STR.data() == layer)
                poly.layer = ECAD_KICAD_PCB_LAYER_BOTTOM_CRTYD_ID;
        }
    }
    comp.polys.emplace_back(std::move(poly));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractArc(const Tree & node, Component & comp)
{
    Arc arc;
    GetValue(node.branches.at(0).branches.begin(), arc.start[0], arc.start[1]);
    GetValue(node.branches.at(1).branches.begin(), arc.end[0], arc.end[1]);
    GetValue(node.branches.at(2).branches.begin(), arc.angle);
    GetValue(node.branches.at(4).branches.begin(), arc.width);
    comp.arcs.emplace_back(std::move(arc));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractLine(const Tree & node, Component & comp, Instance & inst)
{
    Line line;
    if (0 == inst.angle or 180 == inst.angle) {
        GetValue(node.branches.at(0).branches.begin(), line.start[0], line.start[1]);
        GetValue(node.branches.at(1).branches.begin(), line.end[0], line.end[1]);
    }
    else {
        GetValue(node.branches.at(0).branches.begin(), line.start[1], line.start[0]);
        GetValue(node.branches.at(1).branches.begin(), line.end[1], line.end[0]);
    }
    GetValue(node.branches.at(2).branches.begin(), line.width);
    comp.lines.emplace_back(std::move(line));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractPadstack(const Tree & node, Component & comp, Instance & inst, EIndex & noNameId, bool isBottomComp)
{
    Padstack padstack;
    padstack.id = comp.pads.size();
    padstack.rule = Rule{0.25, 0.25};
    padstack.name = node.branches.at(0).value;
    if ("\"\"" == padstack.name) {
        padstack.name = "Unnammed" + std::to_string(noNameId);
        noNameId++;
    }

    padstack.SetType(node.branches.at(1).value);
    padstack.SetShape(node.branches.at(2).value);
    TryGetValue(node.branches.at(3).branches.begin(), node.branches.at(3).branches.end(),
                padstack.pos[0], padstack.pos[1], padstack.angle);
    if (isBottomComp) padstack.pos[1] *= -1;
    padstack.angle -= inst.angle;

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
                                                padstack.shape, inst.angle, 
                                                padstack.angle, padstack.roundRectRatio, 32);
    
    padstack.rule.clearance = 0;
    comp.pad2IndexMap.emplace(padstack.name, padstack.id); 
    comp.pads.emplace_back(std::move(padstack));
}

ECAD_INLINE void ECadExtKiCadHandler::ExtractPad(const Tree & node, Component & comp, Instance & inst, EIndex & noNameId)
{
    auto & db = *m_kicadDatabase;
    auto pinName = node.branches.front().value;
    if ("\"\"" == pinName) {
        pinName = "Unnamed" + std::to_string(noNameId);
        noNameId++;
    }
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

    if (not db.isComponentId(comp.id)) {
        //todo err msg
        return;
    }

    auto padstackId = comp.GetPadstackId(pinName);
    if (invalidIndex == padstackId) {
        // todo err msg
        return;
    }

    Pin pin(padstackId, comp.id, inst.id);

    //todo. pin layers
    if (connected) {
        inst.pinNetMap.emplace(pinName, netId);
        if (auto * net = FindNetByName(netName); net)
            net->pins.emplace_back(std::move(pin));
    }
    else {
        inst.pinNetMap.emplace(pinName, invalidIndex);
        db.unconnectedPins.emplace_back(std::move(pin));
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
    return &m_kicadDatabase->nets.at(id);
}

} // namespace ecad::ext::kicad