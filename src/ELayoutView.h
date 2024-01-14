#pragma once
#include "interfaces/ILayoutView.h"
#include "interfaces/IIterator.h"
#include "ECollectionCollection.h"
#include "EObject.h"
#include <array>
namespace ecad {

class ICell;
class ECAD_API ELayoutView : public ECollectionCollection, public EObject, public ILayoutView
{
    ECAD_ALWAYS_INLINE static constexpr std::array<ECollectionType, 4> m_collectionTypes = { ECollectionType::HierarchyObj,
                                                                                             ECollectionType::ConnObj,
                                                                                             ECollectionType::Layer,
                                                                                             ECollectionType::Net };
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    ELayoutView();
public:
    explicit ELayoutView(std::string name, Ptr<ICell> cell);
    virtual ~ELayoutView();

    ///Copy
    ELayoutView(const ELayoutView & other);
    ELayoutView & operator= (const ELayoutView & other);

    const std::string & GetName() const override;
    std::string sUuid() const override;

    ///Units
    const ECoordUnits & GetCoordUnits() const override;

    ///Iterator objects
    NetIter GetNetIter() const override;
    LayerIter GetLayerIter() const override;
    ConnObjIter GetConnObjIter() const override;
    CellInstIter GetCellInstIter() const override;
    ComponentIter GetComponentIter() const override;
    PrimitiveIter GetPrimitiveIter() const override;
    HierarchyObjIter GetHierarchyObjIter() const override;
    PadstackInstIter GetPadstackInstIter() const override;

    ///Database
    CPtr<IDatabase> GetDatabase() const override;

    ///Cell
    void SetCell(Ptr<ICell> cell) override;
    Ptr<ICell> GetCell() const override;

    ///Layer
    ELayerId AppendLayer(UPtr<ILayer> layer) const override;
    std::vector<ELayerId> AppendLayers(std::vector<UPtr<ILayer> > layers) const override;
    void GetStackupLayers(std::vector<Ptr<IStackupLayer> > & layers) const override;
    void GetStackupLayers(std::vector<CPtr<IStackupLayer> > & layers) const override;
    UPtr<ILayerMap> AddDefaultDielectricLayers() const override;
    Ptr<ILayer> FindLayerByLayerId(ELayerId lyrId) const override;
    Ptr<ILayer> FindLayerByName(const std::string & name) const override;

    ///Net
    Ptr<INet> CreateNet(const std::string & name) override;
    Ptr<INet> FindNetByName(const std::string & name) const override;
    Ptr<INet> FindNetByNetId(ENetId netId) const override;

    ///PadstackInst
    Ptr<IPadstackInst> CreatePadstackInst(const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                          ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                          const ETransform2D & transform) override; 
    ///CellInst
    Ptr<ICellInst> CreateCellInst(const std::string & name, Ptr<ILayoutView> defLayout, const ETransform2D & transform) override;
    
    ///Component
    Ptr<IComponent> CreateComponent(const std::string & name, CPtr<IComponentDef> compDef, ELayerId layer, const ETransform2D & transform, bool flipped) override;
    Ptr<IComponent> FindComponentByName(const std::string & name) const override;

    ///Primitive
    Ptr<IPrimitive> CreateGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape) override;
    Ptr<IBondwire> CreateBondwire(std::string name, ENetId net, EFloat radius) override;

    ///Text
    Ptr<IText> CreateText(ELayerId layer, const ETransform2D & transform, const std::string & text) override;

    ///ECollectionCollection
    Ptr<INetCollection> GetNetCollection() const override;
    Ptr<ILayerCollection> GetLayerCollection() const override;
    Ptr<IConnObjCollection> GetConnObjCollection() const override;
    Ptr<ICellInstCollection> GetCellInstCollection() const override;
    Ptr<IComponentCollection> GetComponentCollection() const override;
    Ptr<IPrimitiveCollection> GetPrimitiveCollection() const override;
    Ptr<IHierarchyObjCollection> GetHierarchyObjCollection() const override;
    Ptr<IPadstackInstCollection> GetPadstackInstCollection() const override;

    ///Boundary
    void SetBoundary(UPtr<EShape> boundary) override;
    CPtr<EShape> GetBoundary() const override;

    ///Metal Fraction Mapping
    bool GenerateMetalFractionMapping(const EMetalFractionMappingSettings & settings) override;

    ///Layout to CTMv1
    bool GenerateCTMv1File(const ELayout2CtmSettings & settings) override;

    ///Connectivity Extraction
    void ExtractConnectivity() override;

    ///Layout Polygon Merge
    bool MergeLayerPolygons(const ELayoutPolygonMergeSettings & settings) override;

    ///Thermal Model Extraction
    UPtr<IModel> ExtractThermalModel(const EThermalModelExtractionSettings & settings) override;

    ///Simulation
    EPair<EFloat, EFloat> RunThermalSimulation(const EThermalModelExtractionSettings & extractionSettings, const EThermalSimulationSetup & simulationSetup) override;

    ///Flatten
    void Flatten(const EFlattenOption & option) override;

    ///Mapping
    void Map(CPtr<ILayerMap> lyrMap) override;

    ///Render
    bool Renderer(const ELayoutViewRendererSettings & settings) const override;

    ///Modify
    bool ModifyStackupLayerThickness(const std::string & name, EFloat thickness) override;
protected:
    ///Copy
    virtual Ptr<ELayoutView> CloneImp() const override { return new ELayoutView(*this); }
    virtual void SyncCloneReference(ECloneOption option);

protected:
    mutable UPtr<EShape> m_boundary;
    Ptr<ICell> m_cell;
};

ECAD_ALWAYS_INLINE const std::string & ELayoutView::GetName() const
{
    return EObject::GetName();
}

ECAD_ALWAYS_INLINE std::string ELayoutView::sUuid() const
{
    return EObject::sUuid();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ELayoutView)