#ifndef ECAD_HEADER_ONLY
#include "ECadExtGdsHandler.h"
#endif

#include "extension/gds/EGdsFileIO.h"
#include "generic/geometry/Utility.hpp"
#include "generic/tools/Format.hpp"
#include "ETransform.h"
#include "Interface.h"
#include "ELayerMap.h"
#include "EDataMgr.h"
namespace ecad {
namespace ext {
namespace gds {

namespace fmt = generic::format;

ECAD_INLINE ECadExtGdsHandler::ECadExtGdsHandler(const std::string & gdsFile)
 : m_gdsFile(gdsFile){}

ECAD_INLINE SPtr<IDatabase> ECadExtGdsHandler::CreateDatabase(const std::string & name, Ptr<std::string> err)
{
    EGdsDB db;
    EGdsReader reader(db);
    if(!reader(m_gdsFile)) return nullptr;

    auto & eMgr = EDataMgr::Instance();
    if(eMgr.OpenDatabase(name)){
        if(err) *err = fmt::Format2String("Error: database %1% is already exist.", name);
        return nullptr;
    }

    //reset temporary data
    Reset();
    m_database = eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    //set units
    m_database->SetCoordUnits(db.coordUnits);

    //import layers
    int id = 0;
    m_layerIdMap.clear();
    std::vector<UPtr<ILayer> > layers;
    for(const auto & lyr : db.Layers()){
        std::string name = "layer_" + std::to_string(lyr);
        auto layer = eMgr.CreateStackupLayer(name, ELayerType::ConductingLayer, 0, 0);
        layers.push_back(std::move(layer));
        m_layerIdMap.insert(std::make_pair(lyr, static_cast<ELayerId>(id++)));
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
    auto & eMgr = EDataMgr::Instance();
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

ECAD_INLINE void ECadExtGdsHandler::ImportOneLayer(EGdsObject::LayerId id)
{
    auto & eMgr = EDataMgr::Instance();
    auto eLyrId = static_cast<ELayerId>(id); 
    m_layerIdMap.insert(std::make_pair(id, eLyrId));
}

ECAD_INLINE void ECadExtGdsHandler::ImportOnePolygon(CPtr<EGdsPolygon> polygon, Ptr<ILayoutView> iLayoutView)
{
    if(nullptr == polygon) return;

    if(!m_layerIdMap.count(polygon->layer))
        ImportOneLayer(polygon->layer);

    auto & eMgr = EDataMgr::Instance();
    auto eLyrId = m_layerIdMap.at(polygon->layer);
    auto eShape = UPtr<EShape>(new EPolygon(std::move(polygon->shape)));
    eMgr.CreateGeometry2D(iLayoutView, eLyrId, noNet, std::move(eShape));
}

ECAD_INLINE void ECadExtGdsHandler::ImportOnePath(CPtr<EGdsPath> path, Ptr<ILayoutView> iLayoutView)
{
    if(nullptr == path) return;

    if(!m_layerIdMap.count(path->layer))
        ImportOneLayer(path->layer);

    auto & eMgr = EDataMgr::Instance();
    auto eLyrId = m_layerIdMap.at(path->layer);
    auto eShape = UPtr<EShape>(new EPath(std::move(path->shape)));
    eMgr.CreateGeometry2D(iLayoutView, eLyrId, noNet, std::move(eShape));
}

ECAD_INLINE void ECadExtGdsHandler::ImportOneText(CPtr<EGdsText> text, Ptr<ILayoutView> iLayoutView)
{
    if(nullptr == text) return;

    if(!m_layerIdMap.count(text->layer))
        ImportOneLayer(text->layer);
    
    auto & eMgr = EDataMgr::Instance();
    auto eLyrId = m_layerIdMap.at(text->layer);
    auto transform = makeETransform2D(text->scale, text->rotation, text->position);
    auto iText = eMgr.CreateText(iLayoutView, eLyrId, transform, text->text);
    //todo other paras
}

ECAD_INLINE void ECadExtGdsHandler::ImportCellReferences(const EGdsCell & cell, Ptr<ICell> iCell)
{
    auto & eMgr = EDataMgr::Instance();
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
    m_database.reset();
    m_layerIdMap.clear();
}

}//namespace gds
}//namespace ext
}//namespace ecad