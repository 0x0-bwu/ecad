#ifndef ECAD_HEADER_ONLY
#include "EMetalFractionMapping.h"
#endif

#if defined(ECAD_DEBUG_MODE) && defined(BOOST_GIL_IO_PNG_SUPPORT)
#include "generic/tools/Color.hpp"
#endif

#include "generic/geometry/Transform.hpp"
#include "generic/tools/StringHelper.hpp"
#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"
#include "Interface.h"
namespace ecad {
namespace esim {
using namespace generic::geometry;

ECAD_INLINE LayereMetalFractionMapper::LayereMetalFractionMapper(const Setting & settings, LayoutMetalFraction & fraction, ELayerId layerId, bool isMetal)
 : m_id(layerId)
 , m_bMetal(isMetal)
 , m_settings(settings)
 , m_fraction(fraction)
{
}

ECAD_INLINE LayereMetalFractionMapper::~LayereMetalFractionMapper()
{
}

ECAD_INLINE void LayereMetalFractionMapper::GenerateMetalFractionMapping(CPtr<ILayoutView> layout, const MapCtrl & ctrl)
{
    m_solids.clear();
    m_holes.clear();

    bool bSelNet = m_settings.selectNets.size() > 0;
    const auto & selNets = m_settings.selectNets;
    auto primIter = layout->GetPrimitiveIter();
    while(auto primitive = primIter->Next()){
        auto layer = primitive->GetLayer();
        if(noLayer == layer) continue;
        if(m_id != layer) continue;

        auto geom = primitive->GetGeometry2DFromPrimitive();
        if(nullptr == geom) continue;
        auto shape = geom->GetShape();
        if(nullptr == shape) continue;

        auto net = primitive->GetNet();
        if(bSelNet && !selNets.count(static_cast<int>(net))) continue;

        if(shape->hasHole()){
            auto pwh = shape->GetPolygonWithHoles();
            m_solids.emplace_back(std::move(pwh.outline));
            for(auto & hole : pwh.holes)
                m_holes.emplace_back(std::move(hole));
        }
        else {
            auto polygon = shape->GetContour();
            m_solids.emplace_back(std::move(polygon));
        }
    }

    //need merge pads firstly
    if(!m_bMetal){
        auto psInstIter = layout->GetPadstackInstIter();
        while(auto psInst = psInstIter->Next()){

            auto net = psInst->GetNet();
            if(bSelNet && !selNets.count(static_cast<int>(net))) continue;

            ELayerId top, bot;
            psInst->GetLayerRange(top, bot);
            if(m_id < top || m_id > bot) continue;

            auto defData = psInst->GetPadstackDef()->GetPadstackDefData();
            if(nullptr == defData) continue;
            
            CPtr<EShape> shape;
            EPoint2D offset;
            EValue rotation;
            defData->GetViaParameters(shape, offset, rotation);

            if(nullptr == shape) continue;
            auto trans = psInst->GetTransform().GetTransform()
                       * makeShiftTransform2D<EValue>(offset)
                       * makeRotateTransform2D(rotation);
            if(shape->hasHole()){
                auto pwh = trans * shape->GetPolygonWithHoles();
                m_solids.emplace_back(std::move(pwh.outline));
                for(auto & hole : pwh.holes)
                    m_holes.emplace_back(std::move(hole));
            }
            else {
                auto polygon = trans * shape->GetContour();
                m_solids.emplace_back(std::move(polygon));
            }  
        }
    }

    Mapping(ctrl);
}

ECAD_INLINE void LayereMetalFractionMapper::Mapping(const MapCtrl & ctrl)
{
    const auto id = m_id;
    auto solidBlend = [&id](typename LayoutMetalFraction::ResultType & res, const Product & p) { res[id] += p.ratio; };
    auto holeBlend =  [&id](typename LayoutMetalFraction::ResultType & res, const Product & p) { res[id] -= p.ratio; };

    std::vector<CPtr<IntPolygon> > solids(m_solids.size());
    std::vector<CPtr<IntPolygon> > holes(m_holes.size());
    for(size_t i = 0; i < m_solids.size(); ++i) solids[i] = &m_solids[i];
    for(size_t i = 0; i < m_holes.size(); ++i) holes[i] = &m_holes[i];
    std::vector<Property> solidProps(m_solids.size(), nullptr);//property not used now
    std::vector<Property> holePorps(m_holes.size(), nullptr);//property not used now
    Factory::Map2Grid<Property>(solidProps, solids, ctrl, m_fraction, solidBlend);
    Factory::Map2Grid<Property>(holePorps, holes, ctrl, m_fraction, holeBlend);

    auto width = m_fraction.Width();
    auto height = m_fraction.Height();
    for(auto i = 0; i < width; ++i){
        for(auto j = 0; j < height; ++j){
            auto & res = m_fraction(i, j)[id];
            if(res < 0.0f) res = 0.0f;
            if(res > 1.0f) res = 1.0f;
        }
    }
}

ECAD_INLINE LayoutMetalFractionMapper::LayoutMetalFractionMapper(EMetalFractionMappingSettings settings)
 : m_settings(settings)
{
}

ECAD_INLINE LayoutMetalFractionMapper::~LayoutMetalFractionMapper()
{
}

ECAD_INLINE bool LayoutMetalFractionMapper::GenerateMetalFractionMapping(CPtr<ILayoutView> layout)
{
    ECAD_EFFICIENCY_TRACK("metal fraction mapping");
    
    m_mfInfo.reset(new MetalFractionInfo);
    m_mfInfo->grid = m_settings.grid;
    m_mfInfo->coordUnits = m_settings.coordUnits;
    
    auto boundary = layout->GetBoundary();
    auto bbox = boundary->GetBBox();

    m_mfInfo->origin = bbox;
    bbox[0][0] -= m_settings.coordUnits.toCoord(m_settings.regionExtLeft);
    bbox[0][1] -= m_settings.coordUnits.toCoord(m_settings.regionExtBot);
    bbox[1][0] += m_settings.coordUnits.toCoord(m_settings.regionExtRight);
    bbox[1][1] += m_settings.coordUnits.toCoord(m_settings.regionExtTop);
    m_mfInfo->stride[0] = std::round(FCoord(bbox.Length()) / m_settings.grid[0]);
    m_mfInfo->stride[1] = std::round(FCoord(bbox.Width())  / m_settings.grid[1]);

    //update UR bounds
    bbox[1][0] = bbox[0][0] + m_mfInfo->stride[0] * m_mfInfo->grid[0];
    bbox[1][1] = bbox[0][1] + m_mfInfo->stride[1] * m_mfInfo->grid[1];
    m_mfInfo->extension = bbox;
    m_result.reset(new LayoutMetalFraction(m_mfInfo->grid[0], m_mfInfo->grid[1]));

    auto layerCollection = layout->GetLayerCollection();
    auto layerSize = layerCollection->Size();
    auto resultSize = m_result->Size();
    for(size_t i = 0; i < resultSize; ++i)
        (*m_result)[i].assign(layerSize, 0);

    MapCtrl ctrl(bbox, {m_mfInfo->stride[0], m_mfInfo->stride[1]}, m_settings.threads);
    auto layerIter = layout->GetLayerIter();

    //stackuplayer
    while(auto * layer = layerIter->Next()){
        auto * stackupLayer = layer->GetStackupLayerFromLayer();
        if(nullptr == stackupLayer) continue;
        bool isMetal = layer->GetLayerType() == ELayerType::ConductingLayer;
        LayereMetalFractionMapper mapper(m_settings, *m_result, layer->GetLayerId(), isMetal);
        mapper.GenerateMetalFractionMapping(layout, ctrl);

        StackupLayerInfo lyrInfo{ isMetal, stackupLayer->GetElevation(), stackupLayer->GetThickness(), layer->GetName() };
        (m_mfInfo->layers).emplace_back(std::move(lyrInfo));
    }
    //further, via layer
    
    //return WriteResult2File();
    return true; 
}

ECAD_INLINE CPtr<LayoutMetalFraction> LayoutMetalFractionMapper::GetLayoutMetalFraction() const
{
    if(nullptr == m_result) return nullptr;
    return m_result.get();
}

ECAD_INLINE CPtr<MetalFractionInfo> LayoutMetalFractionMapper::GetMetalFractionInfo() const
{
    if(nullptr == m_mfInfo) return nullptr;
    return m_mfInfo.get();
}

ECAD_INLINE bool LayoutMetalFractionMapper::WriteResult2File()
{
    //LAYERS 7
    //LAYNERNAME ISMETAL ELEVATION(m) THICKNESS(m)
    //ORIGIN     LLX(m)  LLY(m) URX(m) URY(m)
    //EXTENSION  LLX(m)  LLY(m) URX(m) URY(m)
    //TILE       WIDTH   HEIGHT
    //RESOLUTION X(m)    Y(m)
    //X Y VALUE1 VALUE2  ...

    using namespace generic::format;
    if(nullptr == m_mfInfo) return false;
    if(!m_fileHelper.Open(m_settings.outFile)) return false;

    const auto & info = *m_mfInfo;
    auto scale = m_settings.coordUnits.Scale2Unit();

    std::stringstream ss;
    char sp(32);
    size_t layers = info.layers.size();
    ss << "LAYERS" << sp << layers << std::endl;
    ss << std::endl;
    for(const auto & layer : info.layers){
        ss << "LAYER" << sp << layer.name << sp << (layer.isMetal ? 1 : 0) << sp;
        ss << Format2String("%1$12.9f %2$12.9f", layer.elevation * scale, layer.thickness * scale) << std::endl;
    }
    ss << std::endl;
    ss << "ORIGIN" << sp;
    ss << Format2String("%1$12.9f %2$12.9f %3$12.9f %4$12.9f", info.origin[0][0] * scale, info.origin[0][1] * scale,
                                                               info.origin[1][0] * scale, info.origin[1][1] * scale);
    ss << std::endl;
    ss << "EXTENSION" << sp;
    ss << Format2String("%1$12.9f %2$12.9f %3$12.9f %4$12.9f", info.extension[0][0] * scale, info.extension[0][1] * scale,
                                                               info.extension[1][0] * scale, info.extension[1][1] * scale);
    ss << std::endl;
    ss << "TILE" << sp <<  info.grid[0] << sp << info.grid[1] << std::endl;
    ss << "RESOLUTION" <<  Format2String("%1$12.9f %2$12.9f", info.stride[0] * scale, info.stride[1] * scale) << std::endl;

    ss << std::endl;
    ss << std::setiosflags(std::ios::fixed) << std::setprecision(6);
    for(size_t i = 0; i < info.grid[0]; ++i){
        for(size_t j = 0; j < info.grid[1]; ++j){
            ss << i << sp << j;
            for(size_t k = 0; k < layers; ++k){
                ss << sp << (*m_result)(i, j)[k];
            }
            ss << std::endl;
        }
    }

    m_fileHelper.Write(ss.str());
    m_fileHelper.Flush();
    m_fileHelper.Close();

#if defined(ECAD_DEBUG_MODE) && defined(BOOST_GIL_IO_PNG_SUPPORT)

    size_t index = 0;
    auto rgbaFunc = [&index](const std::vector<float> & d) {
        int r, g, b, a = 255;
        generic::color::RGBFromScalar(d[index], r, g, b);
        return std::make_tuple(r, g, b, a);
    };

    std::string dirPath  = generic::filesystem::DirName(m_settings.outFile);
    std::string fileName = generic::filesystem::FileName(m_settings.outFile);
    for(index = 0; index < layers; ++index){
        std::string filepng = dirPath + GENERIC_FOLDER_SEPS + fileName + "_" + info.layers[index].name + ".png";
        m_result->WriteImgProfile(filepng, rgbaFunc);
    }
#endif

    return true;
}

ECAD_INLINE bool WriteThermalProfile(const MetalFractionInfo & info, const LayoutMetalFraction & mf, const std::string & filename)
{
    ECAD_ASSERT(info.grid[0] == mf.Width())
    ECAD_ASSERT(info.grid[1] == mf.Height())

    auto dir = generic::filesystem::DirName(filename);
    if(!generic::filesystem::PathExists(dir))
        generic::filesystem::CreateDir(dir);

    std::ofstream out(filename);
    if(!out.is_open()) return false;
    char sp(32);

    const auto & bbox = info.extension;
    const auto & unit = info.coordUnits;

    auto um = generic::unit::Length::Micrometer;
    out << "# Version 3.0" << std::endl;
    out << "# DIE" << sp;
    out << unit.toUnit(bbox[0][0], um) << sp;
    out << unit.toUnit(bbox[0][1], um) << sp;
    out << unit.toUnit(bbox[1][0], um) << sp;
    out << unit.toUnit(bbox[1][1], um) << std::endl;

    out << "# TILE" << sp << info.grid[0] << sp << info.grid[1] << std::endl;

    std::string layers;
    auto layerSize = info.layers.size();
    for(size_t i = 0; i < layerSize; ++i){
        layers += sp;
        const auto & layer = info.layers[i];
        if(!layer.name.empty()) layers.append(layer.name);
        else layers.append("LAYER" + std::to_string(i));
    }

    out << "# LAYER" << sp << layerSize << layers << std::endl;
    out << "# TileID X1(um) Y1(um) X2(um) Y2(um)" << layers << std::endl;

    auto index = 1;
    auto ref = bbox[0];
    auto width = mf.Width();
    auto height = mf.Height();
    for(auto i = 0; i < width; ++i){
        for(auto j = 0; j < height; ++j){
            auto ll = ref + EPoint2D(info.stride[0] * i, info.stride[1] * j);
            auto ur = ll + EPoint2D(info.stride[0], info.stride[1]);
            auto box = EBox2D(ll, ur);

            out << index++;
            out << sp << unit.toUnit(box[0][0], um);
            out << sp << unit.toUnit(box[0][1], um);
            out << sp << unit.toUnit(box[1][0], um);
            out << sp << unit.toUnit(box[1][1], um);

            const auto & values = mf(i, j);
            for(auto k = 0; k < layerSize; ++k){
                out << sp;
                if(k < values.size()) out << values[k];
                else out << 0;
            }
            out << std::endl;
        }
    }
    return true;
}

}//namespace esim
}//namespace ecad