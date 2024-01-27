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
    explicit EPrismaThermalModelQuery(CPtr<EPrismaThermalModel> model);
    virtual ~EPrismaThermalModelQuery() = default;
    void SearchPrismaInstances(const EBox2D & area, std::vector<size_t> & indices) const;
    void SearchPrismaInstances(size_t layer, const EPoint2D & pt, std::vector<size_t> & indices) const;
    size_t NearestLayer(EFloat height) const;

protected:
    void BuildIndexTree() const;
protected:
    CPtr<EPrismaThermalModel> m_model{nullptr};

    using RtVal = std::pair<EPoint2D, size_t>;
    using Rtree = boost::geometry::index::rtree<RtVal, boost::geometry::index::rstar<8>>;
    mutable UPtr<Rtree> m_rtree{nullptr};
};
} // namespace utils
} // namespace model
} // namespace ecad