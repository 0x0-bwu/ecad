#include "ELayoutConnectivity.h"

#include "generic/geometry/Connectivity.hpp"

#include "interface/IPadstackInstCollection.h"
#include "interface/IPrimitiveCollection.h"
#include "interface/IConnObjCollection.h"
#include "interface/INetCollection.h"
#include "interface/IPadstackInst.h"
#include "interface/ILayoutView.h"
#include "interface/IPrimitive.h"
#include "interface/IConnObj.h"
#include "interface/ILayer.h"
#include "interface/INet.h"
#include "basic/EShape.h"
#include "EDataMgr.h"
namespace ecad {
namespace utils {

ECAD_INLINE void ELayoutConnectivity::ConnectivityExtraction(Ptr<ILayoutView> layout)
{
    ECAD_EFFICIENCY_TRACK("layout connectivity extraction")
    generic::geometry::ConnectivityExtractor<ECoord> extractor;

    //add layers connection
    std::vector<CPtr<IStackupLayer> > layers;
    layout->GetStackupLayers(layers);
    for(size_t i = 0; i < layers.size() - 1; ++i){
        size_t j = i + 1;
        auto lyr1 = layers[i]->GetLayerId();
        auto lyr2 = layers[j]->GetLayerId();
        extractor.AddLayerConnection(static_cast<size_t>(lyr1), static_cast<size_t>(lyr2));
    }

    //add objects connection
    auto geomGetter = [](Ptr<IGeometry2D> geom)
    {
        return geom->GetShape()->GetPolygonWithHoles();
    };

    auto textGetter = [](Ptr<IText> text) {
        auto position = text->GetPosition();
        return EBox2D(position, position + EPoint2D(1, 1));
    };

    size_t index(0);
    std::unordered_set<size_t> textLabels;
    std::unordered_map<size_t, Ptr<IConnObj> > connObjMap;
    //primitive
    auto primCollection = layout->GetConnObjCollection()->GetPrimitiveCollection();
    auto primIter = primCollection->GetPrimitiveIter();
    while(auto prim = primIter->Next()){
        auto layer = static_cast<size_t>(prim->GetLayer());
        auto primType = prim->GetPrimitiveType();
        switch(primType){
            case EPrimitiveType::Geometry2D : {
                auto geom = prim->GetGeometry2DFromPrimitive();
                index = extractor.AddObject(layer, geom, geomGetter);
                connObjMap.insert(std::make_pair(index, dynamic_cast<Ptr<IConnObj> >(geom)));
                break;
            }
            case EPrimitiveType::Text : {
                auto text = prim->GetTextFromPrimitive();
                index = extractor.AddObject(layer, text, textGetter);
                textLabels.insert(index);
                connObjMap.insert(std::make_pair(index, dynamic_cast<Ptr<IConnObj> >(text)));
                break;
            }
            default : break;
        }
    }

    //padstack inst
    auto shapeGetter = [](const EShape & shape)
    {
        return shape.GetPolygonWithHoles();
    };

    ELayerId top = noLayer, bot = noLayer;
    auto psInstCollection = layout->GetPadstackInstCollection();
    auto psInstIter = psInstCollection->GetPadstackInstIter();
    while(auto psInst = psInstIter->Next()){
        bool added = false;
        psInst->GetLayerRange(top, bot);
        for(auto lyr = static_cast<int>(top); lyr <= static_cast<int>(bot); ++lyr){
            auto shape = psInst->GetLayerShape(static_cast<ELayerId>(lyr));
            if(nullptr == shape) continue;
            index = extractor.AddObject(static_cast<size_t>(lyr), *shape, shapeGetter);
            if(!added){
                connObjMap.insert(std::make_pair(index, dynamic_cast<Ptr<IConnObj> >(psInst)));
                added = true;
            }
        }
    }

    //extract
    auto graph = extractor.Extract(EDataMgr::Instance().Threads());
    std::vector<std::list<size_t> > cc;
    generic::topology::ConnectedComponents(*graph, cc);
    
    //clear and add nets
    auto nc = layout->GetNetCollection();
    nc->Clear();
    for(size_t i = 0; i < cc.size(); ++i){
        std::string netName;
        const auto & component = cc[i];
        for(auto index : component){
            if(textLabels.count(index)){
                auto text = dynamic_cast<Ptr<IText> >(connObjMap.at(index));
                if(text){
                    netName = text->GetText();
                    break;
                }
            }
        }
        if(netName.empty()) netName = "Auto_Net";
        auto net = nc->CreateNet(nc->NextNetName(netName));
        auto netId = net->GetNetId();
        for(auto index : component){
            if(!connObjMap.count(index)) continue;
            connObjMap.at(index)->SetNet(netId);
        }
    }
}

}//namespace utils
}//namespace ecad