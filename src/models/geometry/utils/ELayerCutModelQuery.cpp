#include "ELayerCutModelQuery.h"
#include "models/geometry/ELayerCutModel.h"

namespace ecad {
namespace model {
namespace utils {
ECAD_INLINE ELayerCutModelQuery::ELayerCutModelQuery(CPtr<ELayerCutModel> model)
 : m_model(model)
{
    const auto & layerPolygons = m_model->m_lyrPolygons;
    for (size_t lyr = 0; lyr < m_model->TotalLayers(); ++lyr) {
        if (lyr > 0 && layerPolygons.at(lyr) == layerPolygons.at(lyr - 1))
            m_rtrees.emplace(lyr, m_rtrees.at(lyr - 1));
        else {
            auto & rtree = m_rtrees.emplace(lyr, std::make_shared<Rtree>()).first->second;
            for (auto i : *layerPolygons.at(lyr)) {
                auto bbox = generic::geometry::Extent(m_model->m_polygons.at(i));
                rtree->insert(std::make_pair(bbox, i));
            }
        }
    }
}

ECAD_INLINE size_t ELayerCutModelQuery::SearchPolygon(size_t layer, const EPoint2D & pt) const
{
    if (not m_model->hasPolygon(layer)) return invalidIndex;

    std::vector<RtVal> results;
    m_rtrees.at(layer)->query(boost::geometry::index::intersects(EBox2D(pt, pt)), std::back_inserter(results));
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

}//namespace utils
}//namespace model
}//namespace ecad
