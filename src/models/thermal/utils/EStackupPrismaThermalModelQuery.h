#pragma once
#include <boost/geometry/index/rtree.hpp>
#include "ECadCommon.h"

namespace ecad {
namespace model {

class EStackupPrismaThermalModel;
namespace utils {
class ECAD_API EStackupPrismaThermalModelQuery
{
public:
    using RtVal = std::pair<EBox2D, size_t>;
    using Rtree = boost::geometry::index::rtree<RtVal, boost::geometry::index::rstar<8>>;
    explicit EStackupPrismaThermalModelQuery(CPtr<EStackupPrismaThermalModel> model, bool lazyBuild = false);
    virtual ~EStackupPrismaThermalModelQuery() = default;
    
    EFloat PrismaInstanceTopBotArea(size_t pIndex) const;//in coords
    ETriangle2D GetPrismaInstanceTemplate(size_t pIndex) const;

    void IntersectsPrismaInstances(size_t layer, size_t pIndex, std::vector<RtVal> & results) const;
    void SearchPrismaInstances(size_t layer, const EBox2D & area, std::vector<RtVal> & results) const;
    void SearchNearestPrismaInstances(size_t layer, const EPoint2D & pt, size_t k, std::vector<RtVal> & results) const;
    size_t NearestLayer(EFloat height) const;
protected:
    CPtr<Rtree> BuildLayerIndexTree(size_t layer) const;
protected:
    mutable std::mutex m_mutex;
    CPtr<EStackupPrismaThermalModel> m_model{nullptr};
    mutable std::unordered_map<size_t, SPtr<Rtree> > m_lyrRtrees;//todo reuse imprint layer's tree
};
} // namespace utils
} // namespace model
} // namespace ecad