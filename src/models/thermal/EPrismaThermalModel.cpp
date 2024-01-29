#include "models/thermal/EPrismaThermalModel.h"
#include "models/thermal/utils/EPrismaThermalModelQuery.h"
#include "utils/ELayoutRetriever.h"
#include "Interface.h"

#include "generic/geometry/BoostGeometryRegister.hpp"
#include "generic/geometry/Triangulation.hpp"
#include "generic/geometry/GeometryIO.hpp"
#include <queue>
namespace ecad::model {

ECAD_INLINE EPrismaThermalModel::EPrismaThermalModel(CPtr<ILayoutView> layout)
 : m_layout(layout)
{
    m_blockBCs.emplace(EOrientation::Top, std::vector<BlockBC>{});
    m_blockBCs.emplace(EOrientation::Bot, std::vector<BlockBC>{});
}

CPtr<IMaterialDefCollection> EPrismaThermalModel::GetMaterialLibrary() const
{
    return m_layout->GetDatabase()->GetMaterialDefCollection();
}

ECAD_INLINE void EPrismaThermalModel::AddBlockBC(EOrientation orient, EBox2D block, EThermalBondaryCondition bc)
{
    m_blockBCs.at(orient).emplace_back(std::move(block), std::move(bc));
}

ECAD_INLINE EPrismaThermalModel::PrismaLayer & EPrismaThermalModel::AppendLayer(PrismaLayer layer)
{
    return layers.emplace_back(std::move(layer));
}

ECAD_INLINE EPrismaThermalModel::LineElement & EPrismaThermalModel::AddLineElement(FPoint3D start, FPoint3D end, ENetId netId, EMaterialId matId, EFloat radius, EFloat current)
{
    ECAD_ASSERT(TotalPrismaElements() > 0/*should add after build prisma model*/)
    LineElement & element = m_lines.emplace_back(LineElement{});
    element.id = TotalPrismaElements() + m_lines.size() - 1;
    element.endPoints.front() = AddPoint(std::move(start));
    element.endPoints.back() = AddPoint(std::move(end));
    element.current = current;
    element.radius = radius;
    element.matId = matId;
    element.netId = netId;
    return element;
}

ECAD_INLINE void EPrismaThermalModel::BuildPrismaModel(EFloat scaleH2Unit, EFloat scale2Meter)
{
    m_scaleH2Unit = scaleH2Unit;
    m_scale2Meter = scale2Meter;
    m_indexOffset = std::vector<size_t>{0};
    for (size_t i = 0; i < TotalLayers(); ++i)
        m_indexOffset.emplace_back(m_indexOffset.back() + layers.at(i).TotalElements());
    
    const auto & triangles = prismaTemplate.triangles;
    std::unordered_map<size_t, std::unordered_map<size_t, size_t> > templateIdMap;
    auto getPtIdxMap = [&templateIdMap](size_t lyrIdx) -> std::unordered_map<size_t, size_t> & {
        auto iter = templateIdMap.find(lyrIdx);
        if (iter == templateIdMap.cend())
            iter = templateIdMap.emplace(lyrIdx, std::unordered_map<size_t, size_t>{}).first;
        return iter->second;
    };
    
    auto total = TotalPrismaElements();
    m_prismas.resize(total);
    m_points.clear();
    for (size_t i = 0; i < total; ++i) {
        auto & instance = m_prismas[i];
        auto [lyrIdx, eleIdx] = PrismaLocalIndex(i);
        instance.layer = &layers.at(lyrIdx);
        instance.element = &instance.layer->elements.at(eleIdx);    
        //points
        auto & topPtIdxMap = getPtIdxMap(lyrIdx);
        auto & botPtIdxMap = getPtIdxMap(lyrIdx + 1);
        const auto & vertices = triangles.at(instance.element->templateId).vertices;
        for (size_t v = 0; v < vertices.size(); ++v) {
            auto topVtxIter = topPtIdxMap.find(vertices.at(v));
            if (topVtxIter == topPtIdxMap.cend()) {
                auto ptIdx = AddPoint(GetPoint(lyrIdx, eleIdx, v));
                topVtxIter = topPtIdxMap.emplace(vertices.at(v), ptIdx).first;
            }
            instance.vertices[v] = topVtxIter->second;
            auto botVtxIter = botPtIdxMap.find(vertices.at(v));
            if (botVtxIter == botPtIdxMap.cend()) {
                auto ptIdx = AddPoint(GetPoint(lyrIdx, eleIdx, v + 3));
                botVtxIter = botPtIdxMap.emplace(vertices.at(v), ptIdx).first;
            }
            instance.vertices[v + 3] = botVtxIter->second;
        }

        //neighbors
        for (size_t n = 0; n < 3; ++n) {
            if (auto nid = instance.element->neighbors.at(n); noNeighbor != nid) {
                auto nb = GlobalIndex(lyrIdx, instance.element->neighbors.at(n));
                instance.neighbors[n] = nb;
            }
        }
        ///top
        if (auto nid = instance.element->neighbors.at(PrismaElement::TOP_NEIGHBOR_INDEX); noNeighbor != nid) {
            auto nb = GlobalIndex(lyrIdx - 1, nid);
            instance.neighbors[PrismaElement::TOP_NEIGHBOR_INDEX] = nb;
        }
        ///bot
        if (auto nid = instance.element->neighbors.at(PrismaElement::BOT_NEIGHBOR_INDEX); noNeighbor != nid) {
            auto nb = GlobalIndex(lyrIdx + 1, nid);
            instance.neighbors[PrismaElement::BOT_NEIGHBOR_INDEX] = nb;
        }
    }
}

ECAD_INLINE void EPrismaThermalModel::AddBondWires(const std::vector<ELayerCutModel::Bondwire> & bondwires)
{
    utils::EPrismaThermalModelQuery query(this);
    for (const auto & bondwire : bondwires)
        AddBondWire(bondwire, &query);
}

ECAD_INLINE void EPrismaThermalModel::AddBondWire(const ELayerCutModel::Bondwire & bondwire, CPtr<utils::EPrismaThermalModelQuery> query)
{
    const auto & pts = bondwire.pt2ds;
    ECAD_ASSERT(pts.size() == bondwire.heights.size());
    for (size_t curr = 0; curr < pts.size() - 1; ++curr) {
        auto next = curr + 1;
        auto p1 = FPoint3D(pts.at(curr)[0] * m_scaleH2Unit, pts.at(curr)[1] * m_scaleH2Unit, bondwire.heights.at(curr));
        auto p2 = FPoint3D(pts.at(next)[0] * m_scaleH2Unit, pts.at(next)[1] * m_scaleH2Unit, bondwire.heights.at(next));
        auto & line = AddLineElement(std::move(p1), std::move(p2), bondwire.netId, bondwire.matId, bondwire.radius, bondwire.current);
        //connection
        if (0 == curr) {
            std::vector<utils::EPrismaThermalModelQuery::RtVal> results;
            query->SearchNearestPrismaInstances(bondwire.layer.front(), pts.at(curr), 1, results);
            ECAD_ASSERT(results.size() == 1)
            std::for_each(results.begin(), results.end(), [&](const auto & r) { line.neighbors.front().push_back(r.second); });
        }
        else {
            auto & prevLine = m_lines.at(m_lines.size() - 2);
            line.neighbors.front().emplace_back(prevLine.id);
            prevLine.neighbors.back().emplace_back(line.id);
        }
        if (next == pts.size() - 1) {
            std::vector<utils::EPrismaThermalModelQuery::RtVal> results;
            query->SearchNearestPrismaInstances(bondwire.layer.back(), pts.at(next), 1, results);
            ECAD_ASSERT(results.size() == 1)
            std::for_each(results.begin(), results.end(), [&](const auto & r) { line.neighbors.back().push_back(r.second); });
        }
    }
}

size_t EPrismaThermalModel::AddPoint(FPoint3D point)
{
    m_points.emplace_back(std::move(point));
    return m_points.size() - 1;
}

FPoint3D EPrismaThermalModel::GetPoint(size_t lyrIndex, size_t eleIndex, size_t vtxIndex) const
{
    const auto & points = prismaTemplate.points;
    const auto & triangles = prismaTemplate.triangles;
    const auto & element = layers.at(lyrIndex).elements.at(eleIndex);
    const auto & triangle = triangles.at(element.templateId);
    EFloat height = vtxIndex < 3 ? layers.at(lyrIndex).elevation :
            isBotLayer(lyrIndex) ? layers.at(lyrIndex).elevation - layers.at(lyrIndex).thickness :
                                   layers.at(lyrIndex + 1).elevation;
    vtxIndex = vtxIndex % 3;
    const auto & pt2d = points.at(triangle.vertices.at(vtxIndex));
    return FPoint3D{pt2d[0] * m_scaleH2Unit, pt2d[1] * m_scaleH2Unit, height};
}

} //namespace ecad::model