#ifndef ECAD_HEADER_ONLY
#include "ELayoutView.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ELayoutView)
#endif

#include "simulation/EMetalFractionMapping.h"
#include "utility/EBoundaryCalculator.h"
#include "utility/ELayoutMergeUtility.h"
#include "utility/ELayoutConnectivity.h"
#include "Interface.h"
#include "EShape.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ELayoutView::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayoutView, ILayoutView>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
    ar & boost::serialization::make_nvp("boundary", m_boundary);
    ar & boost::serialization::make_nvp("cell", m_cell);
}

template <typename Archive>
ECAD_INLINE void ELayoutView::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayoutView, ILayoutView>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
    ar & boost::serialization::make_nvp("boundary", m_boundary);
    ar & boost::serialization::make_nvp("cell", m_cell);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ELayoutView)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ELayoutView::ELayoutView()
 : ELayoutView(std::string{}, nullptr)
{
}

ECAD_INLINE ELayoutView::ELayoutView(std::string name, Ptr<ICell> cell)
 : ECollectionCollection(std::move(name))
 , m_cell(cell)
{
    for(auto type : m_collectionTypes) 
        AddCollection(type);
}

ECAD_INLINE ELayoutView::~ELayoutView()
{
}

ECAD_INLINE ELayoutView::ELayoutView(const ELayoutView & other)
{
    *this = other;
}

ECAD_INLINE ELayoutView & ELayoutView::operator= (const ELayoutView & other)
{
    ECollectionCollection::operator=(other);
    if(other.m_boundary)
        m_boundary.reset(new EPolygon(*other.m_boundary));
    m_cell = other.m_cell;
    //sync, todo
    return *this;
}

ECAD_INLINE NetIter ELayoutView::GetNetIter() const
{
    return GetNetCollection()->GetNetIter();
}

ECAD_INLINE LayerIter ELayoutView::GetLayerIter() const
{
    return GetLayerCollection()->GetLayerIter();
}

ECAD_INLINE ConnObjIter ELayoutView::GetConnObjIter() const
{
    return GetConnObjCollection()->GetConnObjIter();
}

ECAD_INLINE CellInstIter ELayoutView::GetCellInstIter() const
{
    return GetCellInstCollection()->GetCellInstIter();
}

ECAD_INLINE PrimitiveIter ELayoutView::GetPrimitiveIter() const
{
    return GetConnObjCollection()->GetPrimitiveCollection()->GetPrimitiveIter();
}

ECAD_INLINE HierarchyObjIter ELayoutView::GetHierarchyObjIter() const
{
    return GetHierarchyObjCollection()->GetHierarchyObjIter();
}

ECAD_INLINE PadstackInstIter ELayoutView::GetPadstackInstIter() const
{
    return GetConnObjCollection()->GetPadstackInstCollection()->GetPadstackInstIter();
}

ECAD_INLINE void ELayoutView::SetCell(Ptr<ICell> cell)
{
    m_cell = cell;
}

ECAD_INLINE Ptr<ICell> ELayoutView::GetCell() const
{
    return m_cell;
}

ECAD_INLINE ELayerId ELayoutView::AppendLayer(UPtr<ILayer> layer) const
{
    return GetLayerCollection()->AppendLayer(std::move(layer));
}

ECAD_INLINE std::vector<ELayerId> ELayoutView::AppendLayers(std::vector<UPtr<ILayer> > layers) const
{
    return GetLayerCollection()->AppendLayers(std::move(layers));
}

ECAD_INLINE void ELayoutView::GetStackupLayers(std::vector<Ptr<ILayer> > & layers) const
{
    return GetLayerCollection()->GetStackupLayers(layers);
}

ECAD_INLINE void ELayoutView::GetStackupLayers(std::vector<CPtr<ILayer> > & layers) const
{
    return GetLayerCollection()->GetStackupLayers(layers);
}

ECAD_INLINE UPtr<ILayerMap> ELayoutView::AddDefaultDielectricLayers() const
{
    auto lyrMap = GetLayerCollection()->AddDefaultDielectricLayers();
    GetPrimitiveCollection()->Map(lyrMap.get());
    GetPadstackInstCollection()->Map(lyrMap.get());
    return std::move(lyrMap);
}

ECAD_INLINE Ptr<INet> ELayoutView::CreateNet(const std::string & name)
{
    return GetNetCollection()->CreateNet(name);
}

ECAD_INLINE Ptr<IPadstackInst> ELayoutView::CreatePadstackInst(const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                                                ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                                                const ETransform2D & transform)
{
    return GetPadstackInstCollection()->CreatePadstackInst(name, def, net, topLyr, botLyr, layerMap, transform);
}

ECAD_INLINE Ptr<ICellInst> ELayoutView::CreateCellInst(const std::string & name, Ptr<ILayoutView> defLayout, const ETransform2D & transform)
{
    return GetCellInstCollection()->CreateCellInst(this, name, defLayout, transform);
}

ECAD_INLINE Ptr<IPrimitive> ELayoutView::CreateGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape)
{
    return GetPrimitiveCollection()->CreateGeometry2D(layer, net, std::move(shape));
}

ECAD_INLINE Ptr<IText> ELayoutView::CreateText(ELayerId layer, const ETransform2D & transform, const std::string & text)
{
    return GetPrimitiveCollection()->CreateText(layer, transform, text);
}

ECAD_INLINE Ptr<INetCollection> ELayoutView::GetNetCollection() const
{
    return ECollectionCollection::GetNetCollection();
}

ECAD_INLINE Ptr<ILayerCollection> ELayoutView::GetLayerCollection() const
{
    return ECollectionCollection::GetLayerCollection();
}

ECAD_INLINE Ptr<IConnObjCollection> ELayoutView::GetConnObjCollection() const
{
    return ECollectionCollection::GetConnObjCollection();
}

ECAD_INLINE Ptr<ICellInstCollection> ELayoutView::GetCellInstCollection() const
{
    return GetHierarchyObjCollection()->GetCellInstCollection();
}

ECAD_INLINE Ptr<IPrimitiveCollection> ELayoutView::GetPrimitiveCollection() const
{
    return GetConnObjCollection()->GetPrimitiveCollection();
}

ECAD_INLINE Ptr<IHierarchyObjCollection> ELayoutView::GetHierarchyObjCollection() const
{
    return ECollectionCollection::GetHierarchyObjCollection();
}

ECAD_INLINE Ptr<IPadstackInstCollection> ELayoutView::GetPadstackInstCollection() const
{
    return GetConnObjCollection()->GetPadstackInstCollection();
}

ECAD_INLINE void ELayoutView::SetBoundary(UPtr<EPolygon> boundary)
{
    m_boundary = std::move(boundary);
}

ECAD_INLINE CPtr<EPolygon> ELayoutView::GetBoundary() const
{
    if(nullptr == m_boundary) 
        m_boundary = std::move(euti::CalculateBoundary(const_cast<Ptr<ELayoutView> >(this)));
    return m_boundary.get();
}

ECAD_INLINE bool ELayoutView::GenerateMetalFractionMapping(const esim::EMetalFractionMappingSettings & settings) const
{
    esim::LayoutMetalFractionMapper mapper(settings);
    return mapper.GenerateMetalFractionMapping(this);
}

ECAD_INLINE void ELayoutView::ConnectivityExtraction()
{
    euti::ELayoutConnectivity::ConnectivityExtraction(this);
}

ECAD_INLINE void ELayoutView::Flatten(const EFlattenOption & option)
{
    ECAD_UNUSED(option)//todo
    
    auto cellInstIter = GetCellInstIter();
    while(auto cellInst = cellInstIter->Next()){
        auto layout = cellInst->GetFlattenedLayoutView();
        const auto & transform  = cellInst->GetTransform();
        Merge(layout, transform);
    }
    GetHierarchyObjCollection()->GetCellInstCollection()->Clear();
}

ECAD_INLINE void ELayoutView::Merge(CPtr<ILayoutView> other, const ETransform2D & transform)
{
    euti::ELayoutMergeUtility::Merge(this, other, transform);
}

ECAD_INLINE void ELayoutView::Map(CPtr<ILayerMap> lyrMap)
{
    GetPrimitiveCollection()->Map(lyrMap);
    GetPadstackInstCollection()->Map(lyrMap);
}

ECAD_INLINE void ELayoutView::SyncCloneReference(ECloneOption option)
{
    auto nc = GetNetCollection();
    auto lc = GetLayerCollection();
    //todo
}

}//namesapce ecad