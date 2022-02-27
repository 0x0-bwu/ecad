#ifndef ECAD_HEADER_ONLY
#include "ECadExtXflHandler.h"
#endif

#include "generic/geometry/Utility.hpp"
#include "generic/tools/Format.hpp"
#include "EMaterialProp.h"
#include "EXflParser.h"
#include "ETransform.h"
#include "Interface.h"
#include "EDataMgr.h"
#include <boost/geometry/index/rtree.hpp>
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
    ImportMaterialDefs();

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

    //import board geom
    ImportBoardGeom(layout);


    return m_database;
}

ECAD_INLINE void ECadExtXflHandler::ImportMaterialDefs()
{
    auto & mgr = EDataMgr::Instance();

    for(const auto & xflMat : m_xflDB->materials) {
        auto name = m_database->GetNextDefName(xflMat.name, EDefinitionType::MaterialDef);
        auto material = mgr.CreateMaterialDef(m_database, name);
        if(nullptr == material) {
            //todo, error handle
            continue;
        }

        m_matNameMap.insert(std::make_pair(xflMat.name, name));

        auto conductivity = mgr.CreateSimpleMaterialProp(xflMat.conductivity);
        material->SetProperty(EMaterialPropId::Conductivity, std::move(conductivity));

        auto permittivity = mgr.CreateSimpleMaterialProp(xflMat.permittivity);
        material->SetProperty(EMaterialPropId::Permittivity, std::move(permittivity));

        auto permeability = mgr.CreateSimpleMaterialProp(xflMat.permeability);
        material->SetProperty(EMaterialPropId::Permeability, std::move(permeability));

        auto lossTangent = mgr.CreateSimpleMaterialProp(xflMat.lossTangent);
        material->SetProperty(EMaterialPropId::MagneticLossTangent, std::move(lossTangent));
    }
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

        if(!m_matNameMap.count(xflVia.material)){
            //todo, error handle
            continue;
        }
        psDefData->SetMaterial(m_matNameMap.at(xflVia.material));

        auto eShape = boost::apply_visitor(eShapeGetter, xflShape->shape);
        GENERIC_ASSERT(eShape != nullptr)

        psDefData->SetViaParameters(std::move(eShape), EPoint2D(0, 0), math::Rad(xflVia.shapeRot));

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

        auto psDef = mgr.CreatePadstackDef(m_database, xflVia.name);
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
        std::string conductingMat = sDefaultConductingMat;
        std::string dielectricMat = sDefaultDielectricMat;
        auto iter_c = m_matNameMap.find(xflLyr.conductingMat);
        if(iter_c != m_matNameMap.end()) conductingMat = iter_c->second;
        auto iter_d = m_matNameMap.find(xflLyr.dielectricMat);
        if(iter_d != m_matNameMap.end()) dielectricMat = iter_d->second;

        auto layer = mgr.CreateStackupLayer(xflLyr.name, type,
                                            elevation * m_scale, xflLyr.thickness * m_scale,
                                            conductingMat, dielectricMat);

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
    auto toEPoint2D = [this](const Point & p) { return EPoint2D(m_scale * p.x, m_scale * p.y); };

    using TShape = std::pair<bool, UPtr<EShape> >;//true-isHole
    using TLayerShapes = std::vector<TShape>;
    using TLayerShapesMap = std::unordered_map<ELayerId, TLayerShapes>;
    for(const auto & route : m_xflDB->routes) {
        auto net = mgr.FindNetByName(layout, route.net);
        if(nullptr == net){
            //todo, error handle
            continue;
        }
        auto netId = net->GetNetId();
        TLayerShapesMap tLayerShapesMap;
        for(const auto & instObj : route.objects){
            //inst path
            if(auto * instPath = boost::get<InstPath>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instPath->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto eShape = eShapeGetter(instPath->path);
                auto ePath = mgr.CreateShapePath(eShape->GetContour().GetPoints(), instPath->width * m_scale);

                tLayerShapesMap.insert(std::make_pair(layer->second, TLayerShapes{})).first->second
                                .emplace_back(std::make_pair(false, std::move(ePath)));
            }
            //inst padstack
            else if(auto * instVia = boost::get<InstVia>(&instObj)) {
                auto sLayer = m_metalLyrIdMap.find(instVia->sLayer);
                auto eLayer = m_metalLyrIdMap.find(instVia->eLayer);
                if(sLayer == m_metalLyrIdMap.end() ||
                    eLayer == m_metalLyrIdMap.end()) {
                        //todo, error handle
                        continue;
                }

                auto layerMap = mgr.FindLayerMapByName(m_database, instVia->name);
                if(nullptr == layerMap) {
                    //todo, error handle
                    continue;
                }

                auto psDef = mgr.FindPadstackDefByName(m_database, instVia->name);
                if(nullptr == psDef) {
                    //todo, error handle
                    continue;
                }

                EMirror2D mirror = instVia->mirror == 'Y' ? EMirror2D::Y : EMirror2D::No;
                auto trans = makeETransform2D(1.0, math::Rad(instVia->rot), toEPoint2D(instVia->loc), mirror);
                
                auto name = GetNextPadstackInstName(instVia->name);
                auto psInst = mgr.CreatePadstackInst(layout, name, psDef, netId, sLayer->second, eLayer->second, layerMap, trans);
                GENERIC_ASSERT(psInst != nullptr)
            }
            //inst bondwire
            else if(auto * instBw = boost::get<InstBondwire>(&instObj)) {
                //todo
                continue;
            }
            //inst polygon
            else if(auto * instPolygon = boost::get<InstPolygon>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instPolygon->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto eShape = eShapeGetter(instPolygon->polygon);
                tLayerShapesMap.insert(std::make_pair(layer->second, TLayerShapes{})).first->second
                                .emplace_back(std::make_pair(instPolygon->isVoid, std::move(eShape)));
            }
            //inst rectangle
            else if(auto * instRect = boost::get<InstRectangle>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instRect->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto eShape = eShapeGetter(instRect->rectangle);
                eShape->Transform(makeETransform2D(1.0, 0.0, toEPoint2D(instRect->loc)));

                tLayerShapesMap.insert(std::make_pair(layer->second, TLayerShapes{})).first->second
                                .emplace_back(std::make_pair(instRect->isVoid, std::move(eShape)));
            }
            //inst square
            else if(auto * instSquare = boost::get<InstSquare>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instSquare->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto eShape = eShapeGetter(instSquare->square);
                eShape->Transform(makeETransform2D(1.0, 0.0, toEPoint2D(instSquare->loc)));

                tLayerShapesMap.insert(std::make_pair(layer->second, TLayerShapes{})).first->second
                                .emplace_back(std::make_pair(instSquare->isVoid, std::move(eShape)));
            }
            //inst diamond
            else if(auto * instDiamond = boost::get<InstDiamond>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instDiamond->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto eShape = eShapeGetter(instDiamond->diamond);
                eShape->Transform(makeETransform2D(1.0, 0.0, toEPoint2D(instDiamond->loc)));
                tLayerShapesMap.insert(std::make_pair(layer->second, TLayerShapes{})).first->second
                                .emplace_back(std::make_pair(instDiamond->isVoid, std::move(eShape)));
            }
            //inst circle
            else if(auto * instCircle = boost::get<InstCircle>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instCircle->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto eShape = eShapeGetter(instCircle->circle);
                eShape->Transform(makeETransform2D(1.0, 0.0, toEPoint2D(instCircle->loc)));
                tLayerShapesMap.insert(std::make_pair(layer->second, TLayerShapes{})).first->second
                                .emplace_back(std::make_pair(instCircle->isVoid, std::move(eShape)));
            }
            //inst annular
            else if(auto * instAnnular = boost::get<InstAnnular>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instAnnular->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto eShape = eShapeGetter(instAnnular->annular);
                eShape->Transform(makeETransform2D(1.0, 0.0, toEPoint2D(instAnnular->loc)));
                tLayerShapesMap.insert(std::make_pair(layer->second, TLayerShapes{})).first->second
                                .emplace_back(std::make_pair(false, std::move(eShape)));
            }
            //inst composite
            else if(auto * instComp = boost::get<InstComposite>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instComp->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto eShape = eShapeGetter(instComp->composite);
                tLayerShapesMap.insert(std::make_pair(layer->second, TLayerShapes{})).first->second
                                .emplace_back(std::make_pair(instComp->isVoid, std::move(eShape)));
            }
            //inst shape from template
            else if(auto * instShape = boost::get<InstShape>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instShape->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }

                auto temp = m_xflDB->GetTemplateShape(instShape->shapeId);
                if(nullptr == temp) {
                    //todo, error handle
                    continue;
                }

                auto eShape = boost::apply_visitor(eShapeGetter, temp->shape);
                EMirror2D m = instShape->mirror == 'N' ? EMirror2D::No : 
                             (instShape->mirror == 'X' ? EMirror2D::X  : EMirror2D::Y);
                if(instShape->rotThenMirror) {
                    eShape->Transform(makeETransform2D(1.0, math::Rad(instShape->rot), toEPoint2D(instShape->loc), m));
                }
                else {
                    eShape->Transform(makeETransform2D(1.0, 0.0, EPoint2D(0, 0), m));
                    eShape->Transform(makeETransform2D(1.0, math::Rad(instShape->rot), toEPoint2D(instShape->loc)));
                }
                tLayerShapesMap.insert(std::make_pair(layer->second, TLayerShapes{})).first->second
                                .emplace_back(std::make_pair(instShape->isVoid, std::move(eShape)));
            }
            else {
                GENERIC_ASSERT(false)
            }
        }
        for(auto & tLayerShapesPair : tLayerShapesMap) {
            auto layerId = tLayerShapesPair.first;
            auto & tLayerShapes = tLayerShapesPair.second;

            bool hasHole = false;
            for(auto & tShape : tLayerShapes) {
                if(tShape.first) {
                    hasHole = true;
                    break;
                }
            }

            if(hasHole) {
                using RtValue = std::pair<Box2D<ECoord>, size_t>;
                using Rtree = boost::geometry::index::rtree<RtValue, boost::geometry::index::dynamic_rstar>;

                Rtree tree(boost::geometry::index::dynamic_rstar(16));
                for(size_t i = 0; i < tLayerShapes.size(); ++i) {
                    if(tLayerShapes[i].first) continue;//skip holes
                    tree.insert(std::make_pair(tLayerShapes[i].second->GetBBox(), i));
                }

                std::vector<std::set<size_t> > idxPwh(tLayerShapes.size());
                std::vector<Polygon2D<ECoord> > contours(tLayerShapes.size());
                for(size_t i = 0; i < tLayerShapes.size(); ++i) {
                    contours[i] = tLayerShapes[i].second->GetContour();
                    if(!tLayerShapes[i].first) continue;//skip solids
                    
                    auto hPoint = contours[i].Front();
                    Box2D<ECoord> searchBox(hPoint, hPoint);
                    std::vector<RtValue> results;
                    tree.query(boost::geometry::index::intersects(searchBox), std::back_inserter(results));
                    for(const auto & result : results) {
                        if(Contains(contours[result.second], hPoint)){
                            idxPwh[result.second].insert(i);
                            break;
                        }
                    }
                }

                for(size_t i = 0; i < idxPwh.size(); ++i) {
                    if(tLayerShapes[i].first) continue;//skip holes
                    if(idxPwh[i].empty()) {
                        auto ePrim = mgr.CreateGeometry2D(layout, layerId, netId, std::move(tLayerShapes[i].second));
                        GENERIC_ASSERT(ePrim != nullptr)
                    }
                    else {
                        auto ePwh = std::make_unique<EPolygonWithHoles>();
                        ePwh->shape.outline = std::move(contours[i]);
                        for(auto index : idxPwh[i]) {
                            ePwh->shape.holes.emplace_back(std::move(contours[i]));
                        }
                        auto ePrim = mgr.CreateGeometry2D(layout, layerId, netId, std::move(ePwh));
                        GENERIC_ASSERT(ePrim != nullptr)
                    }
                }
            }
            else {
                for(auto & tShape : tLayerShapes) {
                    auto ePrim = mgr.CreateGeometry2D(layout, layerId, netId, std::move(tShape.second));
                    GENERIC_ASSERT(ePrim != nullptr)
                }
            }
        }
    }
}

ECAD_INLINE void ECadExtXflHandler::ImportBoardGeom(Ptr<ILayoutView> layout)
{
    auto & mgr = EDataMgr::Instance();
    EShapeGetter eShapeGetter(m_scale, m_circleDiv);
    auto toEPoint2D = [this](const Point & p) { return EPoint2D(m_scale * p.x, m_scale * p.y); };

    auto eShape = UPtr<EShape>();
    if(auto * polygon = boost::get<Polygon>(&(m_xflDB->boardGeom))) {
        eShape = eShapeGetter(*polygon);
    }
    else if(auto * composite = boost::get<Composite>(&(m_xflDB->boardGeom))) {
        eShape = eShapeGetter(*composite);
    }
    else if(auto * boardShape = boost::get<BoardShape>(&(m_xflDB->boardGeom))) {
        auto temp = m_xflDB->GetTemplateShape(boardShape->shapeId);
        if(nullptr == temp) {
            //todo, error handle
            return;
        }

        auto eShape = boost::apply_visitor(eShapeGetter, temp->shape);
        
        EMirror2D m = boardShape->mirror == 'N' ? EMirror2D::No : 
                     (boardShape->mirror == 'X' ? EMirror2D::X  : EMirror2D::Y);
        if(boardShape->rotThenMirror) {
            auto trans = makeETransform2D(1.0, math::Rad(boardShape->rot), toEPoint2D(boardShape->loc), m);
            eShape->Transform(trans);
        }
        else {
            eShape->Transform(makeETransform2D(1.0, 0.0, EPoint2D(0, 0), m));
            auto trans = makeETransform2D(1.0, math::Rad(boardShape->rot), toEPoint2D(boardShape->loc));
            eShape->Transform(trans);
        }
    }
    auto ePolygon = std::make_unique<EPolygon>();
    ePolygon->shape = eShape->GetContour();
    layout->SetBoundary(std::move(ePolygon));
}

ECAD_INLINE void ECadExtXflHandler::Reset()
{
    m_scale = 1.0;

    m_database.reset();
    m_xflDB.reset(new EXflDB);

    m_netIdMap.clear();
    m_matNameMap.clear();
    m_layerIdMap.clear();
    m_metalLyrIdMap.clear();
    m_padstackInstNames.clear();
}

ECAD_INLINE std::string ECadExtXflHandler::GetNextPadstackInstName(const std::string & defName)
{
    size_t index = 1;
    while(true) {
        std::string instName = defName + "_inst_" + std::to_string(index);
        if(!m_padstackInstNames.count(instName)){
            m_padstackInstNames.insert(instName);
            return instName;
        }
        index++;
    }
    return "";
}

}//namespace xfl
}//namespace ext
}//namespace ecad