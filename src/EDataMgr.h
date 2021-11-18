#ifndef ECAD_EDATAMGR_H
#define ECAD_EDATAMGR_H
#include "geometry/Geometries.hpp"
#include "ECadCommon.h"
#include "ETransform.h"
#include "ECadDef.h"
#include <unordered_map>
#include <memory>
#include <mutex>
namespace ecad {

class INet;
class ICell;
class IText;
class ILayer;
class EShape;
class ILayerMap;
class IDatabase;
class ICellInst;
class IPrimitive;
class ILayoutView;
class IPadstackDef;
class IPadstackInst;
class IPadstackDefData;
using namespace generic::geometry;

class ECAD_API EDataMgr
{
public:
    EDataMgr(const EDataMgr &) = delete;
    EDataMgr & operator= (const EDataMgr &) = delete;

    ///Database
    SPtr<IDatabase> CreateDatabase(const std::string & name);
    SPtr<IDatabase> OpenDatabase(const std::string & name);
    bool RemoveDatabase(const std::string & name);
    void ShutDown(bool autoSave = true);

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    bool SaveDatabase(SPtr<IDatabase> database, const std::string & archive, EArchiveFormat fmt = EArchiveFormat::TXT);
    bool LoadDatabase(SPtr<IDatabase> database, const std::string & archive, EArchiveFormat fmt = EArchiveFormat::TXT);
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    ///Cell
    Ptr<ICell> CreateCircuitCell(SPtr<IDatabase> database, const std::string & name);
    Ptr<ICell> FindCellByName(SPtr<IDatabase> database, const std::string & name);

    ///Net
    Ptr<INet> CreateNet(Ptr<ILayoutView> layout, const std::string & name);

    ///Layer
    UPtr<ILayer> CreateStackupLayer(const std::string & name, ELayerType type, FCoord elevation, FCoord thickness);
    
    ///LayerMap
    Ptr<ILayerMap> CreateLayerMap(SPtr<IDatabase> database, const std::string & name);

    ///PadstackDef
    Ptr<IPadstackDef> CreatePadstackDef(Ptr<IDatabase> database, const std::string & name);
    UPtr<IPadstackDefData> CreatePadstackDefData();

    ///PadstackInst
    Ptr<IPadstackInst> CreatePadstackInst(Ptr<ILayoutView> layout, const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                          ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                          const ETransform2D & transform); 

    ///CellInstance
    virtual Ptr<ICellInst> CreateCellInst(Ptr<ILayoutView> layout, const std::string & name,
                                          Ptr<ILayoutView> defLayout, const ETransform2D & transform);

    ///Primitive
    Ptr<IPrimitive> CreateGeometry2D(Ptr<ILayoutView> layout, ELayerId layer, ENetId net, UPtr<EShape> shape);

    ///Shape
    UPtr<EShape> CreateShapePolygon(std::vector<EPoint2D> points);
    UPtr<EShape> CreateShapePolygon(Polygon2D<ECoord> polygon);
    UPtr<EShape> CreateShapePolygonWithHoles(PolygonWithHoles2D<ECoord> pwh);

    ///Text
    Ptr<IText> CreateText(Ptr<ILayoutView> layout, ELayerId layer, const ETransform2D & transform, const std::string & text);

    static EDataMgr & Instance();

private:
    EDataMgr();
    ~EDataMgr();

    mutable std::mutex m_databaseMutex;
    mutable std::mutex m_dataObjsMutex;//unused currently
    std::unordered_map<std::string, SPtr<IDatabase> > m_databases;
};

}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EDataMgr.cpp"
#endif

#endif//ECAD_EDATAMGR_H