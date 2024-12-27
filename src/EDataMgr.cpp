#include "EDataMgr.h"
#include "extension/ECadExtension.h"
#include "design/EPadstackDefData.h"
#include "design/EComponentDef.h"
#include "design/EMaterialProp.h"
#include "design/ELayoutView.h"
#include "design/EDatabase.h"
#include "design/ELayerMap.h"
#include "design/ELayer.h"
#include "design/ECell.h"
#include "basic/EShape.h"

#include <boost/stacktrace.hpp>
#include <csignal>

namespace ecad {

void SignalHandler(int signum)
{
    ::signal(signum, SIG_DFL);
    std::cout << boost::stacktrace::stacktrace();
    ::raise(SIGABRT);
}

EDataMgr::EDataMgr()
{
    static std::once_flag flag;
    std::call_once(flag, []{
        ::signal(SIGSEGV, &SignalHandler);
        ::signal(SIGABRT, &SignalHandler);
    });
}

EDataMgr::~EDataMgr()
{
}

Ptr<IDatabase> EDataMgr::CreateDatabase(const std::string & name)
{
    if (m_databases.count(name)) return nullptr;

    auto database = std::make_shared<EDatabase>(name);
    return m_databases.emplace(name, database).first->second.get();
}

Ptr<IDatabase> EDataMgr::OpenDatabase(const std::string & name)
{
    auto iter = m_databases.find(name);
    if (iter == m_databases.cend()) return nullptr;
    return iter->second.get();
}

bool EDataMgr::RemoveDatabase(const std::string & name)
{
    return m_databases.erase(std::string(name)) > 0;
}

void EDataMgr::ShutDown(bool autoSave)
{
    //todo
    m_databases.clear();

    log::ShutDown();
}

Ptr<IDatabase> EDataMgr::CreateDatabaseFromGds(const std::string & name, const std::string & gds, const std::string & lyrMap)
{
    return ext::CreateDatabaseFromGds(name, gds, lyrMap);
}

Ptr<IDatabase> EDataMgr::CreateDatabaseFromKiCad(const std::string & name, const std::string & kicad)
{
    return ext::CreateDatabaseFromKiCad(name, kicad);
}

Ptr<IDatabase> EDataMgr::CreateDatabaseFromXfl(const std::string & name, const std::string & xfl)
{
    return ext::CreateDatabaseFromXfl(name, xfl);
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
bool EDataMgr::SaveDatabase(CPtr<IDatabase> database, const std::string & archive, EArchiveFormat fmt)
{
    ECAD_ASSERT(database)
    return database->Save(archive, fmt);
}
    
Ptr<IDatabase> EDataMgr::LoadDatabase(const std::string & archive, EArchiveFormat fmt)
{
    auto database = std::make_shared<EDatabase>("");
    if (not database->Load(archive, fmt)) return nullptr;
    if (m_databases.count(database->GetName())) return nullptr;
    return m_databases.emplace(database->GetName(), database).first->second.get();
}
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

Ptr<ICell> EDataMgr::CreateCircuitCell(Ptr<IDatabase> database, const std::string & name)
{
    if(nullptr == database) return nullptr;
    return database->CreateCircuitCell(name);
}

Ptr<ICell> EDataMgr::FindCellByName(CPtr<IDatabase> database, const std::string & name)
{
    if(nullptr == database) return nullptr;
    return database->FindCellByName(name);
}

Ptr<INet> EDataMgr::CreateNet(Ptr<ILayoutView> layout, const std::string & name)
{
    if(nullptr == layout) return nullptr;
    return layout->CreateNet(name);
}

Ptr<INet> EDataMgr::FindNetByName(Ptr<ILayoutView> layout, const std::string & name) const
{
    if(nullptr == layout) return nullptr;
    return layout->FindNetByName(name);
}

UPtr<ILayer> EDataMgr::CreateStackupLayer(const std::string & name, ELayerType type, EFloat elevation, EFloat thickness,
                                                      const std::string & conductingMat, const std::string & dielectricMat)
{
    auto stackupLayer = new EStackupLayer(name, type);
    stackupLayer->SetElevation(elevation);
    stackupLayer->SetThickness(thickness);
    stackupLayer->SetConductingMaterial(conductingMat);
    stackupLayer->SetDielectricMaterial(dielectricMat);
    return UPtr<ILayer>(stackupLayer);
}

Ptr<IComponentDef> EDataMgr::CreateComponentDef(Ptr<IDatabase> database, const std::string & name)
{
    if(nullptr == database) return nullptr;
    return database->CreateComponentDef(name);
}

Ptr<IComponentDef> EDataMgr::FindComponentDefByName(CPtr<IDatabase> database, const std::string & name)
{
    if(nullptr == database) return nullptr;
    return database->FindComponentDefByName(name);
}

Ptr<IMaterialDef> EDataMgr::CreateMaterialDef(Ptr<IDatabase> database, const std::string & name)
{
    if(nullptr == database) return nullptr;
    return database->CreateMaterialDef(name);
}

Ptr<IMaterialDef> EDataMgr::FindMaterialDefByName(CPtr<IDatabase> database, const std::string & name)
{
    if(nullptr == database) return nullptr;
    return database->FindMaterialDefByName(name);
}

UPtr<IMaterialProp> EDataMgr::CreateSimpleMaterialProp(EFloat value)
{
    return UPtr<IMaterialProp>(new EMaterialPropValue(value));
}

UPtr<IMaterialProp> EDataMgr::CreateAnisotropicMaterialProp(const std::array<EFloat, 3> & values)
{
    return UPtr<IMaterialProp>(new EMaterialPropValue(values));
}

UPtr<IMaterialProp> EDataMgr::CreateTensorMateriaProp(const std::array<EFloat, 9> & values)
{
    return UPtr<IMaterialProp>(new EMaterialPropValue(values));
}

UPtr<IMaterialProp> EDataMgr::CreatePolynomialMaterialProp(std::vector<std::vector<EFloat>> coefficients)
{
    return UPtr<IMaterialProp>(new EMaterialPropPolynomial(std::move(coefficients)));
}

Ptr<ILayerMap> EDataMgr::CreateLayerMap(Ptr<IDatabase> database, const std::string & name)
{
    if(nullptr == database) return nullptr;
    return database->CreateLayerMap(name);
}

Ptr<ILayerMap> EDataMgr::FindLayerMapByName(CPtr<IDatabase> database, const std::string & name)
{
    if(nullptr == database) return nullptr;
    return database->FindLayerMapByName(name);
}

Ptr<IPadstackDef> EDataMgr::CreatePadstackDef(Ptr<IDatabase> database, const std::string & name)
{
    if(nullptr == database) return nullptr;
    return database->CreatePadstackDef(name);
}

Ptr<IPadstackDef> EDataMgr::FindPadstackDefByName(CPtr<IDatabase> database, const std::string & name) const
{
    if(nullptr == database) return nullptr;
    return database->FindPadstackDefByName(name);
}

UPtr<IPadstackDefData> EDataMgr::CreatePadstackDefData()
{
    auto defData = new EPadstackDefData;
    return UPtr<IPadstackDefData>(defData);
}

Ptr<IPadstackInst> EDataMgr::CreatePadstackInst(Ptr<ILayoutView> layout, const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                                            ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                                            const ETransform2D & transform)
{
    if (nullptr == layout) return nullptr;
    return layout->CreatePadstackInst(name, def, net, topLyr, botLyr, layerMap, transform);
}

Ptr<ICellInst> EDataMgr::CreateCellInst(Ptr<ILayoutView> layout, const std::string & name,
                                                    Ptr<ILayoutView> defLayout, const ETransform2D & transform)
{
    if (nullptr == layout) return nullptr;
    return layout->CreateCellInst(name, defLayout, transform);
}

Ptr<IComponent> EDataMgr::CreateComponent(Ptr<ILayoutView> layout, const std::string & name,
                                                     CPtr<IComponentDef> compDef, ELayerId layer, const ETransform2D & transform, bool flipped)
{
    if (nullptr == layout) return nullptr;
    return layout->CreateComponent(name, compDef, layer, transform, flipped);
}

bool EDataMgr::GetComponentPinLocation(CPtr<IComponent> component, const std::string & name, FPoint2D & location) const
{
    if (nullptr == component) return false;
    if (EPoint2D p; not component->GetPinLocation(name, p)) return false;
    else location = component->GetComponentDef()->GetDatabase()->GetCoordUnits().toUnit(p);
    return true;
}

Ptr<IPrimitive> EDataMgr::CreateGeometry2D(Ptr<ILayoutView> layout, ELayerId layer, ENetId net, UPtr<EShape> shape)
{
    if (nullptr == layout) return nullptr;
    return layout->CreateGeometry2D(layer, net, std::move(shape));
}

Ptr<IBondwire> EDataMgr::CreateBondwire(Ptr<ILayoutView> layout, std::string name, ENetId net, EFloat radius)
{
    if (nullptr == layout) return nullptr;
    return layout->CreateBondwire(std::move(name), net, radius);
}

UPtr<EShape> EDataMgr::CreateShapeRectangle(const ECoordUnits & coordUnits, const FPoint2D & ll, const FPoint2D & ur)
{
    return CreateShapeRectangle(coordUnits.toCoord(ll), coordUnits.toCoord(ur));
}

UPtr<EShape> EDataMgr::CreateShapeCircle(const ECoordUnits & coordUnits, const FPoint2D & loc, EFloat radius)
{
    return CreateShapeCircle(coordUnits.toCoord(loc), coordUnits.toCoord(radius));
}

UPtr<EShape> EDataMgr::CreateShapePath(const ECoordUnits & coordUnits, const std::vector<FPoint2D> & points, EFloat width)
{
    return CreateShapePath(coordUnits.toCoord(points), coordUnits.toCoord(width));
}

UPtr<EShape> EDataMgr::CreateShapePolygon(const ECoordUnits & coordUnits, const std::vector<FPoint2D> & points, EFloat cornerR)
{
    if (cornerR > 0) {
        generic::geometry::Polygon2D<EFloat> polygon; polygon.Set(points);
        polygon = generic::geometry::RoundCorners(polygon, cornerR, CircleDiv());
        return CreateShapePolygon(coordUnits.toCoord(polygon.GetPoints()));
    }
    return CreateShapePolygon(coordUnits.toCoord(points));
}

UPtr<EShape> EDataMgr::CreateShapePolygon(const ECoordUnits & coordUnits, const std::vector<FCoord> & coords, EFloat cornerR)
{
    std::vector<FPoint2D> points(coords.size() / 2);
    for (size_t i = 0; i < points.size(); ++i)
        points[i] = FPoint2D(coords.at(i * 2), coords.at(i * 2 + 1));
    return CreateShapePolygon(coordUnits, points, cornerR);
}


UPtr<EShape> EDataMgr::CreateShapeRectangle(EPoint2D ll, EPoint2D ur)
{
    auto shape = new ERectangle(std::move(ll), std::move(ur));
    return UPtr<EShape>(shape);
}

UPtr<EShape> EDataMgr::CreateShapeCircle(EPoint2D loc, ECoord radius)
{
    auto shape = new ECircle(std::move(loc), radius, CircleDiv());
    return UPtr<EShape>(shape);
}

UPtr<EShape> EDataMgr::CreateShapePath(std::vector<EPoint2D> points, ECoord width)
{
    auto shape = new EPath;
    shape->shape = std::move(points);
    shape->SetWidth(width);
    return UPtr<EShape>(shape);
}

UPtr<EShape> EDataMgr::CreateShapePolygon(std::vector<EPoint2D> points)
{
    EPolygonData polygon;
    polygon.Set(std::move(points));
    return CreateShapePolygon(polygon);
}

UPtr<EShape> EDataMgr::CreateShapePolygon(EPolygonData polygon)
{
    auto shape = new EPolygon;
    shape->shape = std::move(polygon);
    return UPtr<EShape>(shape);
}

UPtr<EShape> EDataMgr::CreateShapePolygonWithHoles(EPolygonWithHolesData pwh)
{
    auto shape = new EPolygonWithHoles;
    shape->shape = std::move(pwh);
    return UPtr<EShape>(shape);
}

UPtr<EShape> EDataMgr::CreateShapeFromTemplate(ETemplateShape ts, ETransform2D trans)
{
    auto shape = new EShapeFromTemplate(ts, trans);
    return UPtr<EShape>(shape);
}

EPolygon EDataMgr::CreatePolygon(const ECoordUnits & coordUnits, const std::vector<FPoint2D> & points)
{
    EPolygon polygon;
    polygon.shape.Set(coordUnits.toCoord(points));
    return polygon;
}
    
EBox2D EDataMgr::CreateBox(const ECoordUnits & coordUnits, const FPoint2D & ll, const FPoint2D & ur)
{
    return EBox2D(coordUnits.toCoord(ll), coordUnits.toCoord(ur));
}

ETransform2D EDataMgr::CreateTransform2D(const ECoordUnits & coordUnits, EFloat scale, EFloat rotation, const FVector2D & offset, EMirror2D mirror)
{
    return makeETransform2D(scale, rotation, coordUnits.toCoord(offset), mirror);
}

Ptr<IText> EDataMgr::CreateText(Ptr<ILayoutView> layout, ELayerId layer, const ETransform2D & transform, const std::string & text)
{
    if(nullptr == layout) return nullptr;
    return layout->CreateText(layer, transform, text);
}


Ptr<IComponentDefPin> EDataMgr::CreateComponentDefPin(Ptr<IComponentDef> compDef, const std::string & pinName, FPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef, ELayerId lyr)
{
    if(nullptr == compDef) return nullptr;
    return compDef->CreatePin(pinName, compDef->GetDatabase()->GetCoordUnits().toCoord(loc), type, psDef, lyr);
}

EDataMgr & EDataMgr::Instance()
{
    static EDataMgr mgr;
    return mgr;
}

char EDataMgr::HierSep() const
{
    return m_settings.hierSep;
}

size_t EDataMgr::Threads() const
{
    return m_settings.threads;
}

void EDataMgr::SetThreads(size_t threads)
{
    m_settings.threads = threads;
}

size_t EDataMgr::CircleDiv() const
{
    return m_settings.circleDiv;
}

void EDataMgr::Init(ELogLevel level, const std::string & workDir)
{   
    //threads
    m_settings.threads = std::thread::hardware_concurrency();
#ifdef ECAD_OPEN_MP_SUPPORT
    Eigen::setNbThreads(m_settings.threads);
#endif//ECAD_OPEN_MP_SUPPORT

    //log
    std::string logFile = workDir.empty() ? generic::fs::CurrentPath().string() : workDir + ECAD_SEPS + "ecad.log";
    auto logger = workDir.empty() ? log::OstreamLoggerMT("ecad", std::cout) : 
                log::BasicLoggerMT("ecad", workDir + ECAD_SEPS + "ecad.log");
    logger->SetLevel(level);
    log::SetDefaultLogger(logger);
}
}//namespace ecad