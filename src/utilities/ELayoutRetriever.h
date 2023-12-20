#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include <unordered_map>
namespace ecad {

class ILayer;
class IComponent;
class ILayoutView;
class IStackupLayer;
namespace eutils {
class ECAD_API ELayoutRetriever
{
public:
    explicit ELayoutRetriever(CPtr<ILayoutView> layout);
    virtual ~ELayoutRetriever() = default;

    bool GetLayerStackupHeightThickness(FCoord & elevation, FCoord & thickness) const;
    bool GetLayerHeightThickness(ELayerId layerId, FCoord & elevation, FCoord & thickness) const;
    bool GetLayerHeightThickness(CPtr<ILayer> layer, FCoord & elevation, FCoord & thickness) const;
    bool GetComponentHeightThickness(CPtr<IComponent> component, FCoord & elevation, FCoord & thickness) const;

    CPtr<IStackupLayer> SearchStackupLayer(FCoord height) const;

private:
    void BuildLayerHeightsMap() const;

private:  
    CPtr<ILayoutView> m_layout{nullptr};
    mutable std::unordered_map<ELayerId, std::pair<FCoord, FCoord> > m_lyrHeightsMap;
};

}//namespace eutils
}//namespace ecad