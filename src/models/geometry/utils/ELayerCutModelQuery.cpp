#include "ELayerCutModelQuery.h"
#include "models/geometry/ELayerCutModel.h"

namespace ecad {
namespace model {
namespace utils {
ECAD_INLINE ELayerCutModelQuery::ELayerCutModelQuery(CPtr<ELayerCutModel> model)
 : m_model(model)
{
}

ECAD_INLINE size_t ELayerCutModelQuery::SearchPolygon(size_t layer, const EPoint2D & pt) const
{
    if (not m_model->hasPolygon(layer)) return invalidIndex;

    std::vector<RtVal> results;
    BuildLayerIndexTree(layer)->query(boost::geometry::index::intersects(EBox2D(pt, pt)), std::back_inserter(results));
    if (results.empty()) return invalidIndex;
    const auto & polygons = m_model->m_polygons;
    auto cmp = [&](auto i1, auto i2){ return polygons.at(i1).Area() > polygons.at(i2).Area(); };
    std::priority_queue<size_t, std::vector<size_t>, decltype(cmp)> pq(cmp);
    for (const auto & result : results) {
        if (generic::geometry::Contains(polygons.at(result.second), pt))
            pq.emplace(result.second);
    }
    if (not pq.empty()) return pq.top();
    return invalidIndex;
}

ECAD_INLINE CPtr<ELayerCutModelQuery::Rtree> ELayerCutModelQuery::BuildLayerIndexTree(size_t layer) const
{
    if (auto iter = m_rtrees.find(layer); iter != m_rtrees.cend()) return iter->second.get();
    auto & rtree = m_rtrees.emplace(layer, std::make_shared<Rtree>()).first->second;
    for (auto i : m_model->m_lyrPolygons.at(layer)) {
        auto bbox = generic::geometry::Extent(m_model->m_polygons.at(i));
        rtree->insert(std::make_pair(bbox, i));
    }
    return rtree.get();
}

}//namespace utils
}//namespace model
}//namespace ecad
