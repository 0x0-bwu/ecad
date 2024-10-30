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
    void ExtractNode(const Tree & node);
    void ExtractLayer(const Tree & node);
    void ExtractNet(const Tree & node);
    void ExtractNetClass(const Tree & node);
    void ExtractModule(const Tree & node);
    void ExtractGrLine(const Tree & node);
    void ExtractSegment(const Tree & node);
    void ExtractVia(const Tree & node);

    void ExtractCircle(const Tree & node);
    void ExtractArc(const Tree & node);
    void ExtractPoly(const Tree & node);
    void ExtractLine(const Tree & node);
    void ExtractPadstack(const Tree & node);
    void ExtractPad(const Tree & node);
    
    template <typename... Args>
    static void GetValue(const std::string & s, Args & ...args)
    {
        static std::stringstream ss;
        ss.str(s); ss.clear();
        (ss >> ... >> args);
    }

    template <typename... Args>
    static void GetValue(std::vector<Tree>::const_iterator iter, Args & ...args)
    {
        ([&]{
            GetValue(iter->value, args);
            std::advance(iter, 1);
        }(), ...);
    }

    template <typename... Args>
    static void TryGetValue(std::vector<Tree>::const_iterator iter, std::vector<Tree>::const_iterator end, Args & ...args)
    {
        ([&]{
            if (iter != end) {
                GetValue(iter->value, args);
                std::advance(iter, 1);
            }
        }(), ...);
    }

    EIndex GetNetId(const std::string & name) const;
    EIndex GetLayerId(const std::string & name) const;

    Net * FindNetByName(const std::string & name) const;

private:
    std::string m_filename;
    UPtr<Database> m_kicad{nullptr};
    Ptr<IDatabase> m_database{nullptr};

    ///temp lut
    std::unordered_map<std::string, Int64> m_net2IndexMap;
    std::unordered_map<Int64, std::string> m_index2NetMap;
    std::unordered_map<std::string, Int64> m_layer2IndexMap;
    std::unordered_map<Int64, std::string> m_index2LayerMap;
    std::unordered_map<std::string, Int64> m_comp2IndexMap;
    std::unordered_map<std::string, Int64> m_inst2IndexMap;
    std::unordered_map<std::string, EPair<Int64, Int64>> m_name2DiffPairNetMap;

    //func lut
    std::unordered_map<std::string, std::function<void(const Tree &)>> m_functions;

    //current state
    struct Current
    {
        Database * db = nullptr;
        Instance * inst = nullptr;
        Component * comp = nullptr;
        EIndex noNamePinId{0};
        EIndex noNamePadstackId{0};
    };
    Current m_current;
};

}//namespace kicad
}//namespace ext
}//namespace ecad