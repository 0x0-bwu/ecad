#include "EStackupPrismaThermalModelQuery.h"
#include "models/thermal/EStackupPrismaThermalModel.h"

namespace ecad::model::utils {
ECAD_INLINE EStackupPrismaThermalModelQuery::EStackupPrismaThermalModelQuery(CPtr<EStackupPrismaThermalModel> model)
 : m_model(model)
{
}

ECAD_INLINE void EStackupPrismaThermalModelQuery::IntersectsPrismaInstance(size_t layer, size_t pIndex, std::vector<RtVal> & results) const
{
    results.clear();
    auto rtree = BuildLayerIndexTree(layer);
    auto it = m_model->m_prismas.at(pIndex).element->templateId;
    const auto & triangulation = *m_model->GetLayerPrismaTemplate(layer);
    auto area = tri::TriangulationUtility<EPoint2D>::GetBondBox(triangulation, it);
    rtree->query(boost::geometry::index::intersects(area), std::back_inserter(results));
}

ECAD_INLINE CPtr<EStackupPrismaThermalModelQuery::Rtree> EStackupPrismaThermalModelQuery::BuildLayerIndexTree(size_t layer) const
{
    if (auto iter = m_lyrRtrees.find(layer); iter != m_lyrRtrees.cend()) return iter->second.get();
    auto & rtree = m_lyrRtrees.emplace(layer, std::make_shared<Rtree>()).first->second;
    const auto & prismas = m_model->m_prismas;
    const auto & indexOffset = m_model->m_indexOffset;
    const auto & triangulation = *m_model->GetLayerPrismaTemplate(layer);
    for (size_t i = indexOffset.at(layer); i < indexOffset.at(layer + 1); ++i) {
        ECAD_ASSERT(layer == prismas.at(i).layer->id)
        auto it = prismas.at(i).element->templateId;
        auto box = tri::TriangulationUtility<EPoint2D>::GetBondBox(triangulation, it);
        rtree->insert(std::make_pair(std::move(box), i));
    }    
    return rtree.get();
}

} // namespace ecad::model::utils