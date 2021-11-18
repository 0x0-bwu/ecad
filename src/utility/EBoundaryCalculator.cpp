#ifndef ECAD_HEADER_ONLY
#include "utility/EBoundaryCalculator.h"
#endif//ECAD_HEADER_ONLY

#include "generic/geometry/Utility.hpp"
#include "Interface.h"
#include <unordered_map>
namespace ecad {
namespace euti {

ECAD_INLINE UPtr<EPolygon> CalculateBoundary(CPtr<ILayoutView> layout)
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

    auto boundary = UPtr<EPolygon>(new EPolygon);
    boundary->shape = generic::geometry::toPolygon(bbox);
    return std::move(boundary);
}

}//namespace euti
}//namespace ecad