#include "EStackupPrismaThermalModel.h"
#include "models/thermal/utils/EStackupPrismaThermalModelQuery.h"
namespace ecad::model {

ECAD_INLINE EStackupPrismaThermalModel::EStackupPrismaThermalModel(CPtr<ILayoutView> layout)
 : EPrismaThermalModel(layout)
{
}

ECAD_INLINE void EStackupPrismaThermalModel::BuildPrismaModel(EFloat scaleH2Unit, EFloat scale2Meter)
{
    m_scaleH2Unit = scaleH2Unit;
    m_scale2Meter = scale2Meter;
    m_indexOffset = std::vector<size_t>{0};
    for (size_t i = 0; i < TotalLayers(); ++i)
        m_indexOffset.emplace_back(m_indexOffset.back() + layers.at(i).TotalElements());
    

    auto total = TotalPrismaElements();
    m_prismas.resize(total);
    m_points.clear();
    for (size_t i = 0; i < total; ++i) {
        auto & instance = m_prismas[i];
        auto [lyrIdx, eleIdx] = PrismaLocalIndex(i);
        instance.layer = &layers.at(lyrIdx);
        instance.element = &instance.layer->elements.at(eleIdx);    
        //points
        const auto & triangles = m_prismaTemplates.at(lyrIdx)->triangles;
        const auto & vertices = triangles.at(instance.element->templateId).vertices;
        for (size_t v = 0; v < vertices.size(); ++v) {
            instance.vertices[v] = AddPoint(GetPoint(lyrIdx, eleIdx, v));
            instance.vertices[v + 3] = AddPoint(GetPoint(lyrIdx, eleIdx, v + 3));
        }

        //side neighbors
        for (size_t n = 0; n < 3; ++n) {
            if (auto nid = instance.element->neighbors.at(n); noNeighbor != nid) {
                auto nb = GlobalIndex(lyrIdx, instance.element->neighbors.at(n));
                instance.neighbors[n] = nb;
            }
        }
    }
    //top/bot neighbors
    utils::EStackupPrismaThermalModelQuery query(this);
    using RtVal = model::utils::EStackupPrismaThermalModelQuery::RtVal;
    for (size_t i = 0; i < total; ++i) {
        auto & instance = m_prismas[i];
        auto [lyrIdx, eleIdx] = PrismaLocalIndex(i);
        const auto & triangulation = *GetLayerPrismaTemplate(lyrIdx);
        auto it = instance.element->templateId;
        auto triangle = tri::TriangulationUtility<EPoint2D>::GetTriangle(triangulation, it);
        if (isTopLayer(lyrIdx)) instance.neighbors[PrismaElement::TOP_NEIGHBOR_INDEX] = noNeighbor;
        else {
            auto topLyr = lyrIdx - 1;
            std::vector<RtVal> results;
            query.IntersectsPrismaInstance(topLyr, i, results);
            for (size_t j = 0; j < results.size(); ++j) {
                //todo
            }
        }
    }
}
} //namespace ecad::model