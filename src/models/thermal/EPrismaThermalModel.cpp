#include "models/thermal/EPrismaThermalModel.h"
#include "utilities/ELayoutRetriever.h"
#include "Interface.h"

#include "generic/geometry/BoostGeometryRegister.hpp"
#include "generic/geometry/Triangulation.hpp"
#include "generic/geometry/GeometryIO.hpp"
#include <queue>
namespace ecad::emodel::etherm {

ECAD_INLINE ECompactLayout::PowerBlock::PowerBlock(size_t polygon, Height position, LayerRange range, ESimVal powerDensity)
 : polygon(polygon), position(position), range(std::move(range)), powerDensity(powerDensity)
{
}

ECAD_INLINE ECompactLayout::ECompactLayout(EValue vScale2Int) : m_vScale2Int(vScale2Int) {}

ECAD_INLINE void ECompactLayout::AddShape(ENetId netId, EMaterialId solidMat, EMaterialId holeMat, CPtr<EShape> shape, FCoord elevation, FCoord thickness)
{
    if (shape->hasHole()) {
        auto pwh = shape->GetPolygonWithHoles();
        AddPolygon(netId, solidMat, std::move(pwh.outline), false, elevation, thickness);
        for (auto iter = pwh.ConstBeginHoles(); iter != pwh.ConstEndHoles(); ++iter)
            AddPolygon(netId, holeMat, std::move(*iter), true, elevation, thickness);
    }
    else AddPolygon(netId, solidMat, shape->GetContour(), false, elevation, thickness);
}

ECAD_INLINE size_t ECompactLayout::AddPolygon(ENetId netId, EMaterialId matId, EPolygonData polygon, bool isHole, FCoord elevation, FCoord thickness)
{
    if (isHole == polygon.isCCW()) polygon.Reverse();
    ranges.emplace_back(GetLayerRange(elevation, thickness));
    polygons.emplace_back(std::move(polygon));
    materials.emplace_back(matId);
    nets.emplace_back(netId);
    return polygons.size() - 1;
};

ECAD_INLINE void ECompactLayout::AddPowerBlock(EMaterialId matId, EPolygonData polygon, ESimVal totalP, FCoord elevation, FCoord thickness, EValue position)
{
    auto area = polygon.Area();
    auto index = AddPolygon(ENetId::noNet, matId, std::move(polygon), false, elevation, thickness);
    Height height = (elevation - thickness * position) * m_vScale2Int;
    powerBlocks.emplace(index, PowerBlock(index, height, GetLayerRange(elevation, thickness), totalP / area));
}

ECAD_INLINE bool ECompactLayout::WriteImgView(std::string_view filename, size_t width) const
{
    return generic::geometry::GeometryIO::WritePNG(filename, polygons.begin(), polygons.end(), width);
}

ECAD_INLINE void ECompactLayout::BuildLayerPolygonLUT()
{
    m_layerOrder.clear();
    m_height2Index.clear();
    std::set<Height> heights;
    for (size_t i = 0; i < ranges.size(); ++i) {
        if (EMaterialId::noMaterial == materials.at(i)) continue;
        heights.emplace(ranges.at(i).first);
        heights.emplace(ranges.at(i).second);
        auto iter = powerBlocks.find(i);
        if (iter != powerBlocks.cend())
            heights.emplace(iter->second.position);
    }
    m_layerOrder = std::vector(heights.begin(), heights.end());
    std::reverse(m_layerOrder.begin(), m_layerOrder.end());
    for (size_t i = 0; i < m_layerOrder.size(); ++i)
        m_height2Index.emplace(m_layerOrder.at(i), i);

    m_rtrees.clear();
    m_lyrPolygons.clear();
    for (size_t i = 0; i < polygons.size(); ++i) {
        if (EMaterialId::noMaterial == materials.at(i)) continue;

        const auto & range = ranges.at(i);
        size_t sLayer = m_height2Index.at(range.first);
        size_t eLayer = std::min(TotalLayers() - 1, m_height2Index.at(range.second));
        for (size_t layer = sLayer; layer <= eLayer; ++layer) {
            auto iter = m_lyrPolygons.find(layer);
            if (iter == m_lyrPolygons.cend())
                iter = m_lyrPolygons.emplace(layer, std::vector<size_t>{}).first;
            iter->second.emplace_back(i);
        }
    }

    for (size_t layer = 0; layer < TotalLayers(); ++layer) {
        auto & rtree = m_rtrees.emplace(layer, std::make_shared<Rtree>()).first->second;
        for (auto i : m_lyrPolygons.at(layer)) {
            auto bbox = generic::geometry::Extent(polygons.at(i));
            rtree->insert(std::make_pair(bbox, i));
        }
    }
}

ECAD_INLINE size_t ECompactLayout::TotalLayers() const
{
    return m_layerOrder.size() - 1;
}

ECAD_INLINE bool ECompactLayout::hasPolygon(size_t layer) const
{
    return m_lyrPolygons.count(layer);
}

ECAD_INLINE size_t ECompactLayout::SearchPolygon(size_t layer, const EPoint2D & pt) const
{
    if (not hasPolygon(layer)) return invalidIndex;
    std::vector<RtVal> results;
    m_rtrees.at(layer)->query(boost::geometry::index::intersects(EBox2D(pt, pt)), std::back_inserter(results));
    if (results.empty()) return invalidIndex;
    auto cmp = [&](auto i1, auto i2){ return polygons.at(i1).Area() > polygons.at(i2).Area(); };
    std::priority_queue<size_t, std::vector<size_t>, decltype(cmp)> pq(cmp);
    for (const auto & result : results) {
        if (generic::geometry::Contains(polygons.at(result.second), pt))
            pq.emplace(result.second);
    }
    if (not pq.empty()) return pq.top();
    return invalidIndex;
}

ECAD_INLINE bool ECompactLayout::GetLayerHeightThickness(size_t layer, FCoord & elevation, FCoord & thickness) const
{
    if (layer >= TotalLayers()) return false;
    elevation = FCoord(m_layerOrder.at(layer)) / m_vScale2Int;
    thickness = elevation - FCoord(m_layerOrder.at(layer + 1)) / m_vScale2Int;
    return true;
}

ECAD_INLINE const EPolygonData & ECompactLayout::GetLayoutBoundary() const
{
    return polygons.front();
}

ECAD_INLINE ECompactLayout::LayerRange ECompactLayout::GetLayerRange(FCoord elevation, FCoord thickness) const
{
    return LayerRange{elevation * m_vScale2Int, (elevation - thickness) * m_vScale2Int};
}

ECAD_INLINE UPtr<ECompactLayout> makeCompactLayout(CPtr<ILayoutView> layout)
{
    ECompactLayout compact(1e6);//todo, scale
    //todo, reserve size   

    eutils::ELayoutRetriever retriever(layout);
    [[maybe_unused]] bool check;
    FCoord elevation, thickness;
    check = retriever.GetLayerStackupHeightThickness(elevation, thickness);
    ECAD_ASSERT(check)
    std::vector<CPtr<IStackupLayer> > stackupLayers;
    layout->GetStackupLayers(stackupLayers);
    for (auto stackupLayer : stackupLayers) {
        auto dielMat = layout->GetDatabase()->FindMaterialDefByName(stackupLayer->GetDielectricMaterial()); { ECAD_ASSERT(dielMat) }
        check = retriever.GetLayerHeightThickness(stackupLayer->GetLayerId(), elevation, thickness); { ECAD_ASSERT(check) } 
        compact.AddShape(ENetId::noNet, dielMat->GetMaterialId(), EMaterialId::noMaterial, layout->GetBoundary(), elevation, thickness);
    }

    auto compIter = layout->GetComponentIter();
    while (auto * comp = compIter->Next()) {
        auto totalP = comp->GetLossPower();
        if (math::LE<ESimVal>(totalP, 0)) continue;
        auto material = layout->GetDatabase()->FindMaterialDefByName(comp->GetComponentDef()->GetMaterial());
        ECAD_ASSERT(material)
        check = retriever.GetComponentHeightThickness(comp, elevation, thickness);
        ECAD_ASSERT(check)
        compact.AddPowerBlock(material->GetMaterialId(), toPolygon(comp->GetBoundingBox()), totalP, elevation, thickness);    
    }

    std::unordered_map<ELayerId, std::pair<FCoord, FCoord> > layerElevationThicknessMap;
    std::unordered_map<ELayerId, std::pair<EMaterialId, EMaterialId> > layerMaterialMap;
    auto primitives = layout->GetPrimitiveCollection();
    for (size_t i = 0; i < primitives->Size(); ++i) {
        auto prim = primitives->GetPrimitive(i);
        auto geom = prim->GetGeometry2DFromPrimitive();
        if (nullptr == geom) continue;

        auto shape = geom->GetShape();
        auto netId = prim->GetNet();
        auto lyrId = prim->GetLayer();
        auto matIt = layerMaterialMap.find(lyrId);
        if (matIt == layerMaterialMap.cend()) {
            auto layer = layout->GetLayerCollection()->FindLayerByLayerId(lyrId);
            if (nullptr == layer) { ECAD_ASSERT(false); continue; }
            auto stackupLayer = layer->GetStackupLayerFromLayer();
            ECAD_ASSERT(nullptr != stackupLayer);
            auto condMat = layout->GetDatabase()->FindMaterialDefByName(stackupLayer->GetConductingMaterial());
            auto dielMat = layout->GetDatabase()->FindMaterialDefByName(stackupLayer->GetDielectricMaterial());
            ECAD_ASSERT(condMat && dielMat);
            matIt = layerMaterialMap.emplace(lyrId, std::make_pair(condMat->GetMaterialId(), dielMat->GetMaterialId())).first;
        }
        const auto & [condMatId, dielMatId] = matIt->second;
        auto iter = layerElevationThicknessMap.find(lyrId);
        if (iter == layerElevationThicknessMap.cend()) {
            check = retriever.GetLayerHeightThickness(lyrId, elevation, thickness);
            ECAD_ASSERT(check)
            iter = layerElevationThicknessMap.emplace(lyrId, std::make_pair(elevation, thickness)).first;
        }
        std::tie(elevation, thickness) = iter->second;
        compact.AddShape(netId, condMatId, dielMatId, shape, elevation, thickness);
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
        for (int i = std::min(top, bot); i <= std::max(top, bot); ++i) {
            auto lyrId = static_cast<ELayerId>(i);
            auto shape = psInst->GetLayerShape(static_cast<ELayerId>(lyrId));
            auto iter = layerElevationThicknessMap.find(lyrId);
            if (iter == layerElevationThicknessMap.cend()) {
                check = retriever.GetLayerHeightThickness(lyrId, elevation, thickness);
                ECAD_ASSERT(check)
                iter = layerElevationThicknessMap.emplace(lyrId, std::make_pair(elevation, thickness)).first;
            }
            std::tie(elevation, thickness) = iter->second;
            compact.AddShape(netId, material->GetMaterialId(), material->GetMaterialId(), shape.get(), elevation, thickness);
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