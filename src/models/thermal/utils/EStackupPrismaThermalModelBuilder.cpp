#include "EStackupPrismaThermalModelBuilder.h"
#include "models/thermal/EStackupPrismaThermalModel.h"
#include "EStackupPrismaThermalModelQuery.h"
#include "models/geometry/ELayerCutModel.h"

#include "generic/geometry/BooleanOperation.hpp"
#include "generic/thread/ThreadPool.hpp"

namespace ecad::model::utils {
ECAD_INLINE EStackupPrismaThermalModelBuilder::EStackupPrismaThermalModelBuilder(Ptr<EStackupPrismaThermalModel> model)
 : m_model(model)
{
    m_query.reset(new EStackupPrismaThermalModelQuery(m_model));
}

ECAD_INLINE EStackupPrismaThermalModelBuilder::~EStackupPrismaThermalModelBuilder()
{
}

ECAD_INLINE void EStackupPrismaThermalModelBuilder::BuildPrismaModel(EFloat scaleH2Unit, EFloat scale2Meter, size_t threads)
{
    m_model->m_scaleH2Unit = scaleH2Unit;
    m_model->m_scale2Meter = scale2Meter;
    m_model->m_indexOffset = std::vector<size_t>{0};
    for (size_t i = 0; i < m_model->TotalLayers(); ++i)
        m_model->m_indexOffset.emplace_back(m_model->m_indexOffset.back() + m_model->layers.at(i).TotalElements());
    
    auto total = m_model->TotalPrismaElements();
    m_model->m_prismas.resize(total);
    m_model->m_points.resize(total * 6);
    for (size_t i = 0; i < total; ++i) {
        auto & instance = m_model->m_prismas[i];
        auto [lyrIdx, eleIdx] = m_model->PrismaLocalIndex(i);
        instance.layer = &m_model->layers.at(lyrIdx);
        instance.element = &instance.layer->elements.at(eleIdx);    
        //points
        for (size_t v = 0; v < 6; ++v) {
            m_model->m_points[6 * i + v] = m_model->GetPoint(lyrIdx, eleIdx, v);
            instance.vertices[v] = 6 * i + v;
        }

        //side neighbors
        for (size_t n = 0; n < 3; ++n) {
            if (auto nid = instance.element->neighbors.at(n); noNeighbor != nid) {
                auto nb = m_model->GlobalIndex(lyrIdx, instance.element->neighbors.at(n));
                instance.neighbors[n] = nb;
            }
        }
    }

    if (threads > 1) {
        generic::thread::ThreadPool pool(threads);
        size_t size = m_model->TotalPrismaElements();
        size_t blocks = pool.Threads();
        size_t blockSize = size / blocks;

        size_t begin = 0;
        for(size_t i = 0; i < blocks && blockSize > 0; ++i){
            size_t end = begin + blockSize;
            pool.Submit(std::bind(&EStackupPrismaThermalModelBuilder::BuildPrismaInstanceTopBotNeighbors, this, begin, end));
            begin = end;
        }
        size_t end = size;
        if(begin != end)
            pool.Submit(std::bind(&EStackupPrismaThermalModelBuilder::BuildPrismaInstanceTopBotNeighbors, this, begin, end));        
    }
    else BuildPrismaInstanceTopBotNeighbors(0, m_model->TotalPrismaElements());
}

ECAD_INLINE void EStackupPrismaThermalModelBuilder::BuildPrismaInstanceTopBotNeighbors(size_t start, size_t end)
{
    using RtVal = model::utils::EStackupPrismaThermalModelQuery::RtVal;
    for (size_t i = start; i < end; ++i) {
        auto & instance = m_model->m_prismas[i];
        auto [lyrIdx, eleIdx] = m_model->PrismaLocalIndex(i);
        auto triangle1 = m_query->GetPrismaInstanceTemplate(i);
        auto triangle1Area = triangle1.Area();
        if (m_model->isTopLayer(lyrIdx)) instance.neighbors[PrismaElement::TOP_NEIGHBOR_INDEX] = noNeighbor;
        else {
            auto topLyr = lyrIdx - 1;
            std::vector<RtVal> results;
            m_query->IntersectsPrismaInstances(topLyr, i, results);
            for (size_t j = 0; j < results.size(); ++j) {
                auto triangle2 = m_query->GetPrismaInstanceTemplate(results.at(j).second);
                if (auto area = GetIntersectArea(triangle1, triangle2); area > 0)
                   instance.contactInstances.front().emplace_back(results.at(j).second, EFloat(area) / triangle1Area); 
            }
            if (instance.contactInstances.empty())
                instance.neighbors[PrismaElement::TOP_NEIGHBOR_INDEX] = noNeighbor;
            else instance.neighbors[PrismaElement::TOP_NEIGHBOR_INDEX] = i;
        }

        if (m_model->isBotLayer(lyrIdx)) instance.neighbors[PrismaElement::BOT_NEIGHBOR_INDEX] = noNeighbor;
        else {
            auto botLyr = lyrIdx + 1;
            std::vector<RtVal> results;
            m_query->IntersectsPrismaInstances(botLyr, i, results);
            for (size_t j = 0; j < results.size(); ++j) {
                auto triangle2 = m_query->GetPrismaInstanceTemplate(results.at(j).second);
                if (auto area = GetIntersectArea(triangle1, triangle2); area > 0)
                    instance.contactInstances.back().emplace_back(results.at(j).second, EFloat(area) / triangle1Area);
            }
            if (instance.contactInstances.empty())
                instance.neighbors[PrismaElement::BOT_NEIGHBOR_INDEX] = noNeighbor;
            else instance.neighbors[PrismaElement::BOT_NEIGHBOR_INDEX] = i;
        }
    }
}

ECAD_INLINE void EStackupPrismaThermalModelBuilder::AddBondWiresFromLayerCutModel(CPtr<ELayerCutModel> lcm)
{
    for (const auto & bondwire : lcm->GetAllBondwires()) {
        const auto & pts = bondwire.pt2ds;
        ECAD_ASSERT(pts.size() == bondwire.heights.size());
        for (size_t curr = 0; curr < pts.size() - 1; ++curr) {
            auto next = curr + 1;
            auto p1 = FPoint3D(pts.at(curr)[0] * m_model->m_scaleH2Unit, pts.at(curr)[1] * m_model->m_scaleH2Unit, bondwire.heights.at(curr));
            auto p2 = FPoint3D(pts.at(next)[0] * m_model->m_scaleH2Unit, pts.at(next)[1] * m_model->m_scaleH2Unit, bondwire.heights.at(next));
            auto & line = m_model->AddLineElement(std::move(p1), std::move(p2), bondwire.netId, bondwire.matId, bondwire.radius, bondwire.current);
            //connection
            if (0 == curr) {
                std::vector<utils::EStackupPrismaThermalModelQuery::RtVal> results;
                auto layer = lcm->GetLayerIndexByHeight(lcm->GetHeight(bondwire.heights.front()));
                m_query->SearchNearestPrismaInstances(layer, pts.at(curr), 1, results);
                ECAD_ASSERT(results.size() == 1)
                std::for_each(results.begin(), results.end(), [&](const auto & r) { line.neighbors.front().push_back(r.second); });
            }
            else {
                auto & prevLine = m_model->m_lines.at(m_model->m_lines.size() - 2);
                line.neighbors.front().emplace_back(prevLine.id);
                prevLine.neighbors.back().emplace_back(line.id);
            }
            if (next == pts.size() - 1) {
                std::vector<utils::EStackupPrismaThermalModelQuery::RtVal> results;
                auto layer = lcm->GetLayerIndexByHeight(lcm->GetHeight(bondwire.heights.back()));
                m_query->SearchNearestPrismaInstances(layer, pts.at(next), 1, results);
                ECAD_ASSERT(results.size() == 1)
                std::for_each(results.begin(), results.end(), [&](const auto & r) { line.neighbors.back().push_back(r.second); });
            }
        }
    }
}

ECAD_INLINE EFloat EStackupPrismaThermalModelBuilder::GetIntersectArea(const ETriangle2D & t1, const ETriangle2D & t2)
{
    EFloat area = 0;
    std::vector<EPolygonData> output;
    generic::geometry::boolean::Intersect(t1, t2, output);
    std::for_each(output.begin(), output.end(), [&area](const EPolygonData & p) { area += p.Area(); });
    return area;
}

} // namespace ecad::model::utils
