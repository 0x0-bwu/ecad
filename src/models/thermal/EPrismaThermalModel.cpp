#include "models/thermal/EPrismaThermalModel.h"
#include "interfaces/IPrimitiveCollection.h"
#include "interfaces/IPadstackInst.h"
#include "interfaces/IPadstackDef.h"
#include "interfaces/IMaterialDef.h"
#include "interfaces/ILayoutView.h"
#include "interfaces/IPrimitive.h"
#include "interfaces/ILayer.h"

#include "generic/geometry/GeometryIO.hpp"
namespace ecad::emodel::etherm {

ECAD_INLINE void ECompactLayout::AddShape(ENetId netId, ELayerId layerId, CPtr<EShape> shape)
{
    auto addPolygon = [&](ENetId netId, ELayerId layerId, EPolygonData polygon, bool isHole)
    {
        if (not polygon.isCCW()) polygon.Reverse();
        polygons.emplace_back(std::move(polygon));
        layers.emplace_back(layerId);
        nets.emplace_back(netId);
    };

    if (shape->hasHole()) {
        auto pwh = shape->GetPolygonWithHoles();
        addPolygon(netId, layerId, std::move(pwh.outline), false);
        for (auto iter = pwh.ConstBeginHoles(); iter != pwh.ConstEndHoles(); ++iter)
            addPolygon(netId, layerId, std::move(*iter), true);
    }
    else addPolygon(netId, layerId, shape->GetContour(), false);
}

ECAD_INLINE bool ECompactLayout::WriteImgView(std::string_view filename, size_t width) const
{
    return generic::geometry::GeometryIO::WritePNG<EPolygonData>(filename, polygons.begin(), polygons.end(), width);
}

ECAD_INLINE UPtr<ECompactLayout> makeCompactLayout(CPtr<ILayoutView> layout)
{
    ECompactLayout compact;
    //todo, reserve size   

    compact.AddShape(ENetId::noNet, ELayerId::noLayer, layout->GetBoundary());

    auto primitives = layout->GetPrimitiveCollection();
    for (size_t i = 0; i < primitives->Size(); ++i) {
        auto prim = primitives->GetPrimitive(i);
        auto geom = prim->GetGeometry2DFromPrimitive();
        if (nullptr == geom) continue;

        auto shape = geom->GetShape();
        auto netId = prim->GetNet();
        auto lyrId = prim->GetLayer();
        compact.AddShape(netId, lyrId, shape);
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
            compact.AddShape(netId, layerId, shape.get());
        }
    }
    return std::make_unique<ECompactLayout>(compact);
}
} //namespace ecad::emodel::etherm