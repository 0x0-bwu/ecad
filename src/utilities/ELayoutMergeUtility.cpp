#include "utilities/ELayoutMergeUtility.h"

#include <unordered_map>
#include "Interface.h"
#include "EShape.h"
namespace ecad {
namespace utils {

ECAD_INLINE void ELayoutMergeUtility::Merge(Ptr<ILayoutView> layout, CPtr<ILayoutView> other, CPtr<ILayerMap> layermap, const ETransform2D & transform)
{
    // ECAD_EFFICIENCY_TRACK("layout merge")
    if(layout == other) return;

    //Boundary
    // auto boundary = *(layout->GetBoundary());
    // auto otherBdy = *(other->GetBoundary());
    // otherBdy.Transform(transform);
    // boundary.ConvexHull(otherBdy);
    // layout->SetBoundary(UPtr<EPolygon>(new EPolygon(std::move(boundary))));

    //Net
    std::unordered_map<ENetId, ENetId> netIdMap;//<other, this>
    netIdMap.emplace(ENetId::noNet, ENetId::noNet);
    auto netIter = other->GetNetIter();
    while (auto * net = netIter->Next()){
        if (auto thisNet = layout->FindNetByName(net->GetName()); thisNet)
            netIdMap.emplace(net->GetNetId(), thisNet->GetNetId());
        else {
            auto clone = net->Clone();
            auto added = layout->GetNetCollection()->AddNet(std::move(clone));
            netIdMap.emplace(net->GetNetId(), added->GetNetId());
        }
    }

    UPtr<ILayerMap> defaultLyrMap;
    if (nullptr == layermap) {
        defaultLyrMap = layout->GetLayerCollection()->GetDefaultLayerMap();
        layermap = defaultLyrMap.get();
    }
    
    //HierarchyObj/Cellinst
    auto cellInstIter = other->GetCellInstIter();
    while (auto * cellInst = cellInstIter->Next()){
        auto clone = cellInst->Clone();//todo, move directly
        clone->SetRefLayoutView(layout);
        clone->AddTransform(transform);
        layout->GetCellInstCollection()->AddCellInst(std::move(clone));
    }
    
    //HierarchyObj/Component
    std::unordered_map<CPtr<IComponent>, CPtr<IComponent> > compMap;
    auto compIter = other->GetComponentIter();
    while (auto * comp = compIter->Next()) {
        auto clone = comp->Clone();
        clone->SetPlacementLayer(layermap->GetMappingForward(clone->GetPlacementLayer()));
        clone->AddTransform(transform);
        compMap.emplace(comp, layout->GetComponentCollection()->AddComponent(std::move(clone)));
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
        clone->SetLayer(layermap->GetMappingForward(clone->GetLayer()));

        //Transform
        auto primType = clone->GetPrimitiveType();
        switch(primType) {
            case EPrimitiveType::Geometry2D : {
                clone->GetGeometry2DFromPrimitive()->Transform(transform);
                break;
            }
            case EPrimitiveType::Bondwire : {
                clone->GetBondwireFromPrimitive()->Transform(transform);
                auto bondwire = clone->GetBondwireFromPrimitive();
                bool flipped;
                if (bondwire->GetEndLayer(&flipped) != ELayerId::ComponentLayer)
                    bondwire->SetEndLayer(layermap->GetMappingForward(bondwire->GetEndLayer()), flipped);
                auto iter = compMap.find(bondwire->GetStartComponent());
                if (iter != compMap.cend()) bondwire->SetStartComponent(iter->second);
                iter = compMap.find(bondwire->GetEndComponent());
                if (iter != compMap.cend()) bondwire->SetEndComponent(iter->second);
                break;
            }
            case EPrimitiveType::Text : {
                clone->GetTextFromPrimitive()->AddTransform(transform);
                break;
            }
            default : {
                ECAD_ASSERT(false)
                break;
            }
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

}//namespace utils
}//namespace ecad