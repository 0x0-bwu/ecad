#ifndef ECAD_HEADER_ONLY
#include "utility/ELayoutPolygonMerger.h"
#endif

#include "generic/geometry/PolygonMerge.hpp"
#include "Interface.h"
namespace ecad {
namespace euti {

using namespace generic;
using namespace generic::geometry;
ECAD_INLINE ELayoutPolygonMerger::ELayoutPolygonMerger(Ptr<ILayoutView> layout)
 : m_layout(layout)
{
    std::vector<CPtr<ILayer> > layers;
    layout->GetStackupLayers(layers);

    for(size_t i = 0; i < layers.size(); ++i) {
        auto layer = layers.at(i);
        auto type = layer->GetLayerType();
        if(!m_settings.includeDielectricLayer && type == ELayerType::DielectricLayer) continue;
        if(m_settings.skipTopBotDielectricLayers && type == ELayerType::DielectricLayer) {
            if(i == 0 || i == layers.size() - 1) continue;
        }

        m_mergers.insert(std::make_pair(layer->GetLayerId(), std::make_unique<LayerMerger>()));
    }
}

ECAD_INLINE ELayoutPolygonMerger::~ELayoutPolygonMerger()
{
}

ECAD_INLINE void ELayoutPolygonMerger::SetLayoutMergeSettings(ELayoutPolygonMergeSettings settings)
{
    m_settings = std::move(settings);
}

ECAD_INLINE void ELayoutPolygonMerger::Merge()
{
    FillPolygonsFromLayout();
    MergeLayers();
    FillPolygonsBackToLayout();
}

ECAD_INLINE void ELayoutPolygonMerger::FillPolygonsFromLayout()
{
    m_primTobeRemove.clear();

    size_t i = 0;
    auto primitives = m_layout->GetPrimitiveCollection();
    for(size_t i = 0; i < primitives->Size(); ++i) {
        auto prim = primitives->GetPrimitive(i);
        auto geom = prim->GetGeometry2DFromPrimitive();
        if(nullptr == geom) continue;

        auto shape = geom->GetShape();
        auto netId = prim->GetNet();
        auto lyrId = prim->GetLayer();
        if(FillOneShape(netId, lyrId, shape))
            m_primTobeRemove.insert(i);
    }

    bool bSelNet = m_settings.selectNets.size() > 0;
    const auto & selNets = m_settings.selectNets;
    if(m_settings.includePadstackInst) {
        auto psInstIter = m_layout->GetPadstackInstIter();
        while(auto psInst = psInstIter->Next()){

            auto netId = psInst->GetNet();
            if(bSelNet && !selNets.count(netId)) continue;

            auto defData = psInst->GetPadstackDef()->GetPadstackDefData();
            if(nullptr == defData) continue;

            ELayerId top, bot;
            psInst->GetLayerRange(top, bot);
            for(int lyrId = std::min(top, bot); lyrId <= std::max(top, bot); lyrId++) {
                auto shape = psInst->GetLayerShape(static_cast<ELayerId>(lyrId));
                FillOneShape(netId, static_cast<ELayerId>(lyrId), shape.get());
            }
        }
    }
}

ECAD_INLINE void ELayoutPolygonMerger::MergeLayers()
{
    size_t threads = std::min(m_mergers.size(), m_settings.threads);
    thread::ThreadPool pool(threads);
    for(auto & merger : m_mergers) {
        pool.Submit(std::bind(&ELayoutPolygonMerger::MergeOneLayer, this, merger.second.get()));
    }
}

ECAD_INLINE void ELayoutPolygonMerger::MergeOneLayer(Ptr<LayerMerger> merger)
{
    typename LayerMerger::MergeSettings settings;
    //todo, add settings
    merger->SetMergeSettings(settings);

    PolygonMergeRunner runner(*merger, m_settings.threads);
    runner.Run();
}

ECAD_INLINE void ELayoutPolygonMerger::FillPolygonsBackToLayout()
{
    using PolygonData = typename LayerMerger::PolygonData;
    auto primitives = m_layout->GetPrimitiveCollection();
    for(auto & merger : m_mergers) {
        std::list<CPtr<PolygonData> > polygons;
        merger.second->GetAllPolygons(polygons);
        for(const auto * polygon : polygons) {
            if(!polygon->hasSolid()) continue;
            UPtr<EShape> eShape = nullptr;
            if(!polygon->hasHole()) {
                auto shape = new EPolygon;
                shape->shape = std::move(polygon->solid);
                eShape = UPtr<EShape>(shape);
            }
            else {
                auto shape = new EPolygonWithHoles;
                shape->shape.outline = std::move(polygon->solid);
                shape->shape.holes = std::move(polygon->holes);
                eShape = UPtr<EShape>(shape);
            }
            auto prim = primitives->CreateGeometry2D(merger.first, polygon->property, std::move(eShape));
            GENERIC_ASSERT(prim != nullptr)
        }
    }

    auto iter = m_primTobeRemove.begin();
    for(; iter != m_primTobeRemove.end(); ++iter) {
        primitives->SetPrimitive(std::move(primitives->PopBack()), *iter);
    }
}

ECAD_INLINE bool ELayoutPolygonMerger::FillOneShape(ENetId netId, ELayerId layerId, Ptr<EShape> shape)
{
    if(nullptr == shape) return false;
    auto merger = m_mergers.find(layerId);
    if(merger == m_mergers.end()) return false;
    if(m_settings.selectNets.size() && !m_settings.selectNets.count(netId)) return false;

    switch (shape->GetShapeType()) {
        case EShapeType::Rectangle : {
            auto rect = dynamic_cast<Ptr<ERectangle> >(shape);
            merger->second->AddObject(netId, rect->shape, rect->isVoid());
            break;
        }
        case EShapeType::Path : {
            auto path = dynamic_cast<Ptr<EPath> >(shape);
            merger->second->AddObject(netId, path->GetContour(), path->isVoid());
            break;
        }
        case EShapeType::Polygon : {
            auto polygon = dynamic_cast<Ptr<EPolygon> >(shape);
            merger->second->AddObject(netId, polygon->shape, polygon->isVoid());
            break;
        }
        case EShapeType::PolygonWithHoles : {
            auto pwh = dynamic_cast<Ptr<EPolygonWithHoles> >(shape);
            merger->second->AddObject(netId, pwh->shape);
            break;
        }
        default : {
            GENERIC_ASSERT(false)
        }
    }
    return true;
}

}//namespace euti
}//namespace ecad