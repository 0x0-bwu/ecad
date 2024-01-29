#pragma once
#include <boost/geometry/index/rtree.hpp>
#include "ECadCommon.h"

namespace ecad {
namespace model {

class ELayerCutModel;
namespace utils {
class ECAD_API ELayerCutModelQuery
{
public:
    using RtVal = std::pair<EBox2D, size_t>;
    using Rtree = boost::geometry::index::rtree<RtVal, boost::geometry::index::rstar<8>>;
    explicit ELayerCutModelQuery(CPtr<ELayerCutModel> model);
    virtual ~ELayerCutModelQuery() = default;

    size_t SearchPolygon(size_t layer, const EPoint2D & pt) const;
    
protected:
    CPtr<ELayerCutModel> m_model{nullptr};
    mutable std::unordered_map<size_t, std::shared_ptr<Rtree> > m_rtrees;
};
} // namespace utils
} // namespace model
} // namespace ecad