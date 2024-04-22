#include "ECadExtKiCadHandler.h"
#include "EKiCadParser.h"
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
    
    for (const auto & sub : tree.branches) {
        if ("layers" == sub.value) {
            for (const auto & lyr : sub.branches) {
                auto name = lyr.branches[0].value;
                auto type = lyr.branches[1].value;
                if ("signal" == type || "power" == type) {
                    m_layer2IndexMap.emplace(name, std::stoi(lyr.value));
                    m_index2LayerMap.emplace(std::stoi(lyr.value), name);
                }
            }
            break;
        }
        else if ("net" == sub.value) {
            auto netId = 0;
        }
    }

    m_database= eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    return m_database;
}

} // namespace ecad::ext::kicad