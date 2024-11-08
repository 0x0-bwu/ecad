#pragma once
#include "basic/ECadCommon.h"
#include "EKiCadObjects.h"
#include "EKiCadParser.h"
namespace ecad {

class INet;
class ICell;
class ILayer;
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
    bool ExtractKiCadObjects(Ptr<std::string> err = nullptr);

    void ExtractNode(const Tree & node);
    void ExtractLayer(const Tree & node);
    void ExtractSetup(const Tree & node);
    void ExtractStackup(const Tree & node);
    void ExtractNet(const Tree & node);
    void ExtractFootprint(const Tree & node);
    void ExtractSegment(const Tree & node);
    void ExtractZone(const Tree & node);
    void ExtractVia(const Tree & node);

    void ExtractCircle(const Tree & node);
    void ExtractArc(const Tree & node);
    void ExtractPoly(const Tree & node);
    void ExtractLine(const Tree & node);
    void ExtractPad(const Tree & node);

    void ExtractPoints(const Tree & node, Points & points);
    void ExtractStroke(const Tree & node, Stroke & stroke);

    bool CreateEcadObjects(Ptr<std::string> err = nullptr);
    void CreateEcadLayers(Ptr<ILayoutView> layout);
    void CreateEcadNets(Ptr<ILayoutView> layout);
    void CreateLayoutBoundary(Ptr<ILayoutView> layout);
    
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

    template <typename... Args>
    static void GetValue(const std::vector<Tree> & branches, Args & ...args)
    {
        GetValue(branches.begin(), args...);
    }

    template <typename... Args>
    static void TryGetValue(const std::vector<Tree> & branches, Args & ...args)
    {
        TryGetValue(branches.begin(), branches.end(), args...);
    }

private:
    std::string m_filename;
    UPtr<Database> m_kicad{nullptr};
    Ptr<IDatabase> m_database{nullptr};

    //func lut
    std::unordered_map<std::string, std::function<void(const Tree &)>> m_functions;
    
    // kicad-ecad lut
    struct Lut
    {
        std::unordered_map<EIndex, CPtr<ILayer>> layer;
        std::unordered_map<EIndex, CPtr<INet>> net;
    };
    Lut m_lut;
    
    //current state
    struct State
    {
        EIndex noNamePadId{0};
        Component * comp = nullptr;
        void Reset() { *this = State{}; }
    };
    State m_current;
};

}//namespace kicad
}//namespace ext
}//namespace ecad