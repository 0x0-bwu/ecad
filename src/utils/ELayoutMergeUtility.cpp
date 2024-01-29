#include "utils/ELayoutMergeUtility.h"

#include <unordered_map>
#include "EDataMgr.h"
namespace ecad {
namespace utils {

ECAD_INLINE bool ELayoutMergeUtility::Merge(Ptr<ILayoutView> layout, CPtr<ICellInst> cellInst)
{
    auto other = cellInst->GetFlattenedLayoutView();
    if(layout == other) return false;

    const auto & prefix = cellInst->GetName();
    const auto & transform = cellInst->GetTransform();
    const auto * layermap = cellInst->GetLayerMap();

    char sep = EDataMgr::Instance().HierSep();
    auto getName = [sep, &prefix](const std::string & name) { return prefix + sep + name; };

    //Net
    std::unordered_map<ENetId, ENetId> netIdMap;//<other, this>
    netIdMap.emplace(ENetId::noNet, ENetId::noNet);
    auto netIter = other->GetNetIter();
    while (auto * net = netIter->Next()){
        auto clone = net->Clone();
        clone->SetName(getName(net->GetName()));
        auto added = layout->GetNetCollection()->AddNet(std::move(clone));
        netIdMap.emplace(net->GetNetId(), added->GetNetId());
    }

    UPtr<ILayerMap> defaultLyrMap;
    if (nullptr == layermap) {
        defaultLyrMap = layout->GetLayerCollection()->GetDefaultLayerMap();
        layermap = defaultLyrMap.get();
    }
    
    //HierarchyObj/Cellinst
    auto cellInstIter = other->GetCellInstIter();
    while (auto * cellInst = cellInstIter->Next()){
        auto clone = cellInst->Clone();
        clone->SetName(getName(cellInst->GetName()));
        clone->SetRefLayoutView(layout);
        clone->AddTransform(transform);
        layout->GetCellInstCollection()->AddCellInst(std::move(clone));
    }
    
    //HierarchyObj/Component
    std::unordered_map<CPtr<IComponent>, CPtr<IComponent> > compMap;
    auto compIter = other->GetComponentIter();
    while (auto * comp = compIter->Next()) {
        auto clone = comp->Clone();
        clone->SetName(getName(comp->GetName()));
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
                    bondwire->SetEndLayer(layermap->GetMappingForward(bondwire->GetEndLayer()));
                auto iter = compMap.find(bondwire->GetStartComponent());
                if (iter != compMap.cend()) bondwire->SetStartComponent(iter->second, bondwire->GetStartComponentPin());
                iter = compMap.find(bondwire->GetEndComponent());
                if (iter != compMap.cend()) bondwire->SetEndComponent(iter->second, bondwire->GetEndComponentPin());
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
    return true;
}

}//namespace utils
}//namespace ecad