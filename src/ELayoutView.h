#ifndef ECAD_ELAYOUTVIEW_H
#define ECAD_ELAYOUTVIEW_H
#include "interfaces/ILayoutView.h"
#include "interfaces/IIterator.h"
#include "ECollectionCollection.h"
#include <array>
namespace ecad {

class ICell;
class ICollectionCollection;
class ECAD_API ELayoutView : public ECollectionCollection, public ILayoutView
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

    const std::string & GetName() const;
    std::string sUuid() const;

    ///Units
    const ECoordUnits & GetCoordUnits() const;

    ///Iterator objects
    NetIter GetNetIter() const;
    LayerIter GetLayerIter() const;
    ConnObjIter GetConnObjIter() const;
    CellInstIter GetCellInstIter() const;
    PrimitiveIter GetPrimitiveIter() const;
    HierarchyObjIter GetHierarchyObjIter() const;
    PadstackInstIter GetPadstackInstIter() const;

    ///Cell
    void SetCell(Ptr<ICell> cell);
    Ptr<ICell> GetCell() const;

    ///Layer
    ELayerId AppendLayer(UPtr<ILayer> layer) const;
    std::vector<ELayerId> AppendLayers(std::vector<UPtr<ILayer> > layers) const;
    void GetStackupLayers(std::vector<Ptr<ILayer> > & layers) const;
    void GetStackupLayers(std::vector<CPtr<ILayer> > & layers) const;
    UPtr<ILayerMap> AddDefaultDielectricLayers() const;
    Ptr<ILayer> FindLayerByLayerId(ELayerId lyrId) const;
    Ptr<ILayer> FindLayerByName(const std::string & name) const;

    ///Net
    Ptr<INet> CreateNet(const std::string & name);
    Ptr<INet> FindNetByName(const std::string & name) const;
    Ptr<INet> FindNetByNetId(ENetId netId) const;

    ///PadstackInst
    Ptr<IPadstackInst> CreatePadstackInst(const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                          ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                          const ETransform2D & transform); 
    ///CellInst
    Ptr<ICellInst> CreateCellInst(const std::string & name, Ptr<ILayoutView> defLayout, const ETransform2D & transform);
    
    ///Primitive
    Ptr<IPrimitive> CreateGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape);

    ///Text
    Ptr<IText> CreateText(ELayerId layer, const ETransform2D & transform, const std::string & text);

    ///ECollectionCollection
    Ptr<INetCollection> GetNetCollection() const;
    Ptr<ILayerCollection> GetLayerCollection() const;
    Ptr<IConnObjCollection> GetConnObjCollection() const;
    Ptr<ICellInstCollection> GetCellInstCollection() const;
    Ptr<IPrimitiveCollection> GetPrimitiveCollection() const;
    Ptr<IHierarchyObjCollection> GetHierarchyObjCollection() const;
    Ptr<IPadstackInstCollection> GetPadstackInstCollection() const;

    ///Boundary
    void SetBoundary(UPtr<EPolygon> boundary);
    CPtr<EPolygon> GetBoundary() const;

    ///Metal Fraction Mapping
    bool GenerateMetalFractionMapping(const EMetalFractionMappingSettings & settings) const;

    ///Connectivity Extraction
    void ExtractConnectivity();

    ///Layout Polygon Merge
    bool MergeLayerPolygons(const ELayoutPolygonMergeSettings & settings);

    ///Thermal Network Extraction
    bool ExtractThermalNetwork(const EThermalNetworkExtractionSettings & settings) const;
    
    ///Flatten
    void Flatten(const EFlattenOption & option);
    void Merge(CPtr<ILayoutView> other, const ETransform2D & transform);

    ///Mapping
    void Map(CPtr<ILayerMap> lyrMap);
    
protected:
    ///Copy
    virtual Ptr<ELayoutView> CloneImp() const override { return new ELayoutView(*this); }
    virtual void SyncCloneReference(ECloneOption option);

protected:
    mutable UPtr<EPolygon> m_boundary;
    Ptr<ICell> m_cell;
};

ECAD_ALWAYS_INLINE const std::string & ELayoutView::GetName() const
{
    return ECollectionCollection::GetName();
}

ECAD_ALWAYS_INLINE std::string ELayoutView::sUuid() const
{
    return ECollectionCollection::sUuid();
}

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ELayoutView)

#ifdef ECAD_HEADER_ONLY
#include "ELayoutView.cpp"
#endif

#endif//ECAD_ELAYOUTVIEW_H