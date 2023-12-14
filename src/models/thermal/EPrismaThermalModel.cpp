#include "models/thermal/EPrismaThermalModel.h"
#include "interfaces/IPrimitiveCollection.h"
#include "interfaces/IPadstackInst.h"
#include "interfaces/IPadstackDef.h"
#include "interfaces/IMaterialDef.h"
#include "interfaces/ILayoutView.h"
#include "interfaces/IPrimitive.h"
#include "interfaces/ILayer.h"

#include "generic/geometry/BoostGeometryRegister.hpp"
#include "generic/geometry/Triangulation.hpp"
#include "generic/geometry/GeometryIO.hpp"

namespace ecad::emodel::etherm {

ECAD_INLINE void ECompactLayout::AddShape(ENetId netId, ELayerId layerId, EMaterialId matId, CPtr<EShape> shape)
{
    auto addPolygon = [&](ENetId netId, ELayerId layerId, EMaterialId matId, EPolygonData polygon, bool isHole)
    {
        if (not polygon.isCCW()) polygon.Reverse();
        polygons.emplace_back(std::move(polygon));
        materials.emplace_back(matId);
        layers.emplace_back(layerId);
        nets.emplace_back(netId);
    };

    if (shape->hasHole()) {
        auto pwh = shape->GetPolygonWithHoles();
        addPolygon(netId, layerId, matId, std::move(pwh.outline), false);
        for (auto iter = pwh.ConstBeginHoles(); iter != pwh.ConstEndHoles(); ++iter)
            addPolygon(netId, layerId, matId, std::move(*iter), true);//todo matId
    }
    else addPolygon(netId, layerId, matId, shape->GetContour(), false);
}

ECAD_INLINE bool ECompactLayout::WriteImgView(std::string_view filename, size_t width) const
{
    return generic::geometry::GeometryIO::WritePNG(filename, polygons.begin(), polygons.end(), width);
}

ECAD_INLINE void ECompactLayout::BuildLayerPolygonLUT()
{
    m_rtrees.clear();
    m_lyrPolygons.clear();
    for (size_t i = 1; i < polygons.size(); ++i) {
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
    compact.AddShape(ENetId::noNet, ELayerId::noLayer, static_cast<EMaterialId>(0), layout->GetBoundary());

    auto primitives = layout->GetPrimitiveCollection();
    for (size_t i = 0; i < primitives->Size(); ++i) {
        auto prim = primitives->GetPrimitive(i);
        auto geom = prim->GetGeometry2DFromPrimitive();
        if (nullptr == geom) continue;

        auto shape = geom->GetShape();
        auto netId = prim->GetNet();
        auto lyrId = prim->GetLayer();
        compact.AddShape(netId, lyrId,static_cast<EMaterialId>(0), shape);//todo, material
    }
    auto psInstIter = layout->GetPadstackInstIter();
    while (auto psInst = psInstIter->Next()){
        auto netId = psInst->GetNet();
        auto defData = psInst->GetPadstackDef()->GetPadstackDefData();
        if (nullptr == defData) continue;

        ELayerId top, bot;
        psInst->GetLayerRange(top, bot);
        for (int lyrId = std::min(top, bot); lyrId <= std::max(top, bot); lyrId++) {
            auto layerId = static_cast<ELayerId>(lyrId);
            auto shape = psInst->GetLayerShape(static_cast<ELayerId>(lyrId));
            compact.AddShape(netId, layerId, static_cast<EMaterialId>(0), shape.get());//todo, material
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