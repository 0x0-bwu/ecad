#ifndef ECAD_HEADER_ONLY
#include "utilities/ELayoutMergeUtility.h"
#endif

#include "Interface.h"
#include <unordered_map>
namespace ecad {
namespace euti {

ECAD_INLINE void ELayoutMergeUtility::Merge(Ptr<ILayoutView> layout, CPtr<ILayoutView> other, const ETransform2D & transform)
{
    // ECAD_EFFICIENCY_TRACK("layout merge")
    if(layout == other) return;

    //Boundary
    auto boundary = *(layout->GetBoundary());
    auto otherBdy = *(other->GetBoundary());
    otherBdy.Transform(transform);
    boundary.ConvexHull(otherBdy);
    layout->SetBoundary(UPtr<EPolygon>(new EPolygon(std::move(boundary))));

    //Net
    std::unordered_map<ENetId, ENetId> netIdMap;//<other, this>
    netIdMap.insert(std::make_pair(ENetId::noNet, ENetId::noNet));
    auto netIter = other->GetNetIter();
    while(auto * net = netIter->Next()){
        auto clone = net->Clone();
        auto added = layout->GetNetCollection()->AddNet(std::move(clone));
        netIdMap.insert(std::make_pair(net->GetNetId(), added->GetNetId()));
    }

    //Layer, todo, Layermap check
    auto lyrMap = layout->GetLayerCollection()->GetDefaultLayerMap();

    //HierarchyObj/Cellinst
    auto cellInstIter = other->GetCellInstIter();
    while(auto * cellInst = cellInstIter->Next()){
        auto clone = cellInst->Clone();
        clone->SetRefLayoutView(layout);
        clone->AddTransform(transform);
        layout->GetCellInstCollection()->AddCellInst(std::move(clone));
    }

    //Connobj/Primitive
    auto primIter = other->GetPrimitiveIter();
    while(auto * primitive = primIter->Next()){
        auto clone = primitive->Clone();
        //Net
        if(netIdMap.count(clone->GetNet()))
            clone->SetNet(netIdMap.at(clone->GetNet()));
        else clone->SetNet(ENetId::noNet);

        //Layer
        clone->SetLayer(lyrMap->GetMappingForward(clone->GetLayer()));

        //Transform
        auto primType = clone->GetPrimitiveType();
        switch(primType){
            case EPrimitiveType::Geometry2D : {
                clone->GetGeometry2DFromPrimitive()->Transform(transform);
                break;
            }
            case EPrimitiveType::Text : {
                clone->GetTextFromPrimitive()->AddTransform(transform);
                break;
            }
            default : break;
        }

        layout->GetPrimitiveCollection()->AddPrimitive(std::move(clone));
    }

    //Connobj/Padstackinst
    auto psInstIter = other->GetPadstackInstIter();
    while(auto * psInst = psInstIter->Next()){
        auto clone = psInst->Clone();
        //Net
        if(netIdMap.count(clone->GetNet()))
            clone->SetNet(netIdMap.at(clone->GetNet()));
        else clone->SetNet(ENetId::noNet);

        //todo, Layermap

        //Transform
        clone->AddTransform(transform);
        layout->GetPadstackInstCollection()->AddPadstackInst(std::move(clone));
    }
}

}//namespace euti
}//namespace ecad