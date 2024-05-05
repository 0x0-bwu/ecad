#pragma once
#include "basic/ECadCommon.h"
#include <boost/geometry/index/rtree.hpp>

namespace ecad {
namespace model {

class EPrismThermalModel;
namespace utils {
class ECAD_API EPrismThermalModelQuery
{
public:
    using RtVal = std::pair<EPoint2D, size_t>;
    using Rtree = boost::geometry::index::rtree<RtVal, boost::geometry::index::rstar<8>>;
    explicit EPrismThermalModelQuery(CPtr<EPrismThermalModel> model, bool lazyBuild = true);
    virtual ~EPrismThermalModelQuery() = default;
    void SearchPrismInstances(const EBox2D & area, std::vector<RtVal> & results) const;
    void SearchPrismInstances(size_t layer, const EBox2D & area, std::vector<RtVal> & results) const;
    void SearchNearestPrismInstances(size_t layer, const EPoint2D & pt, size_t k, std::vector<RtVal> & results) const;
    size_t NearestLayer(EFloat height) const;

protected:
    CPtr<Rtree> BuildIndexTree() const;
    CPtr<Rtree> BuildLayerIndexTree(size_t layer) const;
protected:
    CPtr<EPrismThermalModel> m_model{nullptr};

    mutable std::mutex m_mutex;
    mutable UPtr<Rtree> m_rtree{nullptr};
    mutable std::unordered_map<size_t, SPtr<Rtree> > m_lyrRtrees;
};
} // namespace utils
} // namespace model
} // namespace ecad