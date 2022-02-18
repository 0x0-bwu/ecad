#ifndef ECAD_HEADER_ONLY
#include "ECadExtXflHandler.h"
#endif

#include "generic/geometry/Utility.hpp"
#include "generic/tools/Format.hpp"
#include "EXflParser.h"
#include "ETransform.h"
#include "Interface.h"
#include "ELayerMap.h"
#include "EDataMgr.h"
namespace ecad {
namespace ext {
namespace xfl {

namespace fmt = generic::format;

ECAD_INLINE ECadExtXflHandler::ECadExtXflHandler(const std::string & xflFile, size_t circleDiv)
 : m_xflFile(xflFile), m_circleDiv(circleDiv) {}

ECAD_INLINE SPtr<IDatabase> ECadExtXflHandler::CreateDatabase(const std::string & name, std::string * err)
{
    auto & mgr = EDataMgr::Instance();
    if(mgr.OpenDatabase(name)){
        if(err) *err = fmt::Format2String("Error: database %1% is already exist.", name);
        return nullptr;
    }

    Reset();

    EXflReader reader(*m_xflDB);
    if(!reader(m_xflFile)) return nullptr;

    m_xflDB->BuildLUTs();

    //reset temporary data
    m_database = mgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    //set units
    ECoordUnits coordUnits(m_xflDB->unit);
    m_database->SetCoordUnits(coordUnits);
    m_scale = coordUnits.Scale2Coord() / m_xflDB->scale;

    //import material
    //todo

    //create top cell
    auto cell = mgr.CreateCircuitCell(m_database, name);
    auto layout = cell->GetLayoutView();

    //import layers
    ImportLayers(layout);

    //import padstack def, should import layers firstly
    ImportPadstackDefs();

    //import nets
    ImportNets(layout);

    //import connection objects, should import nets firstly
    ImportConnObjs(layout);

    return m_database;
}

ECAD_INLINE void ECadExtXflHandler::ImportPadstackDefs()
{
    auto & mgr = EDataMgr::Instance();
    EShapeGetter eShapeGetter(m_scale, m_circleDiv);

    for(const auto & xflVia : m_xflDB->vias){
        auto xflPs = m_xflDB->GetPadstack(xflVia.padstackId);
        if(nullptr == xflPs){
            //todo, error handle
            continue;
        }

        auto xflShape = m_xflDB->GetTemplateShape(xflVia.shapeId);
        if(nullptr == xflShape){
            //todo, error handle
            continue;
        }

        auto psDefData = mgr.CreatePadstackDefData();

        auto eShape = boost::apply_visitor(eShapeGetter, xflShape->shape);
        GENERIC_ASSERT(eShape != nullptr)

        psDefData->SetViaParameters(std::move(eShape), EPoint2D(0, 0), math::Rad(xflVia.shapeRot));
        psDefData->SetMaterial(xflVia.material);

        auto layerMap = mgr.CreateLayerMap(m_database, xflVia.name);
    
        std::vector<std::string> padLayers;
        for(size_t i = 0; i < xflPs->pads.size(); ++i)
            padLayers.emplace_back("layer_" + std::to_string(i+1));
        psDefData->SetLayers(padLayers);
        for(size_t i = 0; i < xflPs->pads.size(); ++i){
            const auto & xflPad = xflPs->pads.at(i);
            xflShape = m_xflDB->GetTemplateShape(xflPad.shapeId);
            if(nullptr == xflShape){
                //todo, error handle
                continue;
            }
            eShape = boost::apply_visitor(eShapeGetter, xflShape->shape);
            GENERIC_ASSERT(eShape != nullptr)

            psDefData->SetPadParameters(static_cast<ELayerId>(i), std::move(eShape), EPoint2D(0, 0), math::Rad(xflPad.shapeRot));
        
            //todo, anti-pad

            //layer map
            layerMap->SetMapping(static_cast<ELayerId>(i), m_metalLyrIdMap.at(xflPad.sigLyr));
        }

        auto psDef = mgr.CreatePadstackDef(m_database.get(), xflVia.name);
        GENERIC_ASSERT(psDef != nullptr)
        psDef->SetPadstackDefData(std::move(psDefData));
    }

}

ECAD_INLINE void ECadExtXflHandler::ImportLayers(Ptr<ILayoutView> layout)
{
    auto & mgr = EDataMgr::Instance();

    int id = -1;
    int xflLayerId = 1;
    int xflMetalId = 1;
    FCoord elevation = 0.0;
    std::vector<UPtr<ILayer> > layers;
    for(const auto & xflLyr : m_xflDB->layers){
        id++;
        ELayerType type = xflLyr.type == 'D' ? ELayerType::DielectricLayer : ELayerType::ConductingLayer;
        auto layer = mgr.CreateStackupLayer(xflLyr.name, type, elevation * m_scale, xflLyr.thickness * m_scale);
        if(type != ELayerType::DielectricLayer)
            m_metalLyrIdMap.insert(std::make_pair(xflMetalId++, static_cast<ELayerId>(id)));
        m_layerIdMap.insert(std::make_pair(xflLayerId++, static_cast<ELayerId>(id)));
        layers.push_back(std::move(layer));
        elevation -= xflLyr.thickness;
    }
    layout->AppendLayers(CloneHelper(layers));
}

ECAD_INLINE void ECadExtXflHandler::ImportNets(Ptr<ILayoutView> layout)
{
    auto & mgr = EDataMgr::Instance();

    for(const auto & xflNet : m_xflDB->nets){
        auto net = mgr.CreateNet(layout, xflNet.name);
        if(nullptr == net){
            //todo, error handle
            continue;
        }
        m_netIdMap.insert(std::make_pair(xflNet.name, net->GetNetId()));
    }
}

ECAD_INLINE void ECadExtXflHandler::ImportConnObjs(Ptr<ILayoutView> layout)
{
    auto & mgr = EDataMgr::Instance();
    EShapeGetter eShapeGetter(m_scale, m_circleDiv);

    for(const auto & route : m_xflDB->routes) {
        auto net = mgr.FindNetByName(layout, route.net);
        if(nullptr == net){
            //todo, error handle
            continue;
        }
        auto netId = net->GetNetId();
        for(const auto & instObj : route.objects){
            if(auto * instPath = boost::get<InstPath>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instPath->layer);
                if(layer == m_metalLyrIdMap.end()){
                    //todo, error handle
                    continue;
                }
                auto eShape = eShapeGetter(xflShape->shape);
                auto ePath = mgr.CreateShapePath(eShape->GetContour().GetPoints(), instPaht->width * m_scale);

                auto ePrim = mgr.CreateGeometry2D(layout, layer->second, netId, std::move(ePath));
                GENERIC_ASSERT(ePrim != nullptr)
            }
            else if(auto * instVia = boost::get<InstVia>(&instObj)) {
                auto sLayer = m_metalLyrIdMap.find(instVia->sLayer);
                auto eLayer = m_metalLyrIdMap.find(instVia->eLayer);
                if(sLayer == m_metalLyrIdMap.end() ||
                    eLayer == m_metalLyrIdMap.end()) {
                        //todo, error handle
                        continue;
                }

                auto layerMap = mgr.FindLayerMapByName(m_database, instVia.name);
                if(nullptr == layerMap) {
                    //todo, error handle
                    continue;
                }

                auto psDef = mgr.FindPadstackDefByName(m_database, instVia.name);
                if(nullptr == psDef) {
                    //todo, error handle
                    continue;
                }



            }
        }
    }
}


ECAD_INLINE void ECadExtXflHandler::Reset()
{
    m_scale = 1.0;

    m_database.reset();
    m_xflDB.reset(new EXflDB);

    m_netIdMap.clear();
    m_layerIdMap.clear();
    m_metalLyrIdMap.clear();
}

}//namespace xfl
}//namespace ext
}//namespace ecad