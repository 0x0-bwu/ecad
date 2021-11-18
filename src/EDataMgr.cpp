#ifndef ECAD_HEADER_ONLY
#include "EDataMgr.h"
#endif

#include "EPadstackDefData.h"
#include "ELayoutView.h"
#include "EDatabase.h"
#include "ELayerMap.h"
#include "ELayer.h"
#include "EShape.h"
#include "ECell.h"
namespace ecad {

ECAD_INLINE EDataMgr::EDataMgr()
{
}

ECAD_INLINE EDataMgr::~EDataMgr()
{
}

ECAD_INLINE SPtr<IDatabase> EDataMgr::CreateDatabase(const std::string & name)
{
    std::lock_guard<std::mutex> lock(m_databaseMutex);
    if(m_databases.count(name)) return nullptr;

    auto database = std::make_shared<EDatabase>(name);
    m_databases.insert(std::make_pair(name, database));
    return database;
}

ECAD_INLINE SPtr<IDatabase> EDataMgr::OpenDatabase(const std::string & name)
{
    std::lock_guard<std::mutex> lock(m_databaseMutex);
    if(!m_databases.count(name)) return nullptr;

    return m_databases[name];
}

ECAD_INLINE bool EDataMgr::RemoveDatabase(const std::string & name)
{
    std::lock_guard<std::mutex> lock(m_databaseMutex);
    return m_databases.erase(name) > 0;
}

ECAD_INLINE void EDataMgr::ShutDown(bool autoSave)
{
    std::lock_guard<std::mutex> lock(m_databaseMutex);
    //todo
    m_databases.clear();
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
ECAD_INLINE bool EDataMgr::SaveDatabase(SPtr<IDatabase> database, const std::string & archive, EArchiveFormat fmt)
{
    return database->Save(archive, fmt);
}
    
ECAD_INLINE bool EDataMgr::LoadDatabase(SPtr<IDatabase> database, const std::string & archive, EArchiveFormat fmt)
{
    return database->Load(archive, fmt);
}
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE Ptr<ICell> EDataMgr::CreateCircuitCell(SPtr<IDatabase> database, const std::string & name)
{
    return database->CreateCircuitCell(name);
}

ECAD_INLINE Ptr<ICell> EDataMgr::FindCellByName(SPtr<IDatabase> database, const std::string & name)
{
    return database->FindCellByName(name);
}

ECAD_INLINE Ptr<INet> EDataMgr::CreateNet(Ptr<ILayoutView> layout, const std::string & name)
{
    return layout->CreateNet(name);
}

ECAD_INLINE UPtr<ILayer> EDataMgr::CreateStackupLayer(const std::string & name, ELayerType type, FCoord elevation, FCoord thickness)
{
    auto stackupLayer = new EStackupLayer(name, type);
    stackupLayer->SetElevation(elevation);
    stackupLayer->SetThickness(thickness);
    return UPtr<ILayer>(stackupLayer);
}

ECAD_INLINE Ptr<ILayerMap> EDataMgr::CreateLayerMap(SPtr<IDatabase> database, const std::string & name)
{
    return database->CreateLayerMap(name);
}

ECAD_INLINE Ptr<IPadstackDef> EDataMgr::CreatePadstackDef(Ptr<IDatabase> database, const std::string & name)
{
    return database->CreatePadstackDef(name);
}

ECAD_INLINE UPtr<IPadstackDefData> EDataMgr::CreatePadstackDefData()
{
    auto defData = new EPadstackDefData;
    return UPtr<IPadstackDefData>(defData);
}

ECAD_INLINE Ptr<IPadstackInst> EDataMgr::CreatePadstackInst(Ptr<ILayoutView> layout, const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                                            ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                                            const ETransform2D & transform)
{
    return layout->CreatePadstackInst(name, def, net, topLyr, botLyr, layerMap, transform);
}

ECAD_INLINE Ptr<ICellInst> EDataMgr::CreateCellInst(Ptr<ILayoutView> layout, const std::string & name,
                                                    Ptr<ILayoutView> defLayout, const ETransform2D & transform)
{
    return layout->CreateCellInst(name, defLayout, transform);
}

ECAD_INLINE Ptr<IPrimitive> EDataMgr::CreateGeometry2D(Ptr<ILayoutView> layout, ELayerId layer, ENetId net, UPtr<EShape> shape)
{
    return layout->CreateGeometry2D(layer, net, std::move(shape));
}

ECAD_INLINE UPtr<EShape> EDataMgr::CreateShapePolygon(std::vector<Point2D<ECoord> > points)
{
    Polygon2D<ECoord> polygon;
    polygon.Set(std::move(points));
    return CreateShapePolygon(polygon);
}

ECAD_INLINE UPtr<EShape> EDataMgr::CreateShapePolygon(Polygon2D<ECoord> polygon)
{
    auto shape = new EPolygon;
    shape->shape = std::move(polygon);
    return UPtr<EShape>(shape);
}

ECAD_INLINE UPtr<EShape> EDataMgr::CreateShapePolygonWithHoles(PolygonWithHoles2D<ECoord> pwh)
{
    auto shape = new EPolygonWithHoles;
    shape->shape = std::move(pwh);
    return UPtr<EShape>(shape);
}

ECAD_INLINE Ptr<IText> EDataMgr::CreateText(Ptr<ILayoutView> layout, ELayerId layer, const ETransform2D & transform, const std::string & text)
{
    return layout->CreateText(layer, transform, text);
}

ECAD_INLINE EDataMgr & EDataMgr::Instance()
{
    static EDataMgr mgr;
    return mgr;
}



}//namespace ecad