#include "EStackupPrismaThermalModelQuery.h"
#include "models/thermal/EStackupPrismaThermalModel.h"
#include "EDataMgr.h"
namespace ecad::model::utils {
ECAD_INLINE EStackupPrismaThermalModelQuery::EStackupPrismaThermalModelQuery(CPtr<EStackupPrismaThermalModel> model, bool lazyBuild)
 : m_model(model)
{
    if (not lazyBuild) {
        generic::thread::ThreadPool pool(EDataMgr::Instance().Threads());
        for (size_t i = 0; i < m_model->TotalLayers(); ++i)
            pool.Submit(std::bind(&EStackupPrismaThermalModelQuery::BuildLayerIndexTree, this, i));  
    }
}

ECAD_INLINE EFloat EStackupPrismaThermalModelQuery::PrismaInstanceTopBotArea(size_t pIndex) const
{
    return GetPrismaInstanceTemplate(pIndex).Area();
}

ECAD_INLINE ETriangle2D EStackupPrismaThermalModelQuery::GetPrismaInstanceTemplate(size_t pIndex) const
{
    const auto & instance = m_model->GetPrisma(pIndex);
    const auto & triangulation = *m_model->GetLayerPrismaTemplate(instance.layer->id);
    return tri::TriangulationUtility<EPoint2D>::GetTriangle(triangulation, instance.element->templateId);
}

ECAD_INLINE void EStackupPrismaThermalModelQuery::IntersectsPrismaInstances(size_t layer, size_t pIndex, std::vector<RtVal> & results) const
{
    const auto & instance = m_model->GetPrisma(pIndex);
    const auto & triangulation = *m_model->GetLayerPrismaTemplate(instance.layer->id);
    auto area = tri::TriangulationUtility<EPoint2D>::GetBondBox(triangulation, instance.element->templateId);
    return SearchPrismaInstances(layer, area, results);
}

ECAD_INLINE void EStackupPrismaThermalModelQuery::SearchPrismaInstances(size_t layer, const EBox2D & area, std::vector<RtVal> & results) const
{
    results.clear();
    auto rtree = BuildLayerIndexTree(layer);
    rtree->query(boost::geometry::index::intersects(area), std::back_inserter(results));
}

ECAD_INLINE CPtr<EStackupPrismaThermalModelQuery::Rtree> EStackupPrismaThermalModelQuery::BuildLayerIndexTree(size_t layer) const
{
    if (auto iter = m_lyrRtrees.find(layer); iter != m_lyrRtrees.cend()) return iter->second.get();
    
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto iter = m_lyrRtrees.find(layer); iter != m_lyrRtrees.cend()) return iter->second.get();
    auto rtree = std::make_shared<Rtree>();
    const auto & prismas = m_model->m_prismas;
    const auto & indexOffset = m_model->m_indexOffset;
    const auto & triangulation = *m_model->GetLayerPrismaTemplate(layer);
    for (size_t i = indexOffset.at(layer); i < indexOffset.at(layer + 1); ++i) {
        ECAD_ASSERT(layer == prismas.at(i).layer->id)
        auto it = prismas.at(i).element->templateId;
        auto box = tri::TriangulationUtility<EPoint2D>::GetBondBox(triangulation, it);
        rtree->insert(std::make_pair(std::move(box), i));
    }
    return m_lyrRtrees.emplace(layer, rtree).first->second.get();
}

ECAD_INLINE void EStackupPrismaThermalModelQuery::SearchNearestPrismaInstances(size_t layer, const EPoint2D & pt, size_t k, std::vector<RtVal> & results) const
{
    results.clear();
    auto rtree = BuildLayerIndexTree(layer);
    rtree->query(boost::geometry::index::nearest(pt, k), std::back_inserter(results));
}

ECAD_INLINE size_t EStackupPrismaThermalModelQuery::NearestLayer(EFloat height) const
{
    using namespace generic::math;
    const auto & layers = m_model->layers;
    if (GT(height, layers.front().elevation)) return 0;
    for (size_t i = 0; i < layers.size(); ++i) {
        auto top = layers.at(i).elevation;
        auto bot = top - layers.at(i).thickness;
        if (Within<LCRO>(height, bot, top)) return i;
    }
    return layers.size() - 1;
}


} // namespace ecad::model::utils