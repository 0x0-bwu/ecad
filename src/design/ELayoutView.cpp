#include "ELayoutView.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ELayoutView)

#include "extraction/geometry/EGeometryModelExtraction.h"
#include "extraction/thermal/EThermalModelExtraction.h"

#include "simulation/thermal/EThermalSimulation.h"

#include "utility/EMetalFractionMapping.h"
#include "utility/ELayoutPolygonMerger.h"
#include "utility/ELayoutMergeUtility.h"
#include "utility/ELayoutConnectivity.h"
#include "utility/ELayoutViewRenderer.h"
#include "utility/ELayout2CtmUtility.h"
#include "utility/ELayoutModifier.h"
#include "utility/ELayoutRetriever.h"

#include "interface/IHierarchyObjCollection.h"
#include "interface/IPadstackInstCollection.h"
#include "interface/IComponentCollection.h"
#include "interface/IPrimitiveCollection.h"
#include "interface/ICellInstCollection.h"
#include "interface/IConnObjCollection.h"
#include "interface/ILayerCollection.h"
#include "interface/IModelCollection.h"
#include "interface/INetCollection.h"
#include "interface/IComponent.h"
#include "interface/IPrimitive.h"
#include "interface/ILayerMap.h"
#include "interface/IModel.h"
#include "interface/ILayer.h"
#include "interface/ICell.h"

#include "basic/EShape.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void ELayoutView::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayoutView, ILayoutView>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollectionCollection);
    ar & boost::serialization::make_nvp("boundary", m_boundary);
    ar & boost::serialization::make_nvp("cell", m_cell);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ELayoutView)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ELayoutView::ELayoutView()
 : ELayoutView(std::string{}, nullptr)
{
}

ELayoutView::ELayoutView(std::string name, Ptr<ICell> cell)
 : EObject(std::move(name))
 , m_cell(cell)
{
    ECollectionCollection::Init();
}

ELayoutView::~ELayoutView()
{
}

ELayoutView::ELayoutView(const ELayoutView & other)
{
    *this = other;
}

ELayoutView & ELayoutView::operator= (const ELayoutView & other)
{
    ECollectionCollection::operator=(other);
    EObject::operator=(other);
    
    m_boundary = CloneHelper(other.m_boundary);
    m_cell = other.m_cell;

    auto primIter = GetPrimitiveIter();
    while (auto * primitive = primIter->Next()) {
        if (auto * bw = primitive->GetBondwireFromPrimitive(); bw) {
            if (auto * comp = bw->GetStartComponent(); comp)
                bw->SetStartComponent(FindComponentByName(comp->GetName()), bw->GetStartComponentPin());
            if (auto * comp = bw->GetEndComponent(); comp)
                bw->SetEndComponent(FindComponentByName(comp->GetName()), bw->GetEndComponentPin());
        }
    }
    //sync, todo
    return *this;
}

const ECoordUnits & ELayoutView::GetCoordUnits() const
{
    return GetCell()->GetCoordUnits();
}

NetIter ELayoutView::GetNetIter() const
{
    return GetNetCollection()->GetNetIter();
}

LayerIter ELayoutView::GetLayerIter() const
{
    return GetLayerCollection()->GetLayerIter();
}

ConnObjIter ELayoutView::GetConnObjIter() const
{
    return GetConnObjCollection()->GetConnObjIter();
}

CellInstIter ELayoutView::GetCellInstIter() const
{
    return GetCellInstCollection()->GetCellInstIter();
}

ComponentIter ELayoutView::GetComponentIter() const
{
    return GetComponentCollection()->GetComponentIter();
}

PrimitiveIter ELayoutView::GetPrimitiveIter() const
{
    return GetConnObjCollection()->GetPrimitiveCollection()->GetPrimitiveIter();
}

HierarchyObjIter ELayoutView::GetHierarchyObjIter() const
{
    return GetHierarchyObjCollection()->GetHierarchyObjIter();
}

PadstackInstIter ELayoutView::GetPadstackInstIter() const
{
    return GetConnObjCollection()->GetPadstackInstCollection()->GetPadstackInstIter();
}

CPtr<IDatabase> ELayoutView::GetDatabase() const
{
    return m_cell->GetDatabase();
}

void ELayoutView::SetCell(Ptr<ICell> cell)
{
    m_cell = cell;
}

Ptr<ICell> ELayoutView::GetCell() const
{
    return m_cell;
}

ELayerId ELayoutView::AppendLayer(UPtr<ILayer> layer) const
{
    return GetLayerCollection()->AppendLayer(std::move(layer));
}

std::vector<ELayerId> ELayoutView::AppendLayers(std::vector<UPtr<ILayer> > layers) const
{
    return GetLayerCollection()->AppendLayers(std::move(layers));
}

void ELayoutView::GetStackupLayers(std::vector<Ptr<IStackupLayer> > & layers) const
{
    return GetLayerCollection()->GetStackupLayers(layers);
}

void ELayoutView::GetStackupLayers(std::vector<CPtr<IStackupLayer> > & layers) const
{
    return GetLayerCollection()->GetStackupLayers(layers);
}

UPtr<ILayerMap> ELayoutView::AddDefaultDielectricLayers() const
{
    auto lyrMap = GetLayerCollection()->AddDefaultDielectricLayers();
    GetPrimitiveCollection()->Map(lyrMap.get());
    GetPadstackInstCollection()->Map(lyrMap.get());
    return lyrMap;
}

Ptr<ILayer> ELayoutView::FindLayerByLayerId(ELayerId lyrId) const
{
    return GetLayerCollection()->FindLayerByLayerId(lyrId);
}

Ptr<ILayer> ELayoutView::FindLayerByName(const std::string & name) const
{
    return GetLayerCollection()->FindLayerByName(name);
}

Ptr<INet> ELayoutView::CreateNet(const std::string & name)
{
    return GetNetCollection()->CreateNet(name);
}

Ptr<INet> ELayoutView::FindNetByName(const std::string & name) const
{
    return GetNetCollection()->FindNetByName(name);
}

Ptr<INet> ELayoutView::FindNetByNetId(ENetId netId) const
{
    return GetNetCollection()->FindNetByNetId(netId);
}

Ptr<IPadstackInst> ELayoutView::CreatePadstackInst(const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                                                ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                                                const ETransform2D & transform)
{
    return GetPadstackInstCollection()->CreatePadstackInst(name, def, net, topLyr, botLyr, layerMap, transform);
}

Ptr<ICellInst> ELayoutView::CreateCellInst(const std::string & name, Ptr<ILayoutView> defLayout, const ETransform2D & transform)
{
    return GetCellInstCollection()->CreateCellInst(this, name, defLayout, transform);
}

Ptr<IComponent> ELayoutView::CreateComponent(const std::string & name, CPtr<IComponentDef> compDef, ELayerId layer, const ETransform2D & transform, bool flipped)
{
    return GetComponentCollection()->CreateComponent(name, this, compDef, layer, transform, flipped);
}

Ptr<IComponent> ELayoutView::FindComponentByName(const std::string & name) const
{
    return GetComponentCollection()->FindComponentByName(name);
}

Ptr<IPrimitive> ELayoutView::CreateGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape)
{
    return GetPrimitiveCollection()->CreateGeometry2D(layer, net, std::move(shape));
}

Ptr<IBondwire> ELayoutView::CreateBondwire(std::string name, ENetId net, EFloat radius)
{
    return GetPrimitiveCollection()->CreateBondwire(std::move(name), net, radius);
}

Ptr<IText> ELayoutView::CreateText(ELayerId layer, const ETransform2D & transform, const std::string & text)
{
    return GetPrimitiveCollection()->CreateText(layer, transform, text);
}

Ptr<INetCollection> ELayoutView::GetNetCollection() const
{
    return ECollectionCollection::NetCollection();
}

Ptr<ILayerCollection> ELayoutView::GetLayerCollection() const
{
    return ECollectionCollection::LayerCollection();
}

Ptr<IModelCollection> ELayoutView::GetModelCollection() const
{
    return ECollectionCollection::ModelCollection();
}

Ptr<IConnObjCollection> ELayoutView::GetConnObjCollection() const
{
    return ECollectionCollection::ConnObjCollection();
}

Ptr<ICellInstCollection> ELayoutView::GetCellInstCollection() const
{
    return GetHierarchyObjCollection()->GetCellInstCollection();
}

Ptr<IComponentCollection> ELayoutView::GetComponentCollection() const
{
    return GetHierarchyObjCollection()->GetComponentCollection();
}

Ptr<IPrimitiveCollection> ELayoutView::GetPrimitiveCollection() const
{
    return GetConnObjCollection()->GetPrimitiveCollection();
}

Ptr<IHierarchyObjCollection> ELayoutView::GetHierarchyObjCollection() const
{
    return ECollectionCollection::HierarchyObjCollection();
}

Ptr<IPadstackInstCollection> ELayoutView::GetPadstackInstCollection() const
{
    return GetConnObjCollection()->GetPadstackInstCollection();
}

void ELayoutView::SetBoundary(UPtr<EShape> boundary)
{
    m_boundary = std::move(boundary);
}

CPtr<EShape> ELayoutView::GetBoundary() const
{
    if(nullptr == m_boundary) 
        m_boundary = utils::ELayoutRetriever::CalculateLayoutBoundaryShape(const_cast<Ptr<ELayoutView> >(this));
    return m_boundary.get();
}

bool ELayoutView::GenerateMetalFractionMapping(const EMetalFractionMappingSettings & settings)
{
    utils::ELayoutMetalFractionMapper mapper(settings);
    return mapper.GenerateMetalFractionMapping(this);
}

bool ELayoutView::GenerateCTMv1File(const ELayout2CtmSettings & settings)
{
    utils::ELayout2CtmUtility utility(this);
    utility.SetLayout2CtmSettings(settings);
    return utility.GenerateCTMv1File();
}

void ELayoutView::ExtractConnectivity()
{
    utils::ELayoutConnectivity::ConnectivityExtraction(this);
}

bool ELayoutView::MergeLayerPolygons(const ELayoutPolygonMergeSettings & settings)
{
    utils::ELayoutPolygonMerger merger(this);
    merger.SetLayoutMergeSettings(settings);
    merger.Merge();
    return true;
}

CPtr<IModel> ELayoutView::ExtractLayerCutModel(const ELayerCutModelExtractionSettings & settings)
{
    auto collection = GetModelCollection();
    auto model = collection->FindModel(EModelType::LayerCut);
    if (model && model->Match(settings)) {
        ECAD_TRACE("reuse exist layer cut model");
        return model;
    }
    collection->AddModel(extraction::EGeometryModelExtraction::GenerateLayerCutModel(this, settings));
    return collection->FindModel(EModelType::LayerCut);
}

CPtr<IModel> ELayoutView::ExtractThermalModel(const EThermalModelExtractionSettings & settings)
{
    auto modelType = settings.GetModelType();
    auto collection = GetModelCollection();
    auto model = collection->FindModel(modelType);
    if (not settings.forceRebuild && model && model->Match(settings)) {
        ECAD_TRACE("reuse exist %1% model", toString(modelType));
        return model;
    }
    collection->AddModel(extraction::EThermalModelExtraction::GenerateThermalModel(this, settings));
    return collection->FindModel(modelType);
}

EPair<EFloat, EFloat> ELayoutView::RunThermalSimulation(const EThermalStaticSimulationSetup & simulationSetup, std::vector<EFloat> & temperatures)
{
    temperatures.clear();
    if (nullptr == simulationSetup.extractionSettings)
        return {invalidFloat, invalidFloat};
    auto model = ExtractThermalModel(*simulationSetup.extractionSettings);
    if (nullptr == model) return {invalidFloat, invalidFloat};

    simulation::EThermalSimulation sim(model, simulationSetup);
    return sim.RunStaticSimulation(temperatures);
}

EPair<EFloat, EFloat> ELayoutView::RunThermalSimulation(const EThermalTransientSimulationSetup & simulationSetup, const EThermalTransientExcitation & excitation)
{
    if (nullptr == simulationSetup.extractionSettings)
        return {invalidFloat, invalidFloat};
    auto model = ExtractThermalModel(*simulationSetup.extractionSettings);
    if (nullptr == model) return {invalidFloat, invalidFloat};

    simulation::EThermalSimulation sim(model, simulationSetup);
    return sim.RunTransientSimulation(excitation);
}

void ELayoutView::Flatten(const EFlattenOption & option)
{
    ECAD_UNUSED(option)//todo
    
    auto cellInstIter = GetCellInstIter();
    while (auto cellInst = cellInstIter->Next())
        utils::ELayoutMergeUtility::Merge(this, cellInst);
        
    GetHierarchyObjCollection()->GetCellInstCollection()->Clear();
}

void ELayoutView::Map(CPtr<ILayerMap> lyrMap)
{
    GetPrimitiveCollection()->Map(lyrMap);
    GetPadstackInstCollection()->Map(lyrMap);
}

bool ELayoutView::Renderer(const ELayoutViewRendererSettings & settings) const
{
    return utils::ELayoutViewRenderer(settings).Renderer(this);
}

bool ELayoutView::ModifyStackupLayerThickness(const std::string & name, EFloat thickness)
{
    return utils::ELayoutModifier::ModifyStackupLayerThickness(this, name, thickness);
}

void ELayoutView::SyncCloneReference(ECloneOption option)
{
    [[maybe_unused]] auto nc = GetNetCollection();
    [[maybe_unused]] auto lc = GetLayerCollection();
    //todo
}

ECollectionTypes ELayoutView::GetCollectionTypes() const
{
    return { 
        ECollectionType::HierarchyObj,
        ECollectionType::ConnObj,
        ECollectionType::Layer,
        ECollectionType::Model,
        ECollectionType::Net
    }; 
}

}//namespace ecad