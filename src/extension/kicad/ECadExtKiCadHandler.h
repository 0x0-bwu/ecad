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
    void ExtractSetup(const Tree & node);
    void ExtractStackup(const Tree & node);
    void ExtractNet(const Tree & node);
    void ExtractFootprint(const Tree & node);
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

private:
    std::string m_filename;
    UPtr<Database> m_kicad{nullptr};
    Ptr<IDatabase> m_database{nullptr};

    //func lut
    std::unordered_map<std::string, std::function<void(const Tree &)>> m_functions;

    //current state
    struct State
    {
        EIndex noNamePinId{0};
        EIndex noNamePadstackId{0};
        Component * comp = nullptr;
        void Reset() { *this = State{}; }
    };
    State m_current;
};

}//namespace kicad
}//namespace ext
}//namespace ecad