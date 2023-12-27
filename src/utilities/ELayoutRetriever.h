#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include <unordered_map>
namespace ecad {

class ILayer;
class IBondwire;
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
    bool GetComponentBallBumpThickness(CPtr<IComponent> component, FCoord & elevation, FCoord & thickness) const;
    bool GetBondwireHeight(CPtr<IBondwire> bondwire, FCoord & start, FCoord & end, bool & startFlipped, bool & endFlipped) const;
    bool GetBondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<FCoord> & heights) const;
    bool GetBondwireSegmentsWithMinSeg(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<FCoord> & heights, size_t minSeg) const;
    bool GetBondwireSegmentsWithMaxLen(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<FCoord> & heights, ECoord maxLen) const;
    UPtr<EShape> GetBondwireStartSolderJointParameters(CPtr<IBondwire> bondwire, FCoord & elevation, FCoord & thickness, std::string & material) const;
    UPtr<EShape> GetBondwireEndSolderJointParameters(CPtr<IBondwire> bondwire, FCoord & elevation, FCoord & thickness, std::string & material) const;
    CPtr<IStackupLayer> SearchStackupLayer(FCoord height) const;

private:    
    UPtr<EShape> GetBondwireStartSolderJointShape(CPtr<IBondwire> bondwire, FCoord & thickness) const;
    UPtr<EShape> GetBondwireEndSolderJointShape(CPtr<IBondwire> bondwire, FCoord & thickness) const;
    bool GetSimpleBondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<FCoord> & heights) const;
    bool GetJedec4BondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<FCoord> & heights) const;
    void BuildLayerHeightsMap() const;

private:  
    CPtr<ILayoutView> m_layout{nullptr};
    mutable std::unordered_map<ELayerId, std::pair<FCoord, FCoord> > m_lyrHeightsMap;
};

}//namespace eutils
}//namespace ecad