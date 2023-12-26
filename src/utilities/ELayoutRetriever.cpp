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
    thickness = component->GetComponentDef()->GetHeight();
    if (component->isFlipped())
        elevation = iter->second.first - iter->second.second - component->GetComponentDef()->GetSolderBallBumpHeight();
    else elevation = iter->second.first + component->GetComponentDef()->GetSolderBallBumpHeight() + thickness;
    return true;
}

ECAD_INLINE bool ELayoutRetriever::GetComponentBallBumpThickness(CPtr<IComponent> component, FCoord & elevation, FCoord & thickness) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    auto iter = m_lyrHeightsMap.find(component->GetPlacementLayer());
    if (iter == m_lyrHeightsMap.cend()) return false;
    thickness = component->GetComponentDef()->GetSolderBallBumpHeight();
    if (component->isFlipped())
        elevation = iter->second.first - iter->second.second;
    else elevation = iter->second.first + component->GetComponentDef()->GetSolderBallBumpHeight();
    return true;
}

ECAD_INLINE bool ELayoutRetriever::GetBondwireHeight(CPtr<IBondwire> bondwire, FCoord & start, FCoord & end, bool & startFlipped, bool & endFlipped) const
{
    FCoord elevation, thickness;
    auto getHeight = [&](bool start, FCoord & height) {
        auto & flipped = start ? startFlipped : endFlipped;
        auto layerId = start ? bondwire->GetStartLayer(&flipped) : bondwire->GetEndLayer(&flipped);

        FCoord solderJointThickness;
        [[maybe_unused]] auto solderJoint = start ? GetBondwireStartSolderJointShape(bondwire, solderJointThickness) :
                                                    GetBondwireEndSolderJointShape(bondwire, solderJointThickness);
        if (ELayerId::ComponentLayer == layerId) {
            auto comp = start ? bondwire->GetStartComponent() : bondwire->GetEndComponent(); { ECAD_ASSERT(comp) }
            if (not GetComponentHeightThickness(comp, elevation, thickness)) return false;
            height = flipped ? elevation - thickness : elevation;
            if (solderJoint) height = flipped ? height - solderJointThickness : height + solderJointThickness;
            return true;
        }
        else if (ELayerId::noLayer != layerId) {
            if (not GetLayerHeightThickness(layerId, elevation, thickness)) return false;
            height = flipped ? elevation - thickness : elevation;
            if (solderJoint) height = flipped ? height - solderJointThickness : height + solderJointThickness;
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
    switch (bondwire->GetBondwireType())
    {
        case EBondwireType::Simple :
            return GetSimpleBondwireSegments(bondwire, pt2ds, heights);
        case EBondwireType::JEDEC4 :
            return GetJedec4BondwireSegments(bondwire, pt2ds, heights);
        default:
            ECAD_ASSERT(false)
            return false;
    }
}

ECAD_INLINE UPtr<EShape> ELayoutRetriever::GetBondwireStartSolderJointParameters(CPtr<IBondwire> bondwire, FCoord & elevation, FCoord & thickness, std::string & material) const
{
    auto shape = GetBondwireStartSolderJointShape(bondwire, thickness);
    if (nullptr == shape) return nullptr;
    material = bondwire->GetSolderJoints()->GetPadstackDefData()->GetTopSolderBumpMaterial();
    FCoord start, end;
    bool startFlipped, endFlipped;
    if (not GetBondwireHeight(bondwire, start, end, startFlipped, endFlipped)) return nullptr;
    elevation = startFlipped ? start + thickness : start;
    return shape;
}

ECAD_INLINE UPtr<EShape> ELayoutRetriever::GetBondwireEndSolderJointParameters(CPtr<IBondwire> bondwire, FCoord & elevation, FCoord & thickness, std::string & material) const
{
    auto shape = GetBondwireEndSolderJointShape(bondwire, thickness);
    if (nullptr == shape) return nullptr;
    material = bondwire->GetSolderJoints()->GetPadstackDefData()->GetBotSolderBallMaterial();
    FCoord start, end;
    bool startFlipped, endFlipped;
    if (not GetBondwireHeight(bondwire, start, end, startFlipped, endFlipped)) return nullptr;
    elevation = endFlipped ? end + thickness : end;
    return shape;
}

ECAD_INLINE UPtr<EShape> ELayoutRetriever::GetBondwireStartSolderJointShape(CPtr<IBondwire> bondwire, FCoord & thickness) const
{
    auto sj = bondwire->GetSolderJoints();
    if (nullptr == sj) return nullptr;
    if (not sj->GetPadstackDefData()->hasTopSolderBump()) return nullptr;
    
    CPtr<EShape> shape;
    sj->GetPadstackDefData()->GetTopSolderBumpParameters(shape, thickness);
    ECAD_ASSERT(shape)

    auto shapeInst = shape->Clone();
    shapeInst->Transform(makeETransform2D(1.0, 0.0, bondwire->GetStartPt()));
    return shapeInst;
}

ECAD_INLINE UPtr<EShape> ELayoutRetriever::GetBondwireEndSolderJointShape(CPtr<IBondwire> bondwire, FCoord & thickness) const
{
    auto sj = bondwire->GetSolderJoints();
    if (nullptr == sj) return nullptr;
    if (not sj->GetPadstackDefData()->hasBotSolderBall()) return nullptr;
    
    CPtr<EShape> shape;
    sj->GetPadstackDefData()->GetBotSolderBallParameters(shape, thickness);
    ECAD_ASSERT(shape)

    auto shapeInst = shape->Clone();
    shapeInst->Transform(makeETransform2D(1.0, 0.0, bondwire->GetEndPt()));
    return shapeInst;
}

ECAD_INLINE bool ELayoutRetriever::GetSimpleBondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<FCoord> & heights) const
{
   //simple
    pt2ds.resize(4);
    heights.resize(4);  
    pt2ds[0] = bondwire->GetStartPt();
    pt2ds[3] = bondwire->GetEndPt();
    pt2ds[1] = pt2ds.at(0) + (pt2ds.at(3) - pt2ds.at(0)) * 0.125;
    pt2ds[2] = pt2ds.at(0) + (pt2ds.at(3) - pt2ds.at(0)) * 0.875;
    bool startFlipped, endFlipped;
    if (not GetBondwireHeight(bondwire, heights.front(), heights.back(), startFlipped, endFlipped)) return false;
    heights[1] = startFlipped ? heights[0] - bondwire->GetHeight() : heights[0] + bondwire->GetHeight();
    heights[2] = endFlipped ? heights[3] - bondwire->GetHeight() : heights[3] + bondwire->GetHeight();
    return true;  
}

ECAD_INLINE bool ELayoutRetriever::GetJedec4BondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<FCoord> & heights) const
{
   //JEDEC4
    pt2ds.resize(4);
    heights.resize(4);  
    pt2ds[0] = bondwire->GetStartPt();
    pt2ds[1] = pt2ds.front();
    pt2ds[3] = bondwire->GetEndPt();
    pt2ds[2] = pt2ds.at(0) + (pt2ds.at(3) - pt2ds.at(0)) * 0.125;
    bool startFlipped, endFlipped;
    if (not GetBondwireHeight(bondwire, heights.front(), heights.back(), startFlipped, endFlipped)) return false;
    heights[2] = heights[1] = startFlipped ? heights[0] - bondwire->GetHeight() : heights[0] + bondwire->GetHeight();
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
