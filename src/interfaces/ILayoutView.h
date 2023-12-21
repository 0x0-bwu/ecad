#pragma once
#include "ECadSettings.h"
#include "IIterator.h"
#include "Protocol.h"
#include <string>
namespace ecad {

class INet;
class ICell;
class IText;
class EShape;
class ILayer;
class EPolygon;
class IBondwire;
class ICellInst;
class IDatabase;
class ILayerMap;
class IPrimitive;
class ETransform2D;
class IPadstackDef;
class IStackupLayer;
class IPadstackInst;
class INetCollection;
class ILayerCollection;
class IConnObjCollection;
class ICellInstCollection;
class IComponentCollection;
class IPrimitiveCollection;
class IHierarchyObjCollection;
class IPadstackInstCollection;
class ECAD_API ILayoutView : public Clonable<ILayoutView>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ILayoutView() = default;
    virtual const std::string & GetName() const = 0;
    virtual std::string sUuid() const = 0;
    
    ///Units
    virtual const ECoordUnits & GetCoordUnits() const = 0;

    ///Iterating objects
    virtual NetIter GetNetIter() const = 0;
    virtual LayerIter GetLayerIter() const = 0;
    virtual ConnObjIter GetConnObjIter() const = 0;
    virtual CellInstIter GetCellInstIter() const = 0;
    virtual ComponentIter GetComponentIter() const = 0;
    virtual PrimitiveIter GetPrimitiveIter() const = 0;
    virtual HierarchyObjIter GetHierarchyObjIter() const = 0;
    virtual PadstackInstIter GetPadstackInstIter() const = 0;

    ///Database
    virtual CPtr<IDatabase> GetDatabase() const = 0;

    ///Cell
    virtual void SetCell(Ptr<ICell> cell) = 0;
    virtual Ptr<ICell> GetCell() const = 0;

    ///Layer
    virtual ELayerId AppendLayer(UPtr<ILayer> layer) const = 0;
    virtual std::vector<ELayerId> AppendLayers(std::vector<UPtr<ILayer> > layers) const = 0;
    virtual void GetStackupLayers(std::vector<Ptr<IStackupLayer> > & layers) const = 0;
    virtual void GetStackupLayers(std::vector<CPtr<IStackupLayer> > & layers) const = 0;
    virtual UPtr<ILayerMap> AddDefaultDielectricLayers() const = 0;
    virtual Ptr<ILayer> FindLayerByLayerId(ELayerId lyrId) const = 0;
    virtual Ptr<ILayer> FindLayerByName(const std::string & name) const = 0;

    ///Net
    virtual Ptr<INet> CreateNet(const std::string & name) = 0;
    virtual Ptr<INet> FindNetByName(const std::string & name) const = 0;
    virtual Ptr<INet> FindNetByNetId(ENetId netId) const = 0;

    ///Padstack
    virtual Ptr<IPadstackInst> CreatePadstackInst(const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                                    ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                                    const ETransform2D & transform) = 0;
    ///Primitive                                          
    virtual Ptr<IPrimitive> CreateGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape) = 0;
    virtual Ptr<IBondwire> CreateBondwire(std::string name, ENetId net, EPoint2D start, EPoint2D end, FCoord radius) = 0;
    ///Text
    virtual Ptr<IText> CreateText(ELayerId layer, const ETransform2D & transform, const std::string & text) = 0;

    ///CellInstance
    virtual Ptr<ICellInst> CreateCellInst(const std::string & name, Ptr<ILayoutView> defLayout, const ETransform2D & transform) = 0;

    ///Component
    virtual Ptr<IComponent> CreateComponent(const std::string & name, CPtr<IComponentDef> compDef, ELayerId layer, const ETransform2D & transform, FCoord height, bool flipped) = 0;

    ///Collection
    virtual Ptr<INetCollection> GetNetCollection() const = 0;
    virtual Ptr<ILayerCollection> GetLayerCollection() const = 0;
    virtual Ptr<IConnObjCollection> GetConnObjCollection() const = 0;
    virtual Ptr<ICellInstCollection> GetCellInstCollection() const = 0;
    virtual Ptr<IComponentCollection> GetComponentCollection() const = 0;
    virtual Ptr<IPrimitiveCollection> GetPrimitiveCollection() const = 0;
    virtual Ptr<IHierarchyObjCollection> GetHierarchyObjCollection() const = 0;
    virtual Ptr<IPadstackInstCollection> GetPadstackInstCollection() const = 0;

    ///Boundary
    virtual void SetBoundary(UPtr<EPolygon> boundary) = 0;
    virtual CPtr<EPolygon> GetBoundary() const = 0;

    ///Flatten
    virtual void Flatten(const EFlattenOption & option) = 0;

    ///Metal Fraction Mapping
    virtual bool GenerateMetalFractionMapping(const EMetalFractionMappingSettings & settings) = 0;

    ///Layout to CTMv1
    virtual bool GenerateCTMv1File(const ELayout2CtmSettings & settings) = 0;

    ///Connectivity Extraction
    virtual void ExtractConnectivity() = 0;

    ///Layout Polygon Merge
    virtual bool MergeLayerPolygons(const ELayoutPolygonMergeSettings & settings) = 0;

    ///Thermal Network Extraction
    virtual bool ExtractThermalNetwork(const EThermalNetworkExtractionSettings & settings) = 0;

    ///Mapping
    virtual void Map(CPtr<ILayerMap> lyrMap) = 0;

    ///Renderer
    virtual bool Renderer(const ELayoutViewRendererSettings & settings) const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ILayoutView)