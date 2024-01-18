#include "ECadExtGdsHandler.h"

#include "extension/gds/EGdsLayerMap.h"
#include "extension/gds/EGdsFileIO.h"
#include "generic/geometry/Utility.hpp"
#include "generic/tools/Format.hpp"
#include "ETransform.h"
#include "ELayerMap.h"
#include "EDataMgr.h"
namespace ecad {
namespace ext {
namespace gds {

namespace fmt = generic::fmt;

ECAD_INLINE ECadExtGdsHandler::ECadExtGdsHandler(const std::string & gdsFile, const std::string & lyrMapFile)
 : m_gdsFile(gdsFile), m_lyrMapFile(lyrMapFile){}

ECAD_INLINE Ptr<IDatabase> ECadExtGdsHandler::CreateDatabase(const std::string & name, Ptr<std::string> err)
{
    EGdsDB db;
    EGdsReader reader(db);
    if (not reader(m_gdsFile)) return nullptr;

    auto & eMgr = EDataMgr::Instance();
    if (eMgr.OpenDatabase(name)){
        if(err) *err = fmt::Fmt2Str("Error: database %1% is already exist.", name);
        return nullptr;
    }

    //reset temporary data
    Reset();
    m_database= eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    //set units
    m_database->SetCoordUnits(db.coordUnits);

    //import layers
    int id = 0;
    std::vector<UPtr<ILayer> > layers;
    if(!m_lyrMapFile.empty()){
        EGdsLayerMap layerMap;
        bool res = EGdsLayerMapParser(layerMap)(m_lyrMapFile);
        if(!res){
            if(err) *err = fmt::Fmt2Str("Error: failed to parse layer map file %1%.", m_lyrMapFile);
            return nullptr;
        }
        EFloat elevation = 0.0;
        auto gdsLayers = layerMap.GetAllLayers();
        for(const auto & gdsLayer  : gdsLayers){
            auto layer = eMgr.CreateStackupLayer(gdsLayer.name, gdsLayer.type, elevation, gdsLayer.thickness);
            m_layerIdMap.insert(std::make_pair(gdsLayer.layerId, std::set<ELayerId>{})).first->second.insert(static_cast<ELayerId>(id++));
            layers.push_back(std::move(layer));
            elevation -= gdsLayer.thickness;
        }
    }
    else{
        for(const auto & lyr : db.Layers()){
            std::string name = "layer_" + std::to_string(lyr);
            auto layer = eMgr.CreateStackupLayer(name, ELayerType::ConductingLayer, 0, 0);
            m_layerIdMap.insert(std::make_pair(lyr, std::set<ELayerId>{})).first->second.insert(static_cast<ELayerId>(id++));
            layers.push_back(std::move(layer));
        }
    }

    //import cells
    for(const auto & cell : db.cells){
        auto iCell = eMgr.CreateCircuitCell(m_database, cell.name);
        iCell->GetLayoutView()->AppendLayers(CloneHelper(layers));
        //todo duplicate error
        ImportOneCell(cell, iCell);
    }

    //import cell's reference, need import cells firstly
    for(const auto & cell : db.cells){
        auto iCell = eMgr.FindCellByName(m_database, cell.name);
        //todo notfind error
        ImportCellReferences(cell, iCell);
    }
    return m_database;
}

ECAD_INLINE void ECadExtGdsHandler::ImportOneCell(const EGdsCell & cell, Ptr<ICell> iCell)
{
    EDataMgr::Instance();
    auto iLayoutView = iCell->GetLayoutView();

    for(const auto & object : cell.objects){
        switch(object.first){
            case EGdsRecords::BOUNDARY : {
                auto polygon = dynamic_cast<Ptr<EGdsPolygon> >(object.second.get());
                ImportOnePolygon(polygon, iLayoutView);
                break;
            }
            case EGdsRecords::PATH : {
                auto path = dynamic_cast<Ptr<EGdsPath> >(object.second.get());
                ImportOnePath(path, iLayoutView);
                break;
            }
            case EGdsRecords::TEXT : {
                auto text = dynamic_cast<Ptr<EGdsText> >(object.second.get());
                ImportOneText(text, iLayoutView);
                break;
            }
            default : break;
        } 
    }
}

ECAD_INLINE void ECadExtGdsHandler::ImportOnePolygon(CPtr<EGdsPolygon> polygon, Ptr<ILayoutView> iLayoutView)
{
    if(nullptr == polygon) return;

    if(!m_layerIdMap.count(polygon->layer)) return;
    auto eShape = UPtr<EShape>(new EPolygon(std::move(polygon->shape)));

    auto & eMgr = EDataMgr::Instance();
    const auto & eLyrIds = m_layerIdMap.at(polygon->layer);
    
    if(eLyrIds.size() > 1){
        auto eTShape = eMgr.CreateShapeFromTemplate(std::move(eShape));
        for(auto eLyrId : eLyrIds)
            eMgr.CreateGeometry2D(iLayoutView, eLyrId, noNet, eTShape->Clone());
    }
    else eMgr.CreateGeometry2D(iLayoutView, *(eLyrIds.begin()), noNet, std::move(eShape));
}

ECAD_INLINE void ECadExtGdsHandler::ImportOnePath(CPtr<EGdsPath> path, Ptr<ILayoutView> iLayoutView)
{
    if(nullptr == path) return;

    if(!m_layerIdMap.count(path->layer)) return;
    auto eShape = UPtr<EShape>(new EPath(std::move(path->shape)));

    auto & eMgr = EDataMgr::Instance();
    const auto & eLyrIds = m_layerIdMap.at(path->layer);

    if(eLyrIds.size() > 1){
        auto eTShape = eMgr.CreateShapeFromTemplate(std::move(eShape));
        for(auto eLyrId : eLyrIds)
            eMgr.CreateGeometry2D(iLayoutView, eLyrId, noNet, eTShape->Clone());
    }
    else eMgr.CreateGeometry2D(iLayoutView, *(eLyrIds.begin()), noNet, std::move(eShape));
}

ECAD_INLINE void ECadExtGdsHandler::ImportOneText(CPtr<EGdsText> text, Ptr<ILayoutView> iLayoutView)
{
    if(nullptr == text) return;

    if(!m_layerIdMap.count(text->layer)) return;

    auto & eMgr = EDataMgr::Instance();
    const auto & eLyrIds = m_layerIdMap.at(text->layer);
    auto transform = makeETransform2D(text->scale, text->rotation, text->position);
    
    for(auto eLyrId : eLyrIds) {
        [[maybe_unused]] auto iText = eMgr.CreateText(iLayoutView, eLyrId, transform, text->text);
        //todo other paras
    }
}

ECAD_INLINE void ECadExtGdsHandler::ImportCellReferences(const EGdsCell & cell, Ptr<ICell> iCell)
{
    EDataMgr::Instance();
    auto iLayoutView = iCell->GetLayoutView();

    for(const auto & object : cell.objects){
        switch(object.first){
            case EGdsRecords::SREF : {
                auto ref = dynamic_cast<Ptr<EGdsCellReference> >(object.second.get());
                ImportOneCellReference(ref, iLayoutView);
                break;
            }
            case EGdsRecords::AREF : {
                auto arr = dynamic_cast<Ptr<EGdsCellRefArray> >(object.second.get());
                ImportOneCellRefArray(arr, iLayoutView);
                break;
            }
            default : break;
        } 
    }   
}

ECAD_INLINE void ECadExtGdsHandler::ImportOneCellReference(CPtr<EGdsCellReference> ref, Ptr<ILayoutView> iLayoutView)
{
   if(nullptr == ref) return;
    auto & eMgr = EDataMgr::Instance();
    auto iCellDef = eMgr.FindCellByName(m_database, ref->refCell);
    if(nullptr == iCellDef) return;//todo, error report

    auto iLayoutViewDef = iCellDef->GetLayoutView();
    if(nullptr == iLayoutViewDef) return;//todo, error report;

    auto name = ref->refCell + "_inst";
    auto transform = makeETransform2D(ref->scale, ref->rotation, ref->position);
    eMgr.CreateCellInst(iLayoutView, name, iLayoutViewDef, transform);
}

ECAD_INLINE void ECadExtGdsHandler::ImportOneCellRefArray(CPtr<EGdsCellRefArray> arr, Ptr<ILayoutView> iLayoutView)
{
    if(nullptr == arr) return;
    auto & eMgr = EDataMgr::Instance();
    auto iCellDef = eMgr.FindCellByName(m_database, arr->refCell);
    if(nullptr == iCellDef) return;//todo, error report

    auto iLayoutViewDef = iCellDef->GetLayoutView();
    if(nullptr == iLayoutViewDef) return;//todo, error report;

    for(size_t i = 0; i < arr->positions.size(); ++i){
        auto name = arr->refCell + "_inst_" + std::to_string(i);
        auto transform = makeETransform2D(arr->scale, arr->rotation, arr->positions[i]);
        eMgr.CreateCellInst(iLayoutView, name, iLayoutViewDef, transform);
    }
}

ECAD_INLINE void ECadExtGdsHandler::Reset()
{
    m_database = nullptr;
    m_layerIdMap.clear();
}

}//namespace gds
}//namespace ext
}//namespace ecad