#include "EMetalFractionMapping.h"

#if defined(ECAD_DEBUG_MODE)
#include "generic/tools/Color.hpp"
#endif

#include "generic/geometry/Transform.hpp"
#include "generic/tools/StringHelper.hpp"
#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"

#include "interfaces/ILayoutView.h"
#include "interfaces/IPrimitive.h"
#include "interfaces/ILayer.h"

#include "EDataMgr.h"
#include "EShape.h"

namespace ecad {
namespace eutils {
using namespace generic::geometry;

ECAD_INLINE ELayerMetalFractionMapper::ELayerMetalFractionMapper(const Setting & settings, ELayerMetalFraction & fraction, ELayerId layerId)
 : m_layerId(layerId)
 , m_settings(settings)
 , m_fraction(fraction)
{
}

ECAD_INLINE ELayerMetalFractionMapper::~ELayerMetalFractionMapper()
{
}

ECAD_INLINE void ELayerMetalFractionMapper::GenerateMetalFractionMapping(CPtr<ILayoutView> layout, const MapCtrl & ctrl)
{
    m_solids.clear();
    m_holes.clear();

    bool bSelNet = m_settings.selectNets.size() > 0;
    const auto & selNets = m_settings.selectNets;
    auto primIter = layout->GetPrimitiveIter();
    while(auto primitive = primIter->Next()){
        auto layer = primitive->GetLayer();
        if(noLayer == layer) continue;
        if(m_layerId != layer) continue;

        auto netId = primitive->GetNet();
        if(bSelNet && !selNets.count(netId)) continue;

        auto geom = primitive->GetGeometry2DFromPrimitive();
        if(nullptr == geom) continue;
        auto shape = geom->GetShape();
        if(nullptr == shape) continue;

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
    Mapping(ctrl);
}

ECAD_INLINE void ELayerMetalFractionMapper::Mapping(const MapCtrl & ctrl)
{
    auto solidBlend = [](typename ELayerMetalFraction::ResultType & res, const Product & p) { res += p.ratio; };
    auto holeBlend =  [](typename ELayerMetalFraction::ResultType & res, const Product & p) { res -= p.ratio; };

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
    for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < height; ++j) {
            auto & res = m_fraction(i, j);
            if(res < 0.0f) res = 0.0f;
            if(res > 1.0f) res = 1.0f;
        }
    }
}

ECAD_INLINE ELayoutMetalFractionMapper::ELayoutMetalFractionMapper(EMetalFractionMappingSettings settings)
 : m_settings(settings)
{
}

ECAD_INLINE ELayoutMetalFractionMapper::~ELayoutMetalFractionMapper()
{
}

ECAD_INLINE bool ELayoutMetalFractionMapper::GenerateMetalFractionMapping(Ptr<ILayoutView> layout)
{
    ECAD_EFFICIENCY_TRACK("metal fraction mapping");
    
    if(m_settings.mergeGeomBeforeMapping) {
        ELayoutPolygonMergeSettings settings;
        settings.selectNets = m_settings.selectNets;
        layout->MergeLayerPolygons(settings);
    }

    auto coordUnits = layout->GetCoordUnits();
    m_mfInfo.reset(new EMetalFractionInfo);
    m_mfInfo->grid = m_settings.grid;
    m_mfInfo->coordUnits = coordUnits;
    
    auto boundary = layout->GetBoundary();
    auto bbox = boundary->GetBBox();

    m_mfInfo->origin = bbox;
    bbox[0][0] -= coordUnits.toCoord(m_settings.regionExtLeft);
    bbox[0][1] -= coordUnits.toCoord(m_settings.regionExtBot);
    bbox[1][0] += coordUnits.toCoord(m_settings.regionExtRight);
    bbox[1][1] += coordUnits.toCoord(m_settings.regionExtTop);
    m_mfInfo->stride[0] = std::round(FCoord(bbox.Length()) / m_settings.grid[0]);
    m_mfInfo->stride[1] = std::round(FCoord(bbox.Width())  / m_settings.grid[1]);

    //update UR bounds
    bbox[1][0] = bbox[0][0] + m_mfInfo->stride[0] * m_mfInfo->grid[0];
    bbox[1][1] = bbox[0][1] + m_mfInfo->stride[1] * m_mfInfo->grid[1];
    m_mfInfo->extension = bbox;


    m_result.reset(new ELayoutMetalFraction);
    MapCtrl ctrl(bbox, {m_mfInfo->stride[0], m_mfInfo->stride[1]}, EDataMgr::Instance().DefaultThreads());
    
    auto layerIter = layout->GetLayerIter();
    //stackuplayer
    while(auto * layer = layerIter->Next()){
        auto * stackupLayer = layer->GetStackupLayerFromLayer();
        if(nullptr == stackupLayer) continue;
        bool isMetal = layer->GetLayerType() == ELayerType::ConductingLayer;
        auto layerFraction = std::make_shared<ELayerMetalFraction>(m_mfInfo->grid[0], m_mfInfo->grid[1], 0.0);
        ELayerMetalFractionMapper mapper(m_settings, *layerFraction, layer->GetLayerId());
        mapper.GenerateMetalFractionMapping(layout, ctrl);
        m_result->push_back(layerFraction);

        EStackupLayerInfo lyrInfo{ isMetal, stackupLayer->GetElevation(), stackupLayer->GetThickness(), layer->GetName() };
        m_mfInfo->layers.emplace_back(std::move(lyrInfo));
    }

    bool res = true;
    if(!m_settings.outFile.empty()) {
        res = WriteResult2File(coordUnits.Scale2Unit());
        auto rgbaFunc = [](float d) {
            int r, g, b, a = 255;
            d = std::max<float>(0, std::min<float>(1, d));
            generic::color::RGBFromScalar(d, r, g, b);
            return std::make_tuple(r, g, b, a);
        };

        std::string dirPath  = generic::filesystem::DirName(m_settings.outFile);
        std::string fileName = generic::filesystem::FileName(m_settings.outFile);
        for(size_t index = 0; index < m_result->size(); ++index){
            std::string filepng = dirPath + GENERIC_FOLDER_SEPS + fileName + "_" + std::to_string(index) + ".png";
            m_result->at(index)->WriteImgProfile(filepng, rgbaFunc);
        }
    }
    return res;
}

ECAD_INLINE CPtr<ELayoutMetalFraction> ELayoutMetalFractionMapper::GetLayoutMetalFraction() const
{
    if(nullptr == m_result) return nullptr;
    return m_result.get();
}

ECAD_INLINE CPtr<EMetalFractionInfo> ELayoutMetalFractionMapper::GetMetalFractionInfo() const
{
    if(nullptr == m_mfInfo) return nullptr;
    return m_mfInfo.get();
}

ECAD_INLINE bool ELayoutMetalFractionMapper::WriteResult2File(double scale)
{
    //LAYERS 7
    //LAYNERNAME ISMETAL ELEVATION(m) THICKNESS(m)
    //ORIGIN     LLX(m)  LLY(m) URX(m) URY(m)
    //EXTENSION  LLX(m)  LLY(m) URX(m) URY(m)
    //TILE       WIDTH   HEIGHT
    //RESOLUTION X(m)    Y(m)
    //X Y VALUE1 VALUE2  ...

    using namespace generic::fmt;
    if(nullptr == m_mfInfo) return false;
    if(!m_fileHelper.Open(m_settings.outFile)) return false;

    const auto & info = *m_mfInfo;

    std::stringstream ss;
    char sp(32);
    size_t layers = info.layers.size();
    ss << "LAYERS" << sp << layers << std::endl;
    ss << std::endl;
    for(const auto & layer : info.layers){
        ss << "LAYER" << sp << layer.name << sp << (layer.isMetal ? 1 : 0) << sp;
        ss << Fmt2Str("%1$12.9f %2$12.9f", layer.elevation * scale, layer.thickness * scale) << std::endl;
    }
    ss << std::endl;
    ss << "ORIGIN" << sp;
    ss << Fmt2Str("%1$12.9f %2$12.9f %3$12.9f %4$12.9f", info.origin[0][0] * scale, info.origin[0][1] * scale,
                                                               info.origin[1][0] * scale, info.origin[1][1] * scale);
    ss << std::endl;
    ss << "EXTENSION" << sp;
    ss << Fmt2Str("%1$12.9f %2$12.9f %3$12.9f %4$12.9f", info.extension[0][0] * scale, info.extension[0][1] * scale,
                                                               info.extension[1][0] * scale, info.extension[1][1] * scale);
    ss << std::endl;
    ss << "TILE" << sp <<  info.grid[0] << sp << info.grid[1] << std::endl;
    ss << "RESOLUTION" <<  Fmt2Str("%1$12.9f %2$12.9f", info.stride[0] * scale, info.stride[1] * scale) << std::endl;

    ss << std::endl;
    ss << std::setiosflags(std::ios::fixed) << std::setprecision(6);
    for(size_t i = 0; i < info.grid[0]; ++i){
        for(size_t j = 0; j < info.grid[1]; ++j){
            ss << i << sp << j;
            for(size_t k = 0; k < layers; ++k){
                ss << sp << (*(m_result->at(k)))(i, j);
            }
            ss << std::endl;
        }
    }

    m_fileHelper.Write(ss.str());
    m_fileHelper.Flush();
    m_fileHelper.Close();

#if defined(ECAD_DEBUG_MODE)

    size_t index = 0;
    auto rgbaFunc = [](float d) {
        int r, g, b, a = 255;
        generic::color::RGBFromScalar(d, r, g, b);
        return std::make_tuple(r, g, b, a);
    };

    std::string dirPath  = generic::filesystem::DirName(m_settings.outFile);
    std::string fileName = generic::filesystem::FileName(m_settings.outFile);
    for(index = 0; index < layers; ++index){
        std::string filepng = dirPath + GENERIC_FOLDER_SEPS + fileName + "_" + info.layers[index].name + ".png";
        m_result->at(index)->WriteImgProfile(filepng, rgbaFunc);
    }
#endif

    return true;
}

ECAD_INLINE bool WriteThermalProfile(const EMetalFractionInfo & info, const ELayoutMetalFraction & mf, const std::string & filename)
{
    if(info.layers.size() != mf.size()) return false;

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
    auto width = info.grid[0];
    auto height = info.grid[1];
    for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < height; ++j) {
            auto ll = ref + EPoint2D(info.stride[0] * i, info.stride[1] * j);
            auto ur = ll + EPoint2D(info.stride[0], info.stride[1]);
            auto box = EBox2D(ll, ur);

            out << index++;
            out << sp << unit.toUnit(box[0][0], um);
            out << sp << unit.toUnit(box[0][1], um);
            out << sp << unit.toUnit(box[1][0], um);
            out << sp << unit.toUnit(box[1][1], um);

            for (size_t k = 0; k < layerSize; ++k) {
                out << sp;
                out << (*mf[k])(i, j);
            }
            out << std::endl;
        }
    }
    return true;
}

}//namespace eutils
}//namespace ecad