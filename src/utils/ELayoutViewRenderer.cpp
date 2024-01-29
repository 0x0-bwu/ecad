#include "ELayoutViewRenderer.h"

#include "generic/tools/Color.hpp"

#include "EDataMgr.h"
#include "EShape.h"

namespace ecad {
namespace utils {
using namespace generic::geometry;
ECAD_INLINE ELayoutViewRenderer::ELayoutViewRenderer(const ELayoutViewRendererSettings & settings)
 : m_settings(settings)
{
}

ECAD_INLINE bool ELayoutViewRenderer::Renderer(CPtr<ILayoutView> layout)
{
    switch (m_settings.format) {
        case ELayoutViewRendererSettings::Format::PNG :
            return RendererPNG(layout);
        default :
            return false;
    }
}

ECAD_INLINE bool ELayoutViewRenderer::RendererPNG(CPtr<ILayoutView> layout)
{   
    using namespace generic::geometry;
    
    std::vector<EPolygonData> outs;

    //boundary
    auto boundary = layout->GetBoundary()->GetContour();
    outs.emplace_back(std::move(boundary));
    
    //components
    auto compIter = layout->GetComponentIter();
    while (auto * comp = compIter->Next()) {
        auto bbox = comp->GetBoundingBox();
        if (not m_settings.selectLayers.empty()) {
            auto layer = comp->GetPlacementLayer();
            if (m_settings.selectLayers.count(layer))
                continue;
        }
        outs.emplace_back(toPolygon(bbox));
    }

    //primitives
    auto primIter = layout->GetPrimitiveIter();
    while (auto * prim = primIter->Next()) {
        if (not m_settings.selectNets.empty()) {
            auto net = prim->GetNet();
            if (m_settings.selectNets.count(net))
                continue;
        }
        if (not m_settings.selectLayers.empty()) {
            auto layer = prim->GetLayer();
            if (m_settings.selectLayers.count(layer))
                continue;
        }
        if (auto * bw = prim->GetBondwireFromPrimitive(); bw) {
            EPolygonData pd;
            pd << bw->GetStartPt() << bw->GetEndPt();
            outs.emplace_back(std::move(pd));
        }
        else if (auto * geom = prim->GetGeometry2DFromPrimitive(); geom) {
            auto pwh = geom->GetShape()->GetPolygonWithHoles();
            outs.emplace_back(std::move(pwh.outline));
            for (auto hole : pwh.holes)
                outs.emplace_back(std::move(hole));
        }
    }

    //todo flatten cell inst

    auto cellName = layout->GetCell()->GetName();
    auto filename = m_settings.dirName  + ECAD_SEPS + cellName + ".png";
    return GeometryIO::WritePNG(filename.c_str(), outs.begin(), outs.end(), m_settings.width);
}

}//namespace utils
}//namespace ecad