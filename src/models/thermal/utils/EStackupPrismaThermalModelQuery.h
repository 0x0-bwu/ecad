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
    explicit EStackupPrismaThermalModelQuery(CPtr<EStackupPrismaThermalModel> model);
    virtual ~EStackupPrismaThermalModelQuery() = default;
    
    EFloat PrismaInstanceTopBotArea(size_t pIndex) const;//in coords
    ETriangle2D GetPrismaInstanceTemplate(size_t pIndex) const;
    void IntersectsPrismaInstance(size_t layer, size_t pIndex, std::vector<RtVal> & results) const;
protected:
    CPtr<Rtree> BuildLayerIndexTree(size_t layer) const;
protected:
    CPtr<EStackupPrismaThermalModel> m_model{nullptr};
    mutable std::unordered_map<size_t, SPtr<Rtree> > m_lyrRtrees;//todo reuse imprint layer's tree
};
} // namespace utils
} // namespace model
} // namespace ecad