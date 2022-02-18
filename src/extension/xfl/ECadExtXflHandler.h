#ifndef ECAD_EXT_XFL_ECADEXTXFLHANDLER_H
#define ECAD_EXT_XFL_ECADEXTXFLHANDLER_H
#include "extension/xfl/EXflObjects.h"
#include "ECadCommon.h"
#include "Interface.h"
namespace ecad {
namespace ext {
namespace xfl {

class ECAD_API ECadExtXflHandler
{
public:
    explicit ECadExtXflHandler(const std::string & xflFile, size_t circleDiv = 12);
    SPtr<IDatabase> CreateDatabase(const std::string & name, std::string * err = nullptr);

private:
    void ImportPadstackDefs();
    void ImportLayers(Ptr<ILayoutView> layout);
    void ImportNets(Ptr<ILayoutView> layout);
    void ImportConnObjs(Ptr<ILayoutView> layout);

    void Reset();

private:
    std::string m_xflFile;
    size_t m_circleDiv = 12;

private:
    // temporary data
    double m_scale = 1.0;
    UPtr<EXflDB> m_xflDB = nullptr;
    SPtr<IDatabase> m_database = nullptr;
    std::map<int, ELayerId> m_layerIdMap;//xfl to ecad
    std::map<int, ELayerId> m_metalLyrIdMap;//xfl metal Id to ecad
    std::unordered_map<std::string, ENetId> m_netIdMap;
};

}//namespace xfl
}//namespace ext
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "ECadExtXflHandler.cpp"
#endif

#endif//ECAD_EXT_XFL_ECADEXTXFLHANDLER_H