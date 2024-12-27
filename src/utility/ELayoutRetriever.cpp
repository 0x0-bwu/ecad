#include "ELayoutRetriever.h"
#include "interface/Interface.h"

#include "generic/geometry/Utility.hpp"

namespace ecad {
namespace utils {

ELayoutRetriever::ELayoutRetriever(CPtr<ILayoutView> layout)
 : m_layout(layout)
{
}

bool ELayoutRetriever::GetLayerStackupHeightThickness(EFloat & elevation, EFloat & thickness) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    std::vector<CPtr<IStackupLayer> > stackupLayers;
    m_layout->GetStackupLayers(stackupLayers);
    EFloat topElevation, topThickness, botElevation, botThickness;
    GetLayerHeightThickness(stackupLayers.front()->GetLayerId(), topElevation, topThickness);
    GetLayerHeightThickness(stackupLayers.back()->GetLayerId(), botElevation, botThickness);
    elevation = topElevation;
    thickness = topElevation - botElevation + botThickness;
    return true;
}

bool ELayoutRetriever::GetLayerHeightThickness(ELayerId layerId, EFloat & elevation, EFloat & thickness) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    auto iter = m_lyrHeightsMap.find(layerId);
    if (iter == m_lyrHeightsMap.cend()) return false;
    elevation = iter->second.first;
    thickness = iter->second.second;
    return true;
}

bool ELayoutRetriever::GetLayerHeightThickness(CPtr<ILayer> layer, EFloat & elevation, EFloat & thickness) const
{
    return GetLayerHeightThickness(layer->GetLayerId(), elevation, thickness);
}

bool ELayoutRetriever::GetComponentHeightThickness(CPtr<IComponent> component, EFloat & elevation, EFloat & thickness) const
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

bool ELayoutRetriever::GetComponentBallBumpThickness(CPtr<IComponent> component, EFloat & elevation, EFloat & thickness) const
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

bool ELayoutRetriever::GetBondwireHeight(CPtr<IBondwire> bondwire, EFloat & start, EFloat & end, bool & startFlipped, bool & endFlipped) const
{
    EFloat elevation, thickness;
    auto getHeight = [&](bool start, EFloat & height) {
        auto & flipped = start ? startFlipped : endFlipped;
        auto layerId = start ? bondwire->GetStartLayer(&flipped) : bondwire->GetEndLayer(&flipped);

        EFloat solderJointThickness;
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

bool ELayoutRetriever::GetBondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights) const
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

bool ELayoutRetriever::GetBondwireSegmentsWithMinSeg(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights, size_t minSeg) const
{
    if (pt2ds.empty()) {
        auto res =  GetBondwireSegments(bondwire, pt2ds, heights);
        if (not res) return false;
    }

    if (pt2ds.size() >= minSeg) return true;
    auto midPoint = [&](const EPoint2D & p1, EFloat h1, const EPoint2D & p2, EFloat h2) {
        return std::make_pair((p1 + p2) / 2, (h1 + h2) / 2);
    };

    std::vector<EFloat> newHts{heights.front()};
    std::vector<EPoint2D> newPts{pt2ds.front()};
    for (size_t i = 1; i < pt2ds.size(); ++i) {
        auto [mPt, mHt] = midPoint(newPts.back(), newHts.back(), pt2ds.at(i), heights.at(i));
        newPts.emplace_back(std::move(mPt));
        newPts.emplace_back(pt2ds.at(i));
        newHts.emplace_back(mHt);
        newHts.emplace_back(heights.at(i));
    }
    std::swap(newHts, heights);
    std::swap(newPts, pt2ds);
    return GetBondwireSegmentsWithMinSeg(bondwire, pt2ds, heights, minSeg);
}

bool ELayoutRetriever::GetBondwireSegmentsWithMaxLen(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights, ECoord maxLen) const
{
    if (not GetBondwireSegments(bondwire, pt2ds, heights)) return false;
    auto maxLenSq = maxLen * maxLen;
    auto scale2Int = m_layout->GetDatabase()->GetCoordUnits().Scale2Coord();
    auto distanceSq = [&](const EPoint2D & p1, EFloat h1, const EPoint2D & p2, EFloat h2) {
        return generic::geometry::DistanceSq(EPoint3D(p1[0], p1[1], h1 * scale2Int), EPoint3D(p2[0], p2[1], h2 * scale2Int));
    };
    auto midPoint = [&](const EPoint2D & p1, EFloat h1, const EPoint2D & p2, EFloat h2) {
        return std::make_pair((p1 + p2) / 2, (h1 + h2) / 2);
    };
    auto equal = [&](const EPoint2D & p1, EFloat h1, const EPoint2D & p2, EFloat h2) {
        return p1 == p2 && ECoord(h1 * scale2Int) == ECoord(h2 * scale2Int);
    };
    size_t i = 1;
    auto currentPt = pt2ds.at(i);
    auto currentHt = heights.at(i);
    std::vector<EFloat> newHts{heights.front()};
    std::vector<EPoint2D> newPts{pt2ds.front()};
    while (i < pt2ds.size()) {
        auto distSq = distanceSq(newPts.back(), newHts.back(), currentPt, currentHt);
        if (distSq > maxLenSq) {
            std::tie(currentPt, currentHt) = midPoint(newPts.back(), newHts.back(), currentPt, currentHt);
        }
        else {
            newHts.emplace_back(currentHt);
            newPts.emplace_back(currentPt);
            if (equal(currentPt, currentHt, pt2ds.at(i), heights.at(i))) ++i;
            if (i == pt2ds.size()) {
                newHts.emplace_back(heights.back());
                newPts.emplace_back(pt2ds.back());
                break;
            }
            currentHt = heights.at(i);
            currentPt = pt2ds.at(i);
        }
    }
    std::swap(newHts, heights);
    std::swap(newPts, pt2ds);
    return true;
}

UPtr<EShape> ELayoutRetriever::GetBondwireStartSolderJointParameters(CPtr<IBondwire> bondwire, EFloat & elevation, EFloat & thickness, std::string & material) const
{
    auto shape = GetBondwireStartSolderJointShape(bondwire, thickness);
    if (nullptr == shape) return nullptr;
    material = bondwire->GetSolderJoints()->GetPadstackDefData()->GetTopSolderBumpMaterial();
    EFloat start, end;
    bool startFlipped, endFlipped;
    if (not GetBondwireHeight(bondwire, start, end, startFlipped, endFlipped)) return nullptr;
    elevation = startFlipped ? start + thickness : start;
    return shape;
}

UPtr<EShape> ELayoutRetriever::GetBondwireEndSolderJointParameters(CPtr<IBondwire> bondwire, EFloat & elevation, EFloat & thickness, std::string & material) const
{
    auto shape = GetBondwireEndSolderJointShape(bondwire, thickness);
    if (nullptr == shape) return nullptr;
    material = bondwire->GetSolderJoints()->GetPadstackDefData()->GetBotSolderBallMaterial();
    EFloat start, end;
    bool startFlipped, endFlipped;
    if (not GetBondwireHeight(bondwire, start, end, startFlipped, endFlipped)) return nullptr;
    elevation = endFlipped ? end + thickness : end;
    return shape;
}

UPtr<EShape> ELayoutRetriever::GetBondwireStartSolderJointShape(CPtr<IBondwire> bondwire, EFloat & thickness) const
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

UPtr<EShape> ELayoutRetriever::GetBondwireEndSolderJointShape(CPtr<IBondwire> bondwire, EFloat & thickness) const
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

bool ELayoutRetriever::GetSimpleBondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights) const
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

bool ELayoutRetriever::GetJedec4BondwireSegments(CPtr<IBondwire> bondwire, std::vector<EPoint2D> & pt2ds, std::vector<EFloat> & heights) const
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

CPtr<IStackupLayer> ELayoutRetriever::SearchStackupLayer(EFloat height) const
{
    if (m_lyrHeightsMap.empty()) BuildLayerHeightsMap();
    std::vector<CPtr<IStackupLayer> > stackupLayers;
    m_layout->GetStackupLayers(stackupLayers);

    using namespace generic::math;
    EFloat elevation, thickness;
    for (auto stackupLayer : stackupLayers) {
        GetLayerHeightThickness(stackupLayer->GetLayerId(), elevation, thickness);
        if (Within<LORC, EFloat>(height, elevation - thickness, elevation))
            return stackupLayer;
    }
    return nullptr;
}

UPtr<EShape> ELayoutRetriever::CalculateLayoutBoundaryShape(CPtr<ILayoutView> layout)
{
    if(nullptr == layout) return nullptr;

    std::unordered_map<ELayerId, EBox2D> bounds;
    auto lyrIter = layout->GetLayerIter();
    while(auto lyr = lyrIter->Next())
        bounds.insert(std::make_pair(lyr->GetLayerId(), EBox2D{}));

    auto primIter = layout->GetPrimitiveIter();
    while(auto prim = primIter->Next()){
        auto layer = prim->GetLayer();
        if(!bounds.count(layer)) continue;

        auto primType = prim->GetPrimitiveType();
        switch(primType){
            case EPrimitiveType::Geometry2D : {
                auto shape = prim->GetGeometry2DFromPrimitive()->GetShape();
                if(shape) bounds[layer] |= shape->GetBBox();
                break;
            }
            case EPrimitiveType::Text : {
                break;
            }
            default : break;
        }
    }

    EBox2D bbox;
    for(const auto & bound : bounds) bbox |= bound.second;

    auto cellInstIter = layout->GetCellInstIter();
    while(auto cellInst = cellInstIter->Next()){
        auto bdy = cellInst->GetDefLayoutView()->GetBoundary()->Clone();
        bdy->Transform(cellInst->GetTransform());
        bbox |= bdy->GetBBox();
    }

    auto boundary = new EPolygon;
    boundary->shape = generic::geometry::toPolygon(bbox);
    return std::unique_ptr<EShape>(boundary);
}

void ELayoutRetriever::BuildLayerHeightsMap() const
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

}//namespace utils
}//namespace ecad
