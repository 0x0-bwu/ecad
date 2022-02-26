#ifndef ECAD_EUIT_ELAYOUTPOLYGONMERGER_H
#define ECAD_EUIT_ELAYOUTPOLYGONMERGER_H
#include "ECadSettings.h"
#include <unordered_map>
namespace generic {
namespace geometry {

template <typename property_type, typename num_type> class PolygonMerger;

}//namespace geometry
}//namesapce generic
namespace ecad {

class EShape;
class ILayoutView;
namespace euti {

class ECAD_API ELayoutPolygonMerger
{
    using LayerMerger = generic::geometry::PolygonMerger<ENetId, ECoord>;
public:
    explicit ELayoutPolygonMerger(Ptr<ILayoutView> layout);
    ~ELayoutPolygonMerger();

    void SetLayoutMergeSettings(ELayoutPolygonMergeSettings settings);
    void Merge();

private:
    void FillPolygonsFromLayout();
    void MergeLayers();
    void MergeOneLayer(Ptr<LayerMerger> merger);
    void FillPolygonsBackToLayout();
    bool FillOneShape(ENetId netId, ELayerId layerId, Ptr<EShape> shape);
private:
    Ptr<ILayoutView> m_layout;
    ELayoutPolygonMergeSettings m_settings;
    std::unordered_set<size_t> m_primTobeRemove;
    std::unordered_map<ELayerId, UPtr<LayerMerger> > m_mergers;
};

}//namespace euti
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "utility/ELayoutPolygonMerger.cpp"
#endif

#endif//ECAD_EUIT_ELAYOUTPOLYGONMERGER_H
