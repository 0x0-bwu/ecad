#pragma once
#include "ECadCommon.h"

namespace ecad {

class ICell;
class IDatabase;
class ILayoutView;

namespace ext {
namespace kicad {

class ECAD_API ECadExtKiCadHandler
{
public:
    explicit ECadExtKiCadHandler(const std::string & kicadFile);
    Ptr<IDatabase> CreateDatabase(const std::string & name, Ptr<std::string> err = nullptr);

private:
    std::string m_filename;
    Ptr<IDatabase> m_database{nullptr};

    ///temp data
    std::unordered_map<std::string, int64_t> m_net2IndexMap;
    std::unordered_map<int64_t, std::string> m_index2NetMap;
    std::unordered_map<std::string, int64_t> m_layer2IndexMap;
    std::unordered_map<int64_t, std::string> m_index2LayerMap;

};

}//namespace gds
}//namespace ext
}//namespace ecad