#include "EPrismaThermalModelQuery.h"
#include "models/thermal/EPrismaThermalModel.h"

namespace ecad {
namespace model {
namespace utils {
ECAD_INLINE EPrismaThermalModelQuery::EPrismaThermalModelQuery(CPtr<EPrismaThermalModel> model)
 : m_model(model)
{
}

ECAD_INLINE void EPrismaThermalModelQuery::SearchPrismaInstances(const EBox2D & area, std::vector<RtVal> & results) const
{
    results.clear();
    auto rtree = BuildIndexTree();
    rtree->query(boost::geometry::index::covered_by(area), std::back_inserter(results));
}

ECAD_INLINE void EPrismaThermalModelQuery::SearchPrismaInstances(size_t layer, const EBox2D & area, std::vector<RtVal> & results) const
{
    results.clear();
    auto rtree = BuildLayerIndexTree(layer);
    rtree->query(boost::geometry::index::covered_by(area), std::back_inserter(results));
}

ECAD_INLINE void EPrismaThermalModelQuery::SearchNearestPrismaInstances(size_t layer, const EPoint2D & pt, size_t k, std::vector<RtVal> & results) const
{
    results.clear();
    auto rtree = BuildLayerIndexTree(layer);
    rtree->query(boost::geometry::index::nearest(pt, k), std::back_inserter(results));
}

ECAD_INLINE size_t EPrismaThermalModelQuery::NearestLayer(EFloat height) const
{
    using namespace generic::math;
    const auto & layers = m_model->layers;
    if (GT(height, layers.front().elevation)) return 0;
    for (size_t i = 0; i < layers.size(); ++i) {
        auto top = layers.at(i).elevation;
        auto bot = top - layers.at(i).thickness;
        if (Within<LCRO>(height, bot, top)) return i;
    }
    return layers.size();
}

ECAD_INLINE CPtr<EPrismaThermalModelQuery::Rtree> EPrismaThermalModelQuery::BuildIndexTree() const
{
    if (nullptr == m_rtree) {
        m_rtree.reset(new Rtree);
        const auto & prismas = m_model->m_prismas;
        const auto & triangulation = *m_model->GetLayerPrismaTemplate(0);
        for (size_t i = 0; i < prismas.size(); ++i) {
            auto it = prismas.at(i).element->templateId;
            auto point = tri::TriangulationUtility<EPoint2D>::GetCenter(triangulation, it).Cast<ECoord>();
            m_rtree->insert(std::make_pair(point, i));
        }
    }
    return m_rtree.get();
}

ECAD_INLINE CPtr<EPrismaThermalModelQuery::Rtree> EPrismaThermalModelQuery::BuildLayerIndexTree(size_t layer) const
{
    if (auto iter = m_lyrRtrees.find(layer); iter != m_lyrRtrees.cend()) return iter->second.get();
    auto & rtree = m_lyrRtrees.emplace(layer, std::make_shared<Rtree>()).first->second;
    const auto & prismas = m_model->m_prismas;
    const auto & indexOffset = m_model->m_indexOffset;
    const auto & triangulation = *m_model->GetLayerPrismaTemplate(layer);
    for (size_t i = indexOffset.at(layer); i < indexOffset.at(layer + 1); ++i) {
        ECAD_ASSERT(layer == prismas.at(i).layer->id)
        auto it = prismas.at(i).element->templateId;
        auto point = tri::TriangulationUtility<EPoint2D>::GetCenter(triangulation, it).Cast<ECoord>();
        rtree->insert(std::make_pair(point, i));
    }    
    return rtree.get();
}

}//namespace utils
}//namespace model
}//namespace ecad
