#include "models/thermal/EPrismaThermalModel.h"
#include "Interface.h"

#include "generic/geometry/BoostGeometryRegister.hpp"
#include "generic/geometry/Triangulation.hpp"
#include "generic/geometry/GeometryIO.hpp"

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

ECAD_INLINE void ECompactLayout::AddPolygon(ENetId netId, ELayerId layerId, EMaterialId matId, EPolygonData polygon, bool isHole)
{
    if (isHole == polygon.isCCW()) polygon.Reverse();
    polygons.emplace_back(std::move(polygon));
    materials.emplace_back(matId);
    layers.emplace_back(layerId);
    nets.emplace_back(netId);
};

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
    for (const auto & result : results)
        if (generic::geometry::Contains(polygons.at(result.second), pt))
            return result.second;
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
        if (comp->GetLossPower() > 0)
            compact.AddPolygon(ENetId::noNet, ELayerId::noLayer, EMaterialId::noMaterial, toPolygon(comp->GetBoundingBox()), false);
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

} //namespace ecad::emodel::etherm