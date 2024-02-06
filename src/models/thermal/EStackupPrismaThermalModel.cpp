#include "EStackupPrismaThermalModel.h"
#include "models/thermal/utils/EStackupPrismaThermalModelQuery.h"
#include "models/geometry/ELayerCutModel.h"

#include "generic/geometry/BooleanOperation.hpp"

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
    auto getIntersectArea = [](const ETriangle2D & t1, const ETriangle2D & t2) {
        EFloat area = 0;
        std::vector<EPolygonData> output;
        generic::geometry::boolean::Intersect(t1, t2, output);
        std::for_each(output.begin(), output.end(), [&area](const EPolygonData & p) { area += p.Area(); });
        return area;
    };
    utils::EStackupPrismaThermalModelQuery query(this);
    using RtVal = model::utils::EStackupPrismaThermalModelQuery::RtVal;
    for (size_t i = 0; i < total; ++i) {
        auto & instance = m_prismas[i];
        auto [lyrIdx, eleIdx] = PrismaLocalIndex(i);
        auto triangle1 = query.GetPrismaInstanceTemplate(i);
        auto triangle1Area = triangle1.Area();
        if (isTopLayer(lyrIdx)) instance.neighbors[PrismaElement::TOP_NEIGHBOR_INDEX] = noNeighbor;
        else {
            auto topLyr = lyrIdx - 1;
            std::vector<RtVal> results;
            query.IntersectsPrismaInstances(topLyr, i, results);
            for (size_t j = 0; j < results.size(); ++j) {
                auto triangle2 = query.GetPrismaInstanceTemplate(results.at(j).second);
                if (auto area = getIntersectArea(triangle1, triangle2); area > 0)
                   instance.contactInstances.front().emplace_back(results.at(j).second, EFloat(area) / triangle1Area); 
            }
            if (instance.contactInstances.empty())
                instance.neighbors[PrismaElement::TOP_NEIGHBOR_INDEX] = noNeighbor;
            else instance.neighbors[PrismaElement::TOP_NEIGHBOR_INDEX] = i;
        }

        if (isBotLayer(lyrIdx)) instance.neighbors[PrismaElement::BOT_NEIGHBOR_INDEX] = noNeighbor;
        else {
            auto botLyr = lyrIdx + 1;
            std::vector<RtVal> results;
            query.IntersectsPrismaInstances(botLyr, i, results);
            for (size_t j = 0; j < results.size(); ++j) {
                auto triangle2 = query.GetPrismaInstanceTemplate(results.at(j).second);
                if (auto area = getIntersectArea(triangle1, triangle2); area > 0)
                    instance.contactInstances.back().emplace_back(results.at(j).second, EFloat(area) / triangle1Area);
            }
            if (instance.contactInstances.empty())
                instance.neighbors[PrismaElement::BOT_NEIGHBOR_INDEX] = noNeighbor;
            else instance.neighbors[PrismaElement::BOT_NEIGHBOR_INDEX] = i;
        }
    }
}

ECAD_INLINE void EStackupPrismaThermalModel::AddBondWiresFromLayerCutModel(CPtr<ELayerCutModel> lcm)
{
    utils::EStackupPrismaThermalModelQuery query(this);
    for (const auto & bondwire : lcm->GetAllBondwires()) {
        const auto & pts = bondwire.pt2ds;
        ECAD_ASSERT(pts.size() == bondwire.heights.size());
        for (size_t curr = 0; curr < pts.size() - 1; ++curr) {
            auto next = curr + 1;
            auto p1 = FPoint3D(pts.at(curr)[0] * m_scaleH2Unit, pts.at(curr)[1] * m_scaleH2Unit, bondwire.heights.at(curr));
            auto p2 = FPoint3D(pts.at(next)[0] * m_scaleH2Unit, pts.at(next)[1] * m_scaleH2Unit, bondwire.heights.at(next));
            auto & line = AddLineElement(std::move(p1), std::move(p2), bondwire.netId, bondwire.matId, bondwire.radius, bondwire.current);
            //connection
            if (0 == curr) {
                std::vector<utils::EStackupPrismaThermalModelQuery::RtVal> results;
                auto layer = lcm->GetLayerIndexByHeight(lcm->GetHeight(bondwire.heights.front()));
                query.SearchNearestPrismaInstances(layer, pts.at(curr), 1, results);
                ECAD_ASSERT(results.size() == 1)
                std::for_each(results.begin(), results.end(), [&](const auto & r) { line.neighbors.front().push_back(r.second); });
            }
            else {
                auto & prevLine = m_lines.at(m_lines.size() - 2);
                line.neighbors.front().emplace_back(prevLine.id);
                prevLine.neighbors.back().emplace_back(line.id);
            }
            if (next == pts.size() - 1) {
                std::vector<utils::EStackupPrismaThermalModelQuery::RtVal> results;
                auto layer = lcm->GetLayerIndexByHeight(lcm->GetHeight(bondwire.heights.back()));
                query.SearchNearestPrismaInstances(layer, pts.at(next), 1, results);
                ECAD_ASSERT(results.size() == 1)
                std::for_each(results.begin(), results.end(), [&](const auto & r) { line.neighbors.back().push_back(r.second); });
            }
        }
    }
}

} //namespace ecad::model