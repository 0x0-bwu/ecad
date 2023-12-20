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
