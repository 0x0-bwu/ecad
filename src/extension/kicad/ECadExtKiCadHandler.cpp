#include "ECadExtKiCadHandler.h"
#include "EKiCadObjects.h"
#include "EDataMgr.h"

namespace ecad::ext::kicad {

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
    
    EKiCadDB db;
    std::stringstream ss;
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
            break;
        }
        else if ("net" == node.value) {
            EInt64 netId = 0;
            auto netName = node.branches.at(1).value;
            GetValue(ss, node.branches.front().value, netId);
            m_net2IndexMap.emplace(netName, netId);
            m_index2NetMap.emplace(netId, netName);

            if (netName.back() == '+' || netName.back() == '-') {
                auto name = netName.substr(0, netName.size() - 1);
                auto iter = m_name2DiffPairNetMap.find(name);
                if (iter != m_name2DiffPairNetMap.cend())
                    iter->second.second = netId;
                else m_name2DiffPairNetMap.emplace(std::move(name), std::make_pair(netId, EInt64(-1)));
            }
        }
        else if ("net_class" ==  node.value) {
            db.nets.resize(m_index2NetMap.size());
            EInt64 netClassId = db.netClasses.size();
            auto netClassName = node.branches.front().value;
            EFloat clearance{0}, traceWidth{0}, viaDia{0}, viaDrill{0}, uViaDia{0}, uViaDril{0};

            for (const auto & netClassNode : node.branches) {
                const auto & s = netClassNode.branches.front().value;
                if (netClassNode.value == "clearance")
                    GetValue(ss, s, clearance);
                else if (netClassNode.value == "trace_width")
                    GetValue(ss, s, traceWidth);
                else if (netClassNode.value == "via_dia")
                    GetValue(ss, s, viaDia);
                else if (netClassNode.value == "via_drill")
                    GetValue(ss, s, viaDrill);
                else if (netClassNode.value == "uvia_dia")
                    GetValue(ss, s, uViaDia);
                else if (netClassNode.value == "uvia_drill")
                    GetValue(ss, s, uViaDril);
                else if (netClassNode.value == "add_net") {
                    auto netName = netClassNode.branches.front().value;
                    auto netId = -1;
                    auto name = netName.substr(0, netName.size() - 1);
                    auto diffPair = std::make_pair<EInt64, EInt64>(-1, -1);
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
    }

    m_database= eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    return m_database;
}

} // namespace ecad::ext::kicad