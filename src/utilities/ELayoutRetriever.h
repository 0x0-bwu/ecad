#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include <unordered_map>
namespace ecad {

class ILayer;
class IComponent;
class ILayoutView;
namespace eutils {
class ECAD_API ELayoutRetriever
{
public:
    explicit ELayoutRetriever(CPtr<ILayoutView> layout);
    virtual ~ELayoutRetriever() = default;

    bool GetLayerHeight(ELayerId layerId, FCoord & high, FCoord & low) const;
    bool GetLayerHeight(CPtr<ILayer> layer, FCoord & high, FCoord & low) const;
    bool GetComponentHeight(CPtr<IComponent> component, FCoord & high, FCoord & low) const;

private:
    void BuildLayerHeightsMap() const;

private:  
    CPtr<ILayoutView> m_layout{nullptr};
    mutable std::unordered_map<ELayerId, std::pair<FCoord, FCoord> > m_lyrHeightsMap;
};

}//namespace eutils
}//namespace ecad