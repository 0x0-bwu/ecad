#pragma once
#include "basic/ECadCommon.h"
#include "EKiCadObjects.h"
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
    void GetValue(const std::string & s, Args & ...args)
    {
        static std::stringstream ss;
        ss.str(s); ss.clear();
        (ss >> ... >> args);
    }

    Int64 GetNetId(const std::string & name) const;
    Int64 GetLayerId(const std::string & name) const;

    Net * FindNetByName(const std::string & name) const;

private:
    std::string m_filename;
    Ptr<IDatabase> m_database{nullptr};
    UPtr<Database> m_kicadDatabase{nullptr};

    ///temp data
    std::unordered_map<std::string, Int64> m_net2IndexMap;
    std::unordered_map<Int64, std::string> m_index2NetMap;
    std::unordered_map<std::string, Int64> m_layer2IndexMap;
    std::unordered_map<Int64, std::string> m_index2LayerMap;
    std::unordered_map<std::string, Int64> m_comp2IndexMap;
    std::unordered_map<std::string, Int64> m_inst2IndexMap;
    std::unordered_map<std::string, EPair<Int64, Int64>> m_name2DiffPairNetMap;
};

}//namespace kicad
}//namespace ext
}//namespace ecad