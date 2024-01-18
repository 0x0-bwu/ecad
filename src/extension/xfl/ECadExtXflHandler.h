#pragma once
#include "ECadCommon.h"
#include "EXflObjects.h"
namespace ecad {

class EDataMgr;
class IDatabase;
class ILayoutView;

namespace ext {
namespace xfl {

class ECAD_API ECadExtXflHandler
{
public:
    explicit ECadExtXflHandler(const std::string & xflFile, size_t circleDiv = 12);
    Ptr<IDatabase> CreateDatabase(const std::string & name, std::string * err = nullptr);

private:
    void ImportComponentDefs();
    void ImportMaterialDefs();
    void ImportPadstackDefs();
    void ImportLayers(Ptr<ILayoutView> layout);
    void ImportNets(Ptr<ILayoutView> layout);
    void ImportConnObjs(Ptr<ILayoutView> layout);
    void ImportBoardGeom(Ptr<ILayoutView> layout);

    void Reset();

private:
    std::string m_xflFile;
    size_t m_circleDiv = 12;

private:
    std::string GetNextPadstackInstName(const std::string & defName);
    std::pair<int, UPtr<EShape> > makeEShapeFromInstObject(EDataMgr * mgr, const EShapeGetter & eShapeGetter, const InstObject & instObj) const;
    bool isHole(const InstObject & instObj) const;
    EPoint2D makeEPoint2D(const Point & p) const;


private:
    // temporary data
    double m_scale = 1.0;
    UPtr<EXflDB> m_xflDB = nullptr;
    Ptr<IDatabase> m_database = nullptr;
    std::map<int, ELayerId> m_layerIdMap;//xfl to ecad
    std::map<int, ELayerId> m_metalLyrIdMap;//xfl metal Id to ecad
    std::unordered_map<std::string, ENetId> m_netIdMap;
    std::unordered_map<std::string, std::string> m_matNameMap;
    std::unordered_map<std::string, std::string> m_partNameMap;
    std::unordered_set<std::string> m_padstackInstNames;
};

ECAD_ALWAYS_INLINE EPoint2D ECadExtXflHandler::makeEPoint2D(const Point & p) const
{
    return EPoint2D(m_scale * p.x, m_scale * p.y);
}

}//namespace xfl
}//namespace ext
}//namespace ecad