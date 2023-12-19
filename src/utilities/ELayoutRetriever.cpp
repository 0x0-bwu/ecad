#include "utilities/ELayoutRetriever.h"
#include "Interface.h"
namespace ecad {
namespace eutils {

ECAD_INLINE ELayoutRetriever::ELayoutRetriever(CPtr<ILayoutView> layout)
 : m_layout(layout)
{
}

ECAD_INLINE bool ELayoutRetriever::GetLayerHeight(ELayerId layerId, FCoord & high, FCoord & low) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    auto iter = m_lyrHeightsMap.find(layer->GetLayerId());
    if (iter == m_lyrHeightsMap.cend()) return false;
    high = iter->second.first;
    low = iter->second.second;
    return true;
}


ECAD_INLINE bool ELayoutRetriever::GetLayerHeight(CPtr<ILayer> layer, FCoord & high, FCoord & low) const
{
    return GetLayerHeight(layer->GetLayerId(), high, low);
}

ECAD_INLINE bool ELayoutRetriever::GetComponentHeight(CPtr<IComponent> component, FCoord & high, FCoord & low) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    auto iter = m_lyrHeightsMap.find(component->GetPlacementLayer());
    if (iter == m_lyrHeightsMap.cend()) return false;
    if (component->isFlipped()) {
        high = iter->second.second;
        low = high - component->GetHeight();
    }
    else {
        low = iter->second.first;
        high = low + component->GetHeight();
    }
    return true;
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
        m_lyrHeightsMap.emplace(elevation, elevation - thickness);
    }
    //todo via layer
}

}//namespace eutils
}//namespace ecad
