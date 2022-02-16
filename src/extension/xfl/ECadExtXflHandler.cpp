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

ECAD_INLINE ECadExtXflHandler::ECadExtXflHandler(const std::string & xflFile)
 : m_xflFile(xflFile){}

ECAD_INLINE SPtr<IDatabase> ECadExtXflHandler::CreateDatabase(const std::string & name, std::string * err)
{
    auto & eMgr = EDataMgr::Instance();
    if(eMgr.OpenDatabase(name)){
        if(err) *err = fmt::Format2String("Error: database %1% is already exist.", name);
        return nullptr;
    }

    Reset();

    EXflReader reader(*m_xflDB);
    if(!reader(m_xflFile)) return nullptr;

    m_xflDB->BuildLUTs();

    //reset temporary data
    m_database = eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    //set units
    ECoordUnits coordUnits(m_xflDB->unit);
    m_database->SetCoordUnits(coordUnits);
    m_scale2Coord = coordUnits.Scale2Coord() / m_xflDB->scale;

    //import padstack def
    ImportPadstackDefs();

    //create top cell
    auto iCell = eMgr.CreateCircuitCell(m_database, name);
    auto iLayout = iCell->GetLayoutView();

    //import layers
    ImportLayers(iLayout);

    //import nets
    ImportNets(iLayout);

    return m_database;
}

ECAD_INLINE void ECadExtXflHandler::ImportPadstackDefs()
{

}

ECAD_INLINE void ECadExtXflHandler::ImportLayers(Ptr<ILayoutView> layout)
{
    auto & eMgr = EDataMgr::Instance();

    int id = -1;
    int xflLayerId = 1;
    int xflMetalId = 1;
    FCoord elevation = 0.0;
    std::vector<UPtr<ILayer> > layers;
    for(const auto & xflLyr : m_xflDB->layers){
        id++;
        ELayerType type = xflLyr.type == 'D' ? ELayerType::DielectricLayer : ELayerType::ConductingLayer;
        auto layer = eMgr.CreateStackupLayer(xflLyr.name, type, elevation * m_scale2Coord, xflLyr.thickness * m_scale2Coord);
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
    auto & eMgr = EDataMgr::Instance();
    for(const auto & xflNet : m_xflDB->nets){
        auto net = eMgr.CreateNet(layout, xflNet.name);
        if(net == nullptr){
            //error handle
        }
        m_netIdMap.insert(std::make_pair(xflNet.name, net->GetNetId()));
    }
}

ECAD_INLINE void ECadExtXflHandler::Reset()
{
    m_scale2Coord = 1.0;

    m_database.reset();
    m_xflDB.reset(new EXflDB);

    m_netIdMap.clear();
    m_layerIdMap.clear();
    m_metalLyrIdMap.clear();
}

}//namespace xfl
}//namespace ext
}//namespace ecad