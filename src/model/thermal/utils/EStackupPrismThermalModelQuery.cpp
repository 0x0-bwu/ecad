#include "EStackupPrismThermalModelQuery.h"
#include "model/thermal/EStackupPrismThermalModel.h"
#include "EDataMgr.h"
namespace ecad::model::utils {
ECAD_INLINE EStackupPrismThermalModelQuery::EStackupPrismThermalModelQuery(CPtr<EStackupPrismThermalModel> model, bool lazyBuild)
 : m_model(model)
{
    if (not lazyBuild) {
        generic::thread::ThreadPool pool(EDataMgr::Instance().Threads());
        for (size_t i = 0; i < m_model->TotalLayers(); ++i)
            pool.Submit(std::bind(&EStackupPrismThermalModelQuery::BuildLayerIndexTree, this, i));  
    }
}

ECAD_INLINE EFloat EStackupPrismThermalModelQuery::PrismInstanceTopBotArea(size_t pIndex) const
{
    return GetPrismInstanceTemplate(pIndex).Area();
}

ECAD_INLINE ETriangle2D EStackupPrismThermalModelQuery::GetPrismInstanceTemplate(size_t pIndex) const
{
    const auto & instance = m_model->GetPrism(pIndex);
    const auto & triangulation = *m_model->GetLayerPrismTemplate(instance.layer);
    return tri::TriangulationUtility<EPoint2D>::GetTriangle(triangulation, m_model->GetPrismElement(instance.layer, instance.element).templateId);
}

ECAD_INLINE void EStackupPrismThermalModelQuery::IntersectsPrismInstances(size_t layer, size_t pIndex, std::vector<RtVal> & results) const
{
    const auto & instance = m_model->GetPrism(pIndex);
    const auto & triangulation = *m_model->GetLayerPrismTemplate(instance.layer);
    auto area = tri::TriangulationUtility<EPoint2D>::GetBondBox(triangulation, m_model->GetPrismElement(instance.layer, instance.element).templateId);
    return SearchPrismInstances(layer, area, results);
}

ECAD_INLINE void EStackupPrismThermalModelQuery::SearchPrismInstances(size_t layer, const EBox2D & area, std::vector<RtVal> & results) const
{
    results.clear();
    auto rtree = BuildLayerIndexTree(layer);
    rtree->query(boost::geometry::index::intersects(area), std::back_inserter(results));
}

ECAD_INLINE CPtr<EStackupPrismThermalModelQuery::Rtree> EStackupPrismThermalModelQuery::BuildLayerIndexTree(size_t layer) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto iter = m_lyrRtrees.find(layer); iter != m_lyrRtrees.cend()) return iter->second.get();
    auto rtree = std::make_shared<Rtree>();
    const auto & prisms = m_model->m_prisms;
    const auto & indexOffset = m_model->m_indexOffset;
    const auto & triangulation = *m_model->GetLayerPrismTemplate(layer);
    for (size_t i = indexOffset.at(layer); i < indexOffset.at(layer + 1); ++i) {
        const auto & prism = prisms.at(i); { ECAD_ASSERT(layer == prism.layer) }
        const auto & element = m_model->GetPrismElement(prism.layer, prism.element);
        auto box = tri::TriangulationUtility<EPoint2D>::GetBondBox(triangulation, element.templateId);
        rtree->insert(std::make_pair(std::move(box), i));
    }
    return m_lyrRtrees.emplace(layer, rtree).first->second.get();
}

ECAD_INLINE void EStackupPrismThermalModelQuery::SearchNearestPrismInstances(size_t layer, const EPoint2D & pt, size_t k, std::vector<RtVal> & results) const
{
    results.clear();
    auto rtree = BuildLayerIndexTree(layer);
    rtree->query(boost::geometry::index::nearest(pt, k), std::back_inserter(results));
}

ECAD_INLINE size_t EStackupPrismThermalModelQuery::NearestLayer(EFloat height) const
{
    using namespace generic::math;
    const auto & layers = m_model->layers;
    if (GE(height, layers.front().elevation)) return 0;
    for (size_t i = 0; i < layers.size(); ++i) {
        auto top = layers.at(i).elevation;
        auto bot = top - layers.at(i).thickness;
        if (Within<LCRO>(height, bot, top)) return i;
    }
    return layers.size() - 1;
}


} // namespace ecad::model::utils