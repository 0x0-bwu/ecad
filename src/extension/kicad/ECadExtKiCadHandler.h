#pragma once
#include "basic/ECadCommon.h"
#include "EKiCadParser.h"
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
    template <typename... Args>
    void GetValue(std::stringstream & ss, const std::string & s, Args ...args)
    {
        ss.str(s); ss.clear();
        (ss >> ... >> args);
    }
private:
    std::string m_filename;
    Ptr<IDatabase> m_database{nullptr};

    ///temp data
    std::unordered_map<std::string, EInt64> m_net2IndexMap;
    std::unordered_map<EInt64, std::string> m_index2NetMap;
    std::unordered_map<std::string, EInt64> m_layer2IndexMap;
    std::unordered_map<EInt64, std::string> m_index2LayerMap;
    std::unordered_map<std::string, EPair<EInt64, EInt64>> m_name2DiffPairNetMap;
};

}//namespace kicad
}//namespace ext
}//namespace ecad