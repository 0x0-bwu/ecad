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
namespace utils {
class ECAD_API ELayoutRetriever
{
public:
    explicit ELayoutRetriever(CPtr<ILayoutView> layout);
    virtual ~ELayoutRetriever() = default;

    bool GetLayerStackupHeightThickness(EFloat & elevation, EFloat & thickness) const;
    bool GetLayerHeightThickness(ELayerId layerId, EFloat & elevation, EFloat & thickness) const;
    bool GetLayerHeightThickness(CPtr<ILayer> layer, EFloat & elevation, EFloat & thickness) const;
    bool GetComponentHeightThickness(CPtr<IComponent> component, EFloat & elevation, EFloat & thickness) const;
    bool GetComponentBallBumpThickness(CPtr<IComponent> component, EFloat & elevation, EFloat & thickness) const;
    bool GetBondwireHeight(CPtr<IBondwire> bondwire, EFloat & start, EFloat & end, bool & startFlipped, bool & endFlipped) const;
    bool GetBondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights) const;
    bool GetBondwireSegmentsWithMinSeg(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights, size_t minSeg) const;
    bool GetBondwireSegmentsWithMaxLen(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights, ECoord maxLen) const;
    UPtr<EShape> GetBondwireStartSolderJointParameters(CPtr<IBondwire> bondwire, EFloat & elevation, EFloat & thickness, std::string & material) const;
    UPtr<EShape> GetBondwireEndSolderJointParameters(CPtr<IBondwire> bondwire, EFloat & elevation, EFloat & thickness, std::string & material) const;
    CPtr<IStackupLayer> SearchStackupLayer(EFloat height) const;

    static UPtr<EShape> CalculateLayoutBoundaryShape(CPtr<ILayoutView> layout);
private:    
    UPtr<EShape> GetBondwireStartSolderJointShape(CPtr<IBondwire> bondwire, EFloat & thickness) const;
    UPtr<EShape> GetBondwireEndSolderJointShape(CPtr<IBondwire> bondwire, EFloat & thickness) const;
    bool GetSimpleBondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights) const;
    bool GetJedec4BondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights) const;
    void BuildLayerHeightsMap() const;

private:  
    CPtr<ILayoutView> m_layout{nullptr};
    mutable std::unordered_map<ELayerId, std::pair<EFloat, EFloat> > m_lyrHeightsMap;
};

}//namespace utils
}//namespace ecad