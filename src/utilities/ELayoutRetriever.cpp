#include "utilities/ELayoutRetriever.h"
#include "Interface.h"
namespace ecad {
namespace eutils {

ECAD_INLINE ELayoutRetriever::ELayoutRetriever(CPtr<ILayoutView> layout)
 : m_layout(layout)
{
}

ECAD_INLINE bool ELayoutRetriever::GetLayerStackupHeightThickness(FCoord & elevation, FCoord & thickness) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    std::vector<CPtr<IStackupLayer> > stackupLayers;
    m_layout->GetStackupLayers(stackupLayers);
    FCoord topElevation, topThickness, botElevation, botThickness;
    GetLayerHeightThickness(stackupLayers.front()->GetLayerId(), topElevation, topThickness);
    GetLayerHeightThickness(stackupLayers.back()->GetLayerId(), botElevation, botThickness);
    elevation = topElevation;
    thickness = topElevation - botElevation + botThickness;
    return true;
}

ECAD_INLINE bool ELayoutRetriever::GetLayerHeightThickness(ELayerId layerId, FCoord & elevation, FCoord & thickness) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    auto iter = m_lyrHeightsMap.find(layerId);
    if (iter == m_lyrHeightsMap.cend()) return false;
    elevation = iter->second.first;
    thickness = iter->second.second;
    return true;
}

ECAD_INLINE bool ELayoutRetriever::GetLayerHeightThickness(CPtr<ILayer> layer, FCoord & elevation, FCoord & thickness) const
{
    return GetLayerHeightThickness(layer->GetLayerId(), elevation, thickness);
}

ECAD_INLINE bool ELayoutRetriever::GetComponentHeightThickness(CPtr<IComponent> component, FCoord & elevation, FCoord & thickness) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    auto iter = m_lyrHeightsMap.find(component->GetPlacementLayer());
    if (iter == m_lyrHeightsMap.cend()) return false;
    if (component->isFlipped()) {
        thickness = component->GetHeight();
        elevation = iter->second.first - iter->second.second;
    }
    else {
        thickness = component->GetHeight();
        elevation = iter->second.first + thickness;
    }
    return true;
}

ECAD_INLINE bool ELayoutRetriever::GetBondwireHeight(CPtr<IBondwire> bondwire, FCoord & start, FCoord & end) const
{
    FCoord elevation, thickness;
    auto getHeight = [&](bool start, FCoord & height) {
        auto layer = start ? bondwire->GetStartLayer() : bondwire->GetEndLayer();
        if (ELayerId::ComponentLayer == layer) {
            auto comp = start ? bondwire->GetStartComponent() : bondwire->GetEndComponent(); { ECAD_ASSERT(comp) }
            if (not GetComponentHeightThickness(comp, elevation, thickness)) return false;
            height = comp->isFlipped() ? elevation - thickness : elevation;
            return true;
        }
        else if (ELayerId::noLayer != layer) {
            if (not GetLayerHeightThickness(layer->GetLayerId(), elevation, thickness)) return false;
            height = elevation;
            return true;
        }
        return false;
    };
    if (not getHeight(true, start)) return false;
    if (not getHeight(false, end)) return false;
    return true;
}

ECAD_INLINE bool ELayoutRetriever::GetBondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<FCoord> & heights) const
{
    //todo, based on bondwire profile
    
    //JEDEC4
    pt2ds.resize(4);
    heights.resize(4);
    pt2ds[0] = bondwire->GetStartPt();
    pt2ds[1] = pt2ds.front();
    pt2ds[3] = bondwire->GetEndPt();
    auto vet = pt2ds.at(3) - pt2ds.at(0);
    pt2ds[2] = pt2ds.at(0) + 0.125 * vec.Norm2();

    if (not GetBondwireHeight(bondwire, heighs.front(), heighs.back())) return false;
    heights[2] = heights[1] = heights[0] + bondwire->GetHeight();
    return true;
}

ECAD_INLINE CPtr<IStackupLayer> ELayoutRetriever::SearchStackupLayer(FCoord height) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    std::vector<CPtr<IStackupLayer> > stackupLayers;
    m_layout->GetStackupLayers(stackupLayers);

    using namespace generic::math;
    FCoord elevation, thickness;
    for (auto stackupLayer : stackupLayers) {
        GetLayerHeightThickness(stackupLayer->GetLayerId(), elevation, thickness);
        if (Within<LORC, FCoord>(height, elevation - thickness, elevation))
            return stackupLayer;
    }
    return nullptr;
}

ECAD_INLINE void ELayoutRetriever::BuildLayerHeightsMap() const
{
    m_lyrHeightsMap.clear();
    std::vector<CPtr<IStackupLayer> > stackupLayers;
    m_layout->GetStackupLayers(stackupLayers);
    for (size_t i = 0; i < stackupLayers.size(); ++i) {
        auto lyrId = stackupLayers.at(i)->GetLayerId();
        auto elevation = stackupLayers.at(i)->GetElevation();
        auto thickness = stackupLayers.at(i)->GetThickness();
        m_lyrHeightsMap.emplace(lyrId, std::make_pair(elevation, thickness));
    }
    //todo via layer
}

}//namespace eutils
}//namespace ecad
