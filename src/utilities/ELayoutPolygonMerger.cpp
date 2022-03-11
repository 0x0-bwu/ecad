#ifndef ECAD_HEADER_ONLY
#include "utilities/ELayoutPolygonMerger.h"
#endif

#include "generic/geometry/PolygonMerge.hpp"
#include "generic/geometry/GeometryIO.hpp"
#include "generic/tools/FileSystem.hpp"
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
    ECAD_EFFICIENCY_TRACK("layout polygon merge")

    FillPolygonsFromLayout();

    MergeLayers();

#ifdef BOOST_GIL_IO_PNG_SUPPORT
    if(!m_settings.outFile.empty())
        WritePngFiles(m_settings.outFile);
#endif//BOOST_GIL_IO_PNG_SUPPORT

    FillPolygonsBackToLayout();
}

ECAD_INLINE void ELayoutPolygonMerger::FillPolygonsFromLayout()
{
    m_netIdNameMap.clear();
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
    auto threads = m_settings.threads;
    if(threads > 1) {
        thread::ThreadPool pool(threads);
        for(auto & merger : m_mergers)
            pool.Submit(std::bind(&ELayoutPolygonMerger::MergeOneLayer, this, merger.second.get()));
    }
    else {
        for(auto & merger : m_mergers)
            MergeOneLayer(merger.second.get());
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
    if(!shape->isValid()) return false;
    auto merger = m_mergers.find(layerId);
    if(merger == m_mergers.end()) return false;
    if(m_settings.selectNets.size() && !m_settings.selectNets.count(netId)) return false;

    switch (shape->GetShapeType()) {
        case EShapeType::Rectangle : {
            auto rect = dynamic_cast<Ptr<ERectangle> >(shape);
            merger->second->AddObject(netId, rect->shape);
            break;
        }
        case EShapeType::Path : {
            merger->second->AddObject(netId, shape->GetContour());
            break;
        }
        case EShapeType::Circle : {
            merger->second->AddObject(netId, shape->GetContour());
            break;
        }
        case EShapeType::Polygon : {
            auto polygon = dynamic_cast<Ptr<EPolygon> >(shape);
            merger->second->AddObject(netId, polygon->shape);
            break;
        }
        case EShapeType::PolygonWithHoles : {
            auto pwh = dynamic_cast<Ptr<EPolygonWithHoles> >(shape);
            merger->second->AddObject(netId, pwh->shape);
            break;
        }
        case EShapeType::FromTemplate : {
            if(shape->hasHole())
                merger->second->AddObject(netId, shape->GetPolygonWithHoles());
            else merger->second->AddObject(netId, shape->GetContour());
            break;
        }
        default : {
            GENERIC_ASSERT(false)
        }
    }
    return true;
}

#ifdef BOOST_GIL_IO_PNG_SUPPORT
ECAD_INLINE bool ELayoutPolygonMerger::WritePngFiles(const std::string & filename, size_t width)
{
    auto dir = filesystem::DirName(filename);
    if(!filesystem::PathExists(dir))
        filesystem::CreateDir(dir);
    
    bool res = true;
    for(const auto & merger : m_mergers) {
        std::string filePath = filename + '_' + std::to_string(static_cast<int>(merger.first)) + ".png";
        /*res = res && */WritePngFileForOneLayer(filePath, merger.second.get(), width);
    }
    return res;   
}

ECAD_INLINE bool ELayoutPolygonMerger::WritePngFileForOneLayer(const std::string & filename, Ptr<LayerMerger> merger, size_t width)
{
    using PolygonData = typename LayerMerger::PolygonData;

    std::list<CPtr<PolygonData> > polygons;
    merger->GetAllPolygons(polygons);

    std::vector<Polygon2D<ECoord> > outs;
    outs.reserve(polygons.size());
    for(auto polygon : polygons) {
        outs.push_back(polygon->solid);
        for(const auto & hole : polygon->holes) {
            outs.push_back(hole);
        }
    }
    return GeometryIO::WritePNG<Polygon2D<ECoord> >(filename, outs.begin(), outs.end(), width);
}
#endif//BOOST_GIL_IO_PNG_SUPPORT

ECAD_INLINE bool ELayoutPolygonMerger::WriteVtkFiles(const std::string & filename)
{
    auto dir = filesystem::DirName(filename);
    if(!filesystem::PathExists(dir))
        filesystem::CreateDir(dir);
    
    bool res = true;
    for(const auto & merger : m_mergers) {
        std::string filePath = filename + '_' + std::to_string(static_cast<int>(merger.first)) + ".vtk";
        res = res && WriteVtkFileForOneLayer(filePath, merger.second.get());
    }
    return res;
}

ECAD_INLINE bool ELayoutPolygonMerger::WriteVtkFileForOneLayer(const std::string & filename, Ptr<LayerMerger> merger)
{
    using PolygonData = typename LayerMerger::PolygonData;

    std::list<CPtr<PolygonData> > polygons;
    merger->GetAllPolygons(polygons);

    std::vector<Polygon2D<ECoord> > outs;
    outs.reserve(polygons.size());
    for(auto polygon : polygons) {
        outs.push_back(polygon->solid);
        for(const auto & hole : polygon->holes) {
            outs.push_back(hole);
        }
    }
    return GeometryIO::WriteVTK<Polygon2D<ECoord> >(filename, outs.begin(), outs.end());
}

ECAD_INLINE bool ELayoutPolygonMerger::WriteDomDmcFiles(const std::string & filename)
{
    auto dir = filesystem::DirName(filename);
    if(!filesystem::PathExists(dir))
        filesystem::CreateDir(dir);

    std::string dom = filename + ".dom";
    std::string dmc = filename + ".dmc";
    std::fstream f_dom(dom, std::ios::out | std::ios::trunc);
    std::fstream f_dmc(dmc, std::ios::out | std::ios::trunc);
    if(f_dom.bad() || f_dmc.bad()) return false;

    f_dom << std::setiosflags(std::ios::fixed) << std::setprecision(6);
    f_dmc << std::setiosflags(std::ios::fixed) << std::setprecision(6);

    for(const auto & merger : m_mergers)
        WriteDomDmcForOneLayer(f_dom, f_dmc, merger.first, merger.second.get());

    f_dom.close();
    f_dmc.close();

    return true;
}

ECAD_INLINE void ELayoutPolygonMerger::WriteDomDmcForOneLayer(std::fstream & dom, std::fstream & dmc, ELayerId layerId, Ptr<LayerMerger> merger)
{
    using PolygonData = typename LayerMerger::PolygonData;
    int lyrId = static_cast<int>(layerId);
    EValue scale = m_layout->GetCoordUnits().Scale2Unit();
    auto writeOnePolygon = [this, &dom, &dmc, &scale, &lyrId](const std::string & lyrName, int netId, const std::string & netName, const Polygon2D<ECoord> & p, bool isHole)
    {
        char s(32);
        auto box = geometry::Extent(p);
        int solidOrHole = isHole ? 0 : 1;
        //<pt size> <solid/hole> <layer number> <lx> <rx> <ly> <uy> <net number> <net name> <signal layer number> <layer name>
        dmc << p.Size() << s << solidOrHole << s << lyrId << s;
        dmc << box[0][0] * scale << s << box[1][0] * scale << s;
        dmc << box[0][1] * scale << s << box[1][1] * scale << s;
        dmc << netId << s << netName << s << lyrId << s << lyrName << GENERIC_DEFAULT_EOL;
        for(size_t i = 0; i < p.Size(); ++i)
            dom << p[i][0] * scale << s << p[i][1] * scale << GENERIC_DEFAULT_EOL;
    };

    auto writeOnePolygonData = [this, &layerId, &writeOnePolygon](CPtr<PolygonData> pd)
    {
        auto net = m_layout->FindNetByNetId(pd->property);
        auto layer = m_layout->FindLayerByLayerId(layerId);
        if(net && layer) {
            auto netName = net->GetName();
            auto lyrName = layer->GetName();
            auto netId = static_cast<int>(net->GetNetId());
            writeOnePolygon(lyrName, netId, netName, pd->solid, false);
            for(const auto & hole : pd->holes)
                writeOnePolygon(lyrName, netId, netName, hole, true);
        }
    };

    std::list<CPtr<PolygonData> > polygons;
    merger->GetAllPolygons(polygons);
    for(auto polygon : polygons)
        writeOnePolygonData(polygon);
}

}//namespace euti
}//namespace ecad