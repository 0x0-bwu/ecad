#include "models/thermal/EPrismaThermalModel.h"
#include "Interface.h"

#include "generic/geometry/BoostGeometryRegister.hpp"
#include "generic/geometry/Triangulation.hpp"
#include "generic/geometry/GeometryIO.hpp"
#include <queue>
namespace ecad::emodel::etherm {

ECAD_INLINE void ECompactLayout::AddShape(ENetId netId, ELayerId layerId, EMaterialId solidMat, EMaterialId holeMat, CPtr<EShape> shape)
{
    if (shape->hasHole()) {
        auto pwh = shape->GetPolygonWithHoles();
        AddPolygon(netId, layerId, solidMat, std::move(pwh.outline), false);
        for (auto iter = pwh.ConstBeginHoles(); iter != pwh.ConstEndHoles(); ++iter)
            AddPolygon(netId, layerId, holeMat, std::move(*iter), true);//todo matId
    }
    else AddPolygon(netId, layerId, solidMat, shape->GetContour(), false);
}

ECAD_INLINE size_t ECompactLayout::AddPolygon(ENetId netId, ELayerId layerId, EMaterialId matId, EPolygonData polygon, bool isHole)
{
    if (isHole == polygon.isCCW()) polygon.Reverse();
    polygons.emplace_back(std::move(polygon));
    materials.emplace_back(matId);
    layers.emplace_back(layerId);
    nets.emplace_back(netId);
    return polygons.size() - 1;
};

ECAD_INLINE void ECompactLayout::AddPowerBlock(ELayerId layerId, EMaterialId matId, EPolygonData polygon, ESimVal totalP)
{
    auto area = polygon.Area();
    auto index = AddPolygon(ENetId::noNet, layerId, matId, std::move(polygon), false);
    powerBlocks.emplace(index, totalP / area);
}

ECAD_INLINE bool ECompactLayout::WriteImgView(std::string_view filename, size_t width) const
{
    return generic::geometry::GeometryIO::WritePNG(filename, polygons.begin(), polygons.end(), width);
}

ECAD_INLINE void ECompactLayout::BuildLayerPolygonLUT()
{
    m_rtrees.clear();
    m_lyrPolygons.clear();
    for (size_t i = 0; i < polygons.size(); ++i) {
        if (EMaterialId::noMaterial == materials.at(i)) continue;

        auto iter = m_lyrPolygons.find(layers.at(i));
        if (iter == m_lyrPolygons.cend())
            iter = m_lyrPolygons.emplace(layers.at(i), std::vector<size_t>{}).first;
        iter->second.emplace_back(i);
    }

    for (const auto & [layer, pIndices] : m_lyrPolygons) {
        auto & rtree = m_rtrees.emplace(layer, std::make_shared<Rtree>()).first->second;
        for (auto i : pIndices) {
            auto bbox = generic::geometry::Extent(polygons.at(i));
            rtree->insert(std::make_pair(bbox, i));
        }
    }
}

ECAD_INLINE bool ECompactLayout::hasPolygon(ELayerId layerId) const
{
    return m_lyrPolygons.count(layerId);
}

ECAD_INLINE const std::vector<size_t> & ECompactLayout::GetLayerPolygons(ELayerId layerId) const 
{
    ECAD_ASSERT(hasPolygon(layerId));
    return m_lyrPolygons.at(layerId);
}

ECAD_INLINE size_t ECompactLayout::SearchPolygon(ELayerId layerId, const EPoint2D & pt) const
{
    if (not hasPolygon(layerId)) return invalidIndex;
    std::vector<RtVal> results;
    m_rtrees.at(layerId)->query(boost::geometry::index::intersects(EBox2D(pt, pt)), std::back_inserter(results));
    auto cmp = [&](auto i1, auto i2){ return polygons.at(i1).Area() > polygons.at(i2).Area(); };
    std::priority_queue<size_t, std::vector<size_t>, decltype(cmp)> pq(cmp);
    for (const auto & result : results) {
        if (generic::geometry::Contains(polygons.at(result.second), pt))
            pq.emplace(result.second);
    }
    if (not pq.empty()) return pq.top();
    return invalidIndex;
}

ECAD_INLINE const EPolygonData & ECompactLayout::GetLayoutBoundary() const
{
    return polygons.front();
}

ECAD_INLINE UPtr<ECompactLayout> makeCompactLayout(CPtr<ILayoutView> layout)
{
    ECompactLayout compact;
    //todo, reserve size   
    compact.AddShape(ENetId::noNet, ELayerId::noLayer, EMaterialId::noMaterial,  EMaterialId::noMaterial, layout->GetBoundary());

    auto compIter = layout->GetComponentIter();
    while (auto * comp = compIter->Next()) {
        auto totalP = comp->GetLossPower();
        auto material = layout->GetDatabase()->FindMaterialDefByName(comp->GetComponentDef()->GetMaterial());
        compact.AddPowerBlock(comp->GetPlacementLayer(), material->GetMaterialId(), toPolygon(comp->GetBoundingBox()), totalP);    
    }

    std::unordered_map<ELayerId, std::pair<EMaterialId, EMaterialId> > layerMaterialMap;
    auto primitives = layout->GetPrimitiveCollection();
    for (size_t i = 0; i < primitives->Size(); ++i) {
        auto prim = primitives->GetPrimitive(i);
        auto geom = prim->GetGeometry2DFromPrimitive();
        if (nullptr == geom) continue;

        auto shape = geom->GetShape();
        auto netId = prim->GetNet();
        auto lyrId = prim->GetLayer();
        auto iter = layerMaterialMap.find(lyrId);
        if (iter == layerMaterialMap.cend()) {
            auto layer = layout->GetLayerCollection()->FindLayerByLayerId(lyrId);
            if (nullptr == layer) { ECAD_ASSERT(false); continue; }
            auto stackupLayer = layer->GetStackupLayerFromLayer();
            ECAD_ASSERT(nullptr != stackupLayer);
            auto condMat = layout->GetDatabase()->FindMaterialDefByName(stackupLayer->GetConductingMaterial());
            auto dielMat = layout->GetDatabase()->FindMaterialDefByName(stackupLayer->GetDielectricMaterial());
            ECAD_ASSERT(condMat && dielMat);
            iter = layerMaterialMap.emplace(lyrId, std::make_pair(condMat->GetMaterialId(), dielMat->GetMaterialId())).first;
        }
        const auto & [condMatId, dielMatId] = iter->second;
        compact.AddShape(netId, lyrId, condMatId, dielMatId, shape);
    }
    auto psInstIter = layout->GetPadstackInstIter();
    while (auto psInst = psInstIter->Next()){
        auto netId = psInst->GetNet();
        auto defData = psInst->GetPadstackDef()->GetPadstackDefData();
        if (nullptr == defData) continue;

        auto material = layout->GetDatabase()->FindMaterialDefByName(defData->GetMaterial());
        ECAD_ASSERT(nullptr != material);

        ELayerId top, bot;
        psInst->GetLayerRange(top, bot);
        for (int lyrId = std::min(top, bot); lyrId <= std::max(top, bot); lyrId++) {
            auto layerId = static_cast<ELayerId>(lyrId);
            auto shape = psInst->GetLayerShape(static_cast<ELayerId>(lyrId));
            compact.AddShape(netId, layerId, material->GetMaterialId(), material->GetMaterialId(), shape.get());
        }
    }
    compact.BuildLayerPolygonLUT();
    return std::make_unique<ECompactLayout>(compact);
}

ECAD_INLINE EPrismaThermalModel::PrismaLayer & EPrismaThermalModel::AppendLayer(PrismaLayer layer)
{
    return layers.emplace_back(std::move(layer));
}

void EPrismaThermalModel::Build(EValue scaleH2Unit, EValue scale2Meter)
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
    
    auto total = TotalElements();
    m_prismas.resize(total);
    m_points.clear();
    for (size_t i = 0; i < total; ++i) {
        auto & instance = m_prismas[i];
        auto [lyrIdx, eleIdx] = LocalIndex(i);
        instance.layer = &layers.at(lyrIdx);
        instance.element = &instance.layer->elements.at(eleIdx);    
        //points
        auto & topPtIdxMap = getPtIdxMap(lyrIdx);
        auto & botPtIdxMap = getPtIdxMap(lyrIdx + 1);
        const auto & vertices = triangles.at(instance.element->templateId).vertices;
        for (size_t v = 0; v < vertices.size(); ++v) {
            auto topVtxIter = topPtIdxMap.find(vertices.at(v));
            if (topVtxIter == topPtIdxMap.cend()) {
                m_points.emplace_back(GetPoint(lyrIdx, eleIdx, v));
                topVtxIter = topPtIdxMap.emplace(vertices.at(v), m_points.size() - 1).first;
            }
            instance.vertices[v] = topVtxIter->second;
            auto botVtxIter = botPtIdxMap.find(vertices.at(v));
            if (botVtxIter == botPtIdxMap.cend()) {
                m_points.emplace_back(GetPoint(lyrIdx, eleIdx, v + 3));
                botVtxIter = botPtIdxMap.emplace(vertices.at(v), m_points.size() - 1).first;
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

FPoint3D EPrismaThermalModel::GetPoint(size_t lyrIndex, size_t eleIndex, size_t vtxIndex) const
{
    const auto & points = prismaTemplate.points;
    const auto & triangles = prismaTemplate.triangles;
    const auto & element = layers.at(lyrIndex).elements.at(eleIndex);
    const auto & triangle = triangles.at(element.templateId);
    FCoord height = vtxIndex < 3 ? layers.at(lyrIndex).elevation :
            isBotLayer(lyrIndex) ? layers.at(lyrIndex).elevation - layers.at(lyrIndex).thickness :
                                   layers.at(lyrIndex + 1).elevation;
    vtxIndex = vtxIndex % 3;
    const auto & pt2d = points.at(triangle.vertices.at(vtxIndex));
    return FPoint3D{pt2d[0] * m_scaleH2Unit, pt2d[1] * m_scaleH2Unit, height};
}

} //namespace ecad::emodel::etherm