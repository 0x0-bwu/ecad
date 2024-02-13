#pragma once
#include <boost/geometry/index/rtree.hpp>
#include "ECadCommon.h"

namespace ecad {
namespace model {

class EPrismaThermalModel;
namespace utils {
class ECAD_API EPrismaThermalModelQuery
{
public:
    using RtVal = std::pair<EPoint2D, size_t>;
    using Rtree = boost::geometry::index::rtree<RtVal, boost::geometry::index::rstar<8>>;
    explicit EPrismaThermalModelQuery(CPtr<EPrismaThermalModel> model, bool lazyBuild = true);
    virtual ~EPrismaThermalModelQuery() = default;
    void SearchPrismaInstances(const EBox2D & area, std::vector<RtVal> & results) const;
    void SearchPrismaInstances(size_t layer, const EBox2D & area, std::vector<RtVal> & results) const;
    void SearchNearestPrismaInstances(size_t layer, const EPoint2D & pt, size_t k, std::vector<RtVal> & results) const;
    size_t NearestLayer(EFloat height) const;

protected:
    CPtr<Rtree> BuildIndexTree() const;
    CPtr<Rtree> BuildLayerIndexTree(size_t layer) const;
protected:
    CPtr<EPrismaThermalModel> m_model{nullptr};

    mutable std::mutex m_mutex;
    mutable UPtr<Rtree> m_rtree{nullptr};
    mutable std::unordered_map<size_t, SPtr<Rtree> > m_lyrRtrees;
};
} // namespace utils
} // namespace model
} // namespace ecad