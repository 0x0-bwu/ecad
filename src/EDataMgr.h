#pragma once
#include "ECadSettings.h"
#include "Interface.h"
#include "EShape.h"

#include <unordered_map>
#include <memory>
#include <mutex>
namespace ecad {

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

    ///Thirdpart
    SPtr<IDatabase> CreateDatabaseFromGds(const std::string & name, const std::string & gds, const std::string & lyrMap = std::string{});
    SPtr<IDatabase> CreateDatabaseFromXfl(const std::string & name, const std::string & xfl);

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    bool SaveDatabase(SPtr<IDatabase> database, const std::string & archive, EArchiveFormat fmt = EArchiveFormat::BIN);
    bool LoadDatabase(SPtr<IDatabase> database, const std::string & archive, EArchiveFormat fmt = EArchiveFormat::BIN);
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    ///Cell
    Ptr<ICell> CreateCircuitCell(SPtr<IDatabase> database, const std::string & name);
    Ptr<ICell> FindCellByName(SPtr<IDatabase> database, const std::string & name);

    ///Net
    Ptr<INet> CreateNet(Ptr<ILayoutView> layout, const std::string & name);
    Ptr<INet> FindNetByName(Ptr<ILayoutView> layout, const std::string & name);

    ///Layer
    UPtr<ILayer> CreateStackupLayer(const std::string & name, ELayerType type, FCoord elevation, FCoord thickness,
                                    const std::string & conductingMat = sDefaultConductingMat,
                                    const std::string & dirlectricMat = sDefaultDielectricMat);
    
    ///ComponentDef
    Ptr<IComponentDef> CreateComponentDef(SPtr<IDatabase> database, const std::string & name);
    Ptr<IComponentDef> FindComponentDefByName(SPtr<IDatabase> database, const std::string & name);

    ///Material
    Ptr<IMaterialDef> CreateMaterialDef(SPtr<IDatabase> database, const std::string & name);
    Ptr<IMaterialDef> FindMaterialDefByName(SPtr<IDatabase> database, const std::string & name);
    UPtr<IMaterialProp> CreateSimpleMaterialProp(EValue value);
    UPtr<IMaterialProp> CreateAnsiotropicMaterialProp(const std::array<EValue, 3> & values);
    UPtr<IMaterialProp> CreateTensorMateriaProp(const std::array<EValue, 9> & values);

    ///LayerMap
    Ptr<ILayerMap> CreateLayerMap(SPtr<IDatabase> database, const std::string & name);
    Ptr<ILayerMap> FindLayerMapByName(SPtr<IDatabase> database, const std::string & name);

    ///PadstackDef
    Ptr<IPadstackDef> CreatePadstackDef(SPtr<IDatabase> database, const std::string & name);
    Ptr<IPadstackDef> FindPadstackDefByName(SPtr<IDatabase> database, const std::string & name) const;
    UPtr<IPadstackDefData> CreatePadstackDefData();

    ///PadstackInst
    Ptr<IPadstackInst> CreatePadstackInst(Ptr<ILayoutView> layout, const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                          ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                          const ETransform2D & transform); 

    ///CellInstance
    Ptr<ICellInst> CreateCellInst(Ptr<ILayoutView> layout, const std::string & name,
                                  Ptr<ILayoutView> defLayout, const ETransform2D & transform);

    ///Component
    Ptr<IComponent> CreateComponent(Ptr<ILayoutView> layout, const std::string & name, CPtr<IComponentDef> compDef,
                                    ELayerId layer, const ETransform2D & transform, bool flipped);
                                    
    ///Primitive
    Ptr<IPrimitive> CreateGeometry2D(Ptr<ILayoutView> layout, ELayerId layer, ENetId net, UPtr<EShape> shape);
    Ptr<IBondwire> CreateBondwire(Ptr<ILayoutView> layout, std::string name, ENetId net, EPoint2D start, EPoint2D end, FCoord radius);

    ///Shape
    UPtr<EShape> CreateShapeRectangle(EPoint2D ll, EPoint2D ur);
    UPtr<EShape> CreateShapePath(std::vector<EPoint2D> points, ECoord width);
    UPtr<EShape> CreateShapePolygon(std::vector<EPoint2D> points);
    UPtr<EShape> CreateShapePolygon(Polygon2D<ECoord> polygon);
    UPtr<EShape> CreateShapePolygonWithHoles(PolygonWithHoles2D<ECoord> pwh);
    UPtr<EShape> CreateShapeFromTemplate(ETemplateShape ts, ETransform2D trans = ETransform2D{});

    ///Text
    Ptr<IText> CreateText(Ptr<ILayoutView> layout, ELayerId layer, const ETransform2D & transform, const std::string & text);

    ///ComponentDefPin
    Ptr<IComponentDefPin> CreateComponentDefPin(Ptr<IComponentDef> compDef, const std::string & pinName, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef = nullptr, ELayerId lyr = noLayer);
    
    ///Settings
    size_t DefaultThreads() const;
    void SetDefaultThreads(size_t threads);

    static EDataMgr & Instance();

private:
    EDataMgr();
    ~EDataMgr();

    EDataMgrSettings m_settings;
    std::unordered_map<std::string, SPtr<IDatabase> > m_databases;
};

}//namespace ecad
