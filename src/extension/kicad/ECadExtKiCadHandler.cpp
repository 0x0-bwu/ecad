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
    auto & db = *m_kicadDatabase;
    Rule defaultRule{0.25, 0.25};
    for (const auto & node : tree.branches) {
        if ("layers" == node.value) {
            for (const auto & lyr : node.branches) {
                auto name = lyr.branches[0].value;
                auto type = lyr.branches[1].value;
                if ("signal" == type || "power" == type) {
                    m_layer2IndexMap.emplace(name, std::stoi(lyr.value));
                    m_index2LayerMap.emplace(std::stoi(lyr.value), name);
                }
            }
        }
        else if ("net" == node.value and node.branches.size() > 2) {
            Int64 netId = 0;
            auto netName = node.branches.at(1).value;
            GetValue(node.branches.front().value, netId);
            m_net2IndexMap.emplace(netName, netId);
            m_index2NetMap.emplace(netId, netName);

            if (netName.back() == '+' || netName.back() == '-') {
                auto name = netName.substr(0, netName.size() - 1);
                auto iter = m_name2DiffPairNetMap.find(name);
                if (iter != m_name2DiffPairNetMap.cend())
                    iter->second.second = netId;
                else m_name2DiffPairNetMap.emplace(std::move(name), std::make_pair(netId, Int64(-1)));
            }
        }
        else if ("net_class" ==  node.value and node.branches.size()) {
            db.nets.resize(m_index2NetMap.size());
            Int64 netClassId = db.netClasses.size();
            auto netClassName = node.branches.front().value;
            EFloat clearance{0}, traceWidth{0}, viaDia{0}, viaDrill{0}, uViaDia{0}, uViaDril{0};

            for (const auto & netClassNode : node.branches) {
                const auto & s = netClassNode.branches.front().value;
                if (netClassNode.value == "clearance")
                    GetValue(s, clearance);
                else if (netClassNode.value == "trace_width")
                    GetValue(s, traceWidth);
                else if (netClassNode.value == "via_dia")
                    GetValue(s, viaDia);
                else if (netClassNode.value == "via_drill")
                    GetValue(s, viaDrill);
                else if (netClassNode.value == "uvia_dia")
                    GetValue(s, uViaDia);
                else if (netClassNode.value == "uvia_drill")
                    GetValue(s, uViaDril);
                else if (netClassNode.value == "add_net") {
                    auto netName = netClassNode.branches.front().value;
                    auto netId = -1;
                    auto name = netName.substr(0, netName.size() - 1);
                    auto diffPair = std::make_pair<Int64, Int64>(-1, -1);
                    auto netIter = m_net2IndexMap.find(netName);
                    if (netIter != m_net2IndexMap.cend()) {
                        netId = netIter->second;
                        GENERIC_ASSERT(netId < db.nets.size());
                    }
                    else {
                        ECAD_ERROR("undefined net name \"%1\" added in the netclass.", netName);
                    }
                    auto dpIter = m_name2DiffPairNetMap.find(name);
                    if (dpIter != m_name2DiffPairNetMap.cend())
                        diffPair = dpIter->second;
                    db.nets[netId] = Net(netId, std::move(netName), netClassId, std::move(diffPair));
                }
            }
            db.netClasses.emplace_back(netClassId, netClassName, clearance, traceWidth, viaDia, viaDrill, uViaDia, uViaDril);
        }
        else if ("module" == node.value and node.branches.size()) {
            std::string compName = node.branches.front().value;

            Instance instance;
            instance.id = static_cast<Int64>(db.instances.size());
            auto compIter = m_comp2IndexMap.find(compName);
            auto compId = compIter == m_comp2IndexMap.cend() ? -1 : compIter->second;
            Component component;
            component.id = compId;
            component.name = compName;
            bool isBottomComp{false};
            Int64 noNameId{0};

            for (const auto & moduleNode : node.branches) {
                
                if ("locked" == moduleNode.value)
                    instance.locked = true;
                
                if ("layer" == moduleNode.value and moduleNode.branches.size()) {
                    instance.layerId = GetLayerId(moduleNode.branches.front().value);
                    if (0 != instance.layerId)
                        isBottomComp = true;
                }

                if ("at" == moduleNode.value and moduleNode.branches.size()) {
                    GetValue(moduleNode.branches.front().value, instance.x, instance.y);
                    if (3 == moduleNode.branches.size())
                        GetValue(moduleNode.branches.at(2).value, instance.angle);
                }
                if ("fp_text" == moduleNode.value and moduleNode.branches.size()) {
                    //todo.
                }
                //create comp if need
                if (compIter == m_comp2IndexMap.cend()) {

                    if ("fp_line" == moduleNode.value and moduleNode.branches.size() > 3) {
                        Line line;
                        if (0 == instance.angle or 180 == instance.angle) {
                            GetValue(moduleNode.branches.at(0).branches.front().value, line.start[0], line.start[1]);
                            GetValue(moduleNode.branches.at(1).branches.front().value, line.end[0], line.end[1]);
                        }
                        else {
                            GetValue(moduleNode.branches.at(0).branches.front().value, line.start[1], line.start[0]);
                            GetValue(moduleNode.branches.at(1).branches.front().value, line.end[1], line.end[0]);
                        }
                        GetValue(moduleNode.branches.at(2).branches.front().value, line.width);
                        component.lines.emplace_back(std::move(line));
                    }
                    else if ("fp_poly" == moduleNode.value and moduleNode.branches.size() > 2) {
                        Poly poly;
                        for (const auto & coordNode : moduleNode.branches.at(0).branches) {
                            EFloat x{0}, y{0};
                            GetValue(coordNode.branches.front().value, x, y);
                            poly.shape.emplace_back(x, y);
                        }
                        GetValue(moduleNode.branches.at(2).branches.front().value, poly.width);
                        for (const auto & polyNode : moduleNode.branches) {

                            if ("layer" == polyNode.value) {
                                auto layer = polyNode.branches.front().value;
                                if (ECAD_KICAD_PCB_LAYER_FRONT_CRTYD_STR.data() == layer)
                                    poly.layer = ECAD_KICAD_PCB_LAYER_FRONT_CRTYD_ID;
                                else if (ECAD_KICAD_PCB_LAYER_BOTTOM_CRTYD_STR.data() == layer)
                                    poly.layer = ECAD_KICAD_PCB_LAYER_BOTTOM_CRTYD_ID;
                            }
                        }
                        component.polys.emplace_back(std::move(poly));
                    }
                    else if ("fp_circle" == moduleNode.value and moduleNode.branches.size() > 3) {
                        Circle circle;
                        GetValue(moduleNode.branches.at(0).branches.front().value, circle.center[0], circle.center[1]);
                        GetValue(moduleNode.branches.at(1).branches.front().value, circle.end[0], circle.end[1]);
                        GetValue(moduleNode.branches.at(3).branches.front().value, circle.width);
                        component.circles.emplace_back(std::move(circle));
                    }
                    else if ("fp_arc" == moduleNode.value and moduleNode.branches.size() > 4) {
                        Arc arc;
                        GetValue(moduleNode.branches.at(0).branches.front().value, arc.start[0], arc.start[1]);
                        GetValue(moduleNode.branches.at(1).branches.front().value, arc.end[0], arc.end[1]);
                        GetValue(moduleNode.branches.at(2).branches.front().value, arc.angle);
                        GetValue(moduleNode.branches.at(4).branches.front().value, arc.width);
                        component.arcs.emplace_back(std::move(arc));
                    }
                    else if ("pad" == moduleNode.value) {
                        Padstack padstack;
                        padstack.id = component.pads.size();
                        padstack.rule = defaultRule;
                        padstack.name = moduleNode.branches.at(0).value;
                        if ("\"\"" == padstack.name) {
                            padstack.name = "Unnammed" + std::to_string(noNameId);
                            noNameId++;
                        }

                        padstack.SetType(moduleNode.branches.at(1).value);
                        padstack.SetShape(moduleNode.branches.at(2).value);
                        GetValue(moduleNode.branches.at(3).branches.front().value, padstack.pos[0], padstack.pos[1]);
                        if (isBottomComp) padstack.pos[1] *= -1;

                        GetValue(moduleNode.branches.at(4).branches.front().value, padstack.size[0], padstack.size[1]);

                        if (3 == moduleNode.branches.at(3).branches.size())
                            GetValue(moduleNode.branches.at(3).branches.at(2).value, padstack.angle);
                        
                        padstack.angle -= instance.angle;

                        for (const auto & layerNode : moduleNode.branches.at(5).branches) {
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
                            GetValue(moduleNode.branches.at(6).branches.front().value, padstack.roundRectRatio);
                            padstack.shapeCoords = utils::RoundRect2ShapeCoords(padstack.size, padstack.roundRectRatio);
                        }

                        padstack.shapePolygon = utils::Shape2Coords(padstack.size, {0, 0}, 
                                                                    padstack.shape, instance.angle, 
                                                                    padstack.angle, padstack.roundRectRatio, 32);
                        
                        padstack.rule.clearance = 0;
                        component.pad2IndexMap.emplace(padstack.name, padstack.id); 
                        component.pads.emplace_back(std::move(padstack));
                    }
                }
            }

            if (compIter == m_comp2IndexMap.cend()) {
                m_comp2IndexMap.emplace(compName, compId);
                db.components.emplace_back(std::move(component));
            }

            noNameId = 0;
            for (const auto & padNode : node.branches) {
                if ("pad" == padNode.value) {
                    auto pinName = padNode.branches.front().value;
                    if ("\"\"" == pinName) {
                        pinName = "Unnamed" + std::to_string(noNameId);
                        noNameId++;
                    }
                    Int64 netId{0};
                    std::string netName{};
                    bool connected{false};
                    for (const auto & netNode : padNode.branches) {
                        if ("net" == netNode.value) {
                            connected = true;
                            netName = netNode.branches.at(1).value;
                            GetValue(netNode.branches.front().value, netId);
                            break;
                        }
                    }

                    if (not db.isComponentId(compId)) {
                        //todo err msg
                        continue;
                    }

                    auto & comp = db.components.at(compId);
                    auto padstackId = comp.GetPadstackId(pinName);
                    if (-1 == padstackId) {
                        // todo err msg
                        continue;
                    }

                    Pin pin(padstackId, compId, instance.id);

                    //todo. pin layers
                    if (connected) {
                        instance.pinNetMap.emplace(pinName, netId);
                        if (auto * net = FindNetByName(netName); net)
                            net->pins.emplace_back(std::move(pin));
                    }
                    else {
                        instance.pinNetMap.emplace(pinName, -1);
                        db.unconnectedPins.emplace_back(std::move(pin));
                    }
                }
            }
            instance.compId = compId;
            m_inst2IndexMap.emplace(instance.name, instance.id);
            db.instances.emplace_back(std::move(instance));
        }
        else if ("gr_line" == node.value) {
            Line line;
            std::string layer;
            for (const auto & bondNode : node.branches) {
                if ("start" == bondNode.value) {
                    GetValue(bondNode.branches.front().value, line.start[0], line.start[1]);
                }
                else if ("end" == bondNode.value) {
                    GetValue(bondNode.branches.front().value, line.end[0], line.end[1]);
                }
                else if ("angle" == bondNode.value) {
                    GetValue(bondNode.branches.front().value, line.angle);
                }
                else if ("layer" == bondNode.value) {
                    layer = bondNode.branches.front().value;
                    if (ECAD_KICAD_PCB_LAYER_EDGE_CUT_STR.data() != layer)
                        line.layer = -1;
                    else 
                        line.layer = ECAD_KICAD_PCB_LAYER_EDGE_CUT_ID;
                }
                else if ("width" == bondNode.value) {
                    GetValue(bondNode.branches.front().value, line.width);
                }
            }
            if (ECAD_KICAD_PCB_LAYER_EDGE_CUT_ID == line.layer)
                db.boundaryLines.emplace_back(std::move(line));
        }
        else if ("segment" == node.value) {
            Segment segment;
            GetValue(node.branches.at(0).branches.front().value, segment.start[0], segment.start[1]);
            GetValue(node.branches.at(1).branches.front().value, segment.end[0], segment.end[1]);
            GetValue(node.branches.at(2).branches.front().value, segment.width);
            segment.layer = node.branches.at(3).branches.front().value;
            GetValue(node.branches.at(4).branches.front().value, segment.netId);
            if (segment.netId >= 0 and static_cast<size_t>(segment.netId) < db.nets.size()) {
                segment.id = db.nets.at(segment.netId).segments.size();
                db.nets[segment.netId].segments.emplace_back(std::move(segment));
            }
            else {
                //todo err msg
            }
        }
        else if ("via" == node.value) {
            Via via;
            via.type = ViaType::THROUGH;
            for (const auto & viaNode : node.branches) {
                if ("at" == viaNode.value)
                    GetValue(viaNode.branches.front().value, via.pos[0], via.pos[1]);
                else if ("size" == viaNode.value)
                    GetValue(viaNode.branches.front().value, via.size);
                else if ("net" == viaNode.value)
                    GetValue(viaNode.branches.front().value, via.netId);
                else if ("layers" == viaNode.value) {
                    GetValue(viaNode.branches.at(0).value, via.startLayer);
                    GetValue(viaNode.branches.at(1).value, via.endLayer);
                }
                else if ("micro" == viaNode.value)
                    via.type = ViaType::MICRO;
                else if ("bline" == viaNode.value)
                    via.type = ViaType::BLIND_BURIED;
            }
            if (via.netId >= 0 and static_cast<size_t>(via.netId) < db.nets.size()) {
                via.id = db.nets.at(via.netId).vias.size();
                db.nets[via.netId].vias.emplace_back(std::move(via));
            }
            else {
                //todo, err msgs
            }
        }
        else if ("zone" == node.value) {
            //todo
        }
    }

    m_database= eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    return m_database;
}

ECAD_INLINE Int64 ECadExtKiCadHandler::GetNetId(const std::string & name) const
{
    auto iter = m_net2IndexMap.find(name);
    return iter == m_net2IndexMap.cend() ? -1 : iter->second;
}

ECAD_INLINE Int64 ECadExtKiCadHandler::GetLayerId(const std::string & name) const
{
    auto iter = m_layer2IndexMap.find(name);
    return iter == m_layer2IndexMap.cend() ? -1 : iter->second;    
}

ECAD_INLINE Net * ECadExtKiCadHandler::FindNetByName(const std::string & name) const
{
    auto id = GetNetId(name);
    if (id == -1) return nullptr;
    return &m_kicadDatabase->nets.at(id);
}

} // namespace ecad::ext::kicad