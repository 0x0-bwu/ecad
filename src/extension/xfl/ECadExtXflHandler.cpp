#include "ECadExtXflHandler.h"

#include "generic/geometry/Utility.hpp"
#include "generic/tools/Format.hpp"
#include "EMaterialProp.h"
#include "EXflParser.h"
#include "ETransform.h"
#include "EDataMgr.h"
namespace ecad {
namespace ext {
namespace xfl {

namespace fmt = generic::fmt;

ECAD_INLINE ECadExtXflHandler::ECadExtXflHandler(const std::string & xflFile, size_t circleDiv)
 : m_xflFile(xflFile), m_circleDiv(circleDiv) {}

ECAD_INLINE SPtr<IDatabase> ECadExtXflHandler::CreateDatabase(const std::string & name, std::string * err)
{
    auto & mgr = EDataMgr::Instance();
    if(mgr.OpenDatabase(name)){
        if(err) *err = fmt::Fmt2Str("Error: database %1% is already exist.", name);
        return nullptr;
    }

    Reset();

    EXflReader reader(*m_xflDB);
    if (not reader(m_xflFile)) {
        if (err) *err = fmt::Fmt2Str("Error: failed to parse  %1%.", m_xflFile);
        return nullptr;
    }

    //reset temporary data
    m_database = mgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    //set units
    ECoordUnits coordUnits(m_xflDB->unit);
    m_database->SetCoordUnits(coordUnits);
    m_scale = coordUnits.Scale2Coord() / m_xflDB->scale;

    //build xfl look up tables
    EShapeGetter eShapeGetter(m_scale, m_circleDiv);
    m_xflDB->BuildLUTs(eShapeGetter);

    //import components
    ImportComponentDefs();

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

ECAD_INLINE void ECadExtXflHandler::ImportComponentDefs()
{
    // auto & mgr = EDataMgr::Instance();
    
    for (const auto & xflPart : m_xflDB->parts) {
        auto name = m_database->GetNextDefName(xflPart.name, EDefinitionType::ComponentDef);
        m_partNameMap.emplace(xflPart.name, name);

    }
}

ECAD_INLINE void ECadExtXflHandler::ImportMaterialDefs()
{
    auto & mgr = EDataMgr::Instance();

    for (const auto & xflMat : m_xflDB->materials) {
        auto name = m_database->GetNextDefName(xflMat.name, EDefinitionType::MaterialDef);
        auto material = mgr.CreateMaterialDef(m_database, name);
        if (nullptr == material) {
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

        auto ts = m_xflDB->GetTemplateShape(xflVia.shapeId);
        if(nullptr == ts){
            //todo, error handle
            continue;
        }

        auto psDefData = mgr.CreatePadstackDefData();

        if(!xflVia.material.empty()){
            if(!m_matNameMap.count(xflVia.material)){
                //todo, error handle
                continue;
            }
            psDefData->SetMaterial(m_matNameMap.at(xflVia.material));
        }

        auto shape = mgr.CreateShapeFromTemplate(ts);
        ECAD_ASSERT(shape != nullptr)

        psDefData->SetViaParameters(std::move(shape), EPoint2D(0, 0), math::Rad(xflVia.shapeRot));

        auto layerMap = mgr.CreateLayerMap(m_database, xflVia.name);
    
        std::vector<std::string> padLayers;
        for(size_t i = 0; i < xflPs->pads.size(); ++i)
            padLayers.emplace_back("layer_" + std::to_string(i+1));
        psDefData->SetLayers(padLayers);
        for(size_t i = 0; i < xflPs->pads.size(); ++i){
            const auto & xflPad = xflPs->pads.at(i);
            ts = m_xflDB->GetTemplateShape(xflPad.shapeId);
            if(nullptr == ts){
                //todo, error handle
                continue;
            }
            auto shape = mgr.CreateShapeFromTemplate(ts);
            ECAD_ASSERT(shape != nullptr)

            psDefData->SetPadParameters(static_cast<ELayerId>(i), std::move(shape), EPoint2D(0, 0), math::Rad(xflPad.shapeRot));
        
            //todo, anti-pad

            //layer map
            if (auto iter = m_metalLyrIdMap.find(xflPad.sigLyr); iter != m_metalLyrIdMap.cend())
                layerMap->SetMapping(static_cast<ELayerId>(i), iter->second);
        }

        auto psDef = mgr.CreatePadstackDef(m_database, xflVia.name);
        ECAD_ASSERT(psDef != nullptr)
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

    for(const auto & route : m_xflDB->routes) {
        auto net = mgr.FindNetByName(layout, route.net);
        if(nullptr == net){
            //todo, error handle
            continue;
        }
        size_t i = 0;
        auto netId = net->GetNetId();
        while(i < route.objects.size()) {
            auto & instObj = route.objects[i++];
            //inst path
            if(auto * instPath = boost::get<InstPath>(&instObj)) {
                auto layer = m_metalLyrIdMap.find(instPath->layer);
                if(layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto shape = eShapeGetter(instPath->path);
                auto polygon = dynamic_cast<Ptr<EPolygon> >(shape.get());
                if(!polygon || polygon->shape.Size() == 0){
                    //todo, error handle
                    continue;
                }
                auto ePath = mgr.CreateShapePath(polygon->shape.GetPoints(), instPath->width * m_scale);
                [[maybe_unused]] auto ePrim = mgr.CreateGeometry2D(layout, layer->second, netId, std::move(ePath));
                ECAD_ASSERT(ePrim != nullptr)
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
                if (nullptr == layerMap) {
                    //todo, error handle
                    continue;
                }

                auto psDef = mgr.FindPadstackDefByName(m_database, instVia->name);
                if (nullptr == psDef) {
                    //todo, error handle
                    continue;
                }

                const auto & m = instVia->mirror;
                EMirror2D mirror = (m == 'Y' || m == '1') ? EMirror2D::Y : EMirror2D::No;
                auto trans = makeETransform2D(1.0, math::Rad(instVia->rot), makeEPoint2D(instVia->loc), mirror);
                
                auto name = GetNextPadstackInstName(instVia->name);
                [[maybe_unused]] auto psInst = mgr.CreatePadstackInst(layout, name, psDef, netId, sLayer->second, eLayer->second, layerMap, trans);
                ECAD_ASSERT(psInst != nullptr)
            }
            //inst bondwire
            else if (auto * instBw = boost::get<InstBondwire>(&instObj); instBw) {
                //todo
                continue;
            }
            //inst annular
            else if (auto * instAnnular = boost::get<InstAnnular>(&instObj); instAnnular) {
                auto layer = m_metalLyrIdMap.find(instAnnular->layer);
                if (layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                auto shape = eShapeGetter(instAnnular->annular);
                if (!shape->isValid()) {
                    //todo, error handle
                    continue;
                }
                shape->Transform(makeETransform2D(1.0, 0.0, makeEPoint2D(instAnnular->loc)));
                [[maybe_unused]] auto ePrim = mgr.CreateGeometry2D(layout, layer->second, netId, std::move(shape));
                ECAD_ASSERT(ePrim != nullptr)
            }
            //others
            else {
                //ignore if first shape is hole
                if (isHole(instObj)) continue;
                auto [lyr, shape] = makeEShapeFromInstObject(&mgr, eShapeGetter, instObj);
                auto layer = m_metalLyrIdMap.find(lyr);
                if (layer == m_metalLyrIdMap.end()) {
                    //todo, error handle
                    continue;
                }
                if (nullptr == shape) continue;
                std::list<UPtr<EShape> > holes;
                while(i < route.objects.size()) {
                    const auto & instHole = route.objects[i++];
                    if(!isHole(instHole)) { i--; break; }
                    auto [nextLyr, nextShape] = makeEShapeFromInstObject(&mgr, eShapeGetter, instHole);
                    if(nextLyr != lyr || nullptr == nextShape) { i--; break; }
                    holes.emplace_back(std::move(nextShape));
                }
                if (holes.empty()) {
                    if(!shape->isValid()) {
                        //todo, error handle
                        continue;
                    }
                    [[maybe_unused]] auto ePrim = mgr.CreateGeometry2D(layout, layer->second, netId, std::move(shape));
                    ECAD_ASSERT(ePrim != nullptr)
                }
                else {
                    auto pwh = std::make_unique<EPolygonWithHoles>();
                    auto & data = pwh->shape;
                    data.outline = shape->GetContour();
                    for(const auto & hole : holes){
                        data.holes.emplace_back(hole->GetContour());
                    }
                    if(!pwh->isValid()) {
                        //todo, error handle
                        continue;
                    }
                    [[maybe_unused]] auto ePrim = mgr.CreateGeometry2D(layout, layer->second, netId, std::move(pwh));
                    ECAD_ASSERT(ePrim != nullptr)
                }
            }
        }
    }
}

ECAD_INLINE void ECadExtXflHandler::ImportBoardGeom(Ptr<ILayoutView> layout)
{
    if(!m_xflDB->hasBoardGeom) return;
    auto & mgr = EDataMgr::Instance();
    EShapeGetter eShapeGetter(m_scale, m_circleDiv);

    auto shape = UPtr<EShape>();
    if(auto * polygon = boost::get<Polygon>(&(m_xflDB->boardGeom))) {
        shape = eShapeGetter(*polygon);
    }
    else if(auto * composite = boost::get<Composite>(&(m_xflDB->boardGeom))) {
        shape = eShapeGetter(*composite);
    }
    else if(auto * boardShape = boost::get<BoardShape>(&(m_xflDB->boardGeom))) {
        auto ts = m_xflDB->GetTemplateShape(boardShape->shapeId);
        if(nullptr == ts) {
            //todo, error handle
            return;
        }

        auto shape = mgr.CreateShapeFromTemplate(ts);
        EMirror2D m = boardShape->mirror == 'N' ? EMirror2D::No : 
                     (boardShape->mirror == 'X' ? EMirror2D::X  : EMirror2D::Y);
        if(boardShape->rotThenMirror) {
            auto trans = makeETransform2D(1.0, math::Rad(boardShape->rot), makeEPoint2D(boardShape->loc), m);
            shape->Transform(trans);
        }
        else {
            shape->Transform(makeETransform2D(1.0, 0.0, EPoint2D(0, 0), m));
            auto trans = makeETransform2D(1.0, math::Rad(boardShape->rot), makeEPoint2D(boardShape->loc));
            shape->Transform(trans);
        }
    }
    auto ePolygon = std::make_unique<EPolygon>();
    ePolygon->shape = shape->GetContour();
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

//<layer, isVoid>
ECAD_INLINE bool ECadExtXflHandler::isHole(const InstObject & instObj) const
{
    if(auto * instPolygon = boost::get<InstPolygon>(&instObj)) return instPolygon->isVoid;
    else if(auto * instRect = boost::get<InstRectangle>(&instObj)) return instRect->isVoid;
    else if(auto * instSquare = boost::get<InstSquare>(&instObj)) return instSquare->isVoid;
    else if(auto * instDiamond = boost::get<InstDiamond>(&instObj)) return instDiamond->isVoid;
    else if(auto * instCircle = boost::get<InstCircle>(&instObj)) return instCircle->isVoid;
    else if(auto * instComp = boost::get<InstComposite>(&instObj)) return instComp->isVoid;
    else if(auto * instShape = boost::get<InstShape>(&instObj)) return instShape->isVoid;
    return false;
}

ECAD_INLINE std::pair<int, UPtr<EShape> > ECadExtXflHandler::makeEShapeFromInstObject(EDataMgr * mgr, const EShapeGetter & eShapeGetter, const InstObject & instObj) const
{
    //inst polygon
    if(auto * instPolygon = boost::get<InstPolygon>(&instObj)) {
        auto shape = eShapeGetter(instPolygon->polygon);
        return std::make_pair(instPolygon->layer, std::move(shape));
    }
    //inst rectangle
    else if(auto * instRect = boost::get<InstRectangle>(&instObj)) {
        auto shape = eShapeGetter(instRect->rectangle);
        shape->Transform(makeETransform2D(1.0, 0.0, makeEPoint2D(instRect->loc)));
        return std::make_pair(instRect->layer, std::move(shape));
    }
    //inst square
    else if(auto * instSquare = boost::get<InstSquare>(&instObj)) {
        auto shape = eShapeGetter(instSquare->square);
        shape->Transform(makeETransform2D(1.0, 0.0, makeEPoint2D(instSquare->loc)));
        return  std::make_pair(instSquare->layer, std::move(shape));
    }
    //inst diamond
    else if(auto * instDiamond = boost::get<InstDiamond>(&instObj)) {
        auto shape = eShapeGetter(instDiamond->diamond);
        shape->Transform(makeETransform2D(1.0, 0.0, makeEPoint2D(instDiamond->loc)));
        return std::make_pair(instDiamond->layer, std::move(shape));
    }
    //inst circle
    else if(auto * instCircle = boost::get<InstCircle>(&instObj)) {
        auto shape = eShapeGetter(instCircle->circle);
        shape->Transform(makeETransform2D(1.0, 0.0, makeEPoint2D(instCircle->loc)));
        return std::make_pair(instCircle->layer, std::move(shape));
    }
    //inst composite
    else if(auto * instComp = boost::get<InstComposite>(&instObj)) {
        auto shape = eShapeGetter(instComp->composite);
        return std::make_pair(instComp->layer, std::move(shape));
    }
    //inst shape from template
    else if(auto * instShape = boost::get<InstShape>(&instObj)) {
        auto ts = m_xflDB->GetTemplateShape(instShape->shapeId);
        if(nullptr == ts) return std::make_pair(instShape->layer, nullptr);

        auto shape = mgr->CreateShapeFromTemplate(ts);
        EMirror2D m = instShape->mirror == 'N' ? EMirror2D::No : 
                     (instShape->mirror == 'X' ? EMirror2D::X  : EMirror2D::Y);
        if(instShape->rotThenMirror) {
            shape->Transform(makeETransform2D(1.0, math::Rad(instShape->rot), makeEPoint2D(instShape->loc), m));
        }
        else {
            shape->Transform(makeETransform2D(1.0, 0.0, EPoint2D(0, 0), m));
            shape->Transform(makeETransform2D(1.0, math::Rad(instShape->rot), makeEPoint2D(instShape->loc)));
        }
        return std::make_pair(instShape->layer, std::move(shape));
    }
    else {
        ECAD_ASSERT(false)
        return std::make_pair(-1, nullptr);
    }
}

}//namespace xfl
}//namespace ext
}//namespace ecad