#include "ELayerCutModel.h"

#include "generic/geometry/GeometryIO.hpp"

namespace ecad::model {

ECAD_INLINE ELayerCutModel::PowerBlock::PowerBlock(size_t polygon, LayerRange range, EFloat powerDensity)
 : polygon(polygon), range(std::move(range)), powerDensity(powerDensity)
{
}

ECAD_INLINE bool ELayerCutModel::WriteImgView(std::string_view filename, size_t width) const
{
    auto shapes = m_polygons;
    for (const auto & bw : m_bondwires)
        shapes.emplace_back(EPolygonData(bw.pt2ds));
    return generic::geometry::GeometryIO::WritePNG(filename, shapes.begin(), shapes.end(), width);
}

ECAD_INLINE void ELayerCutModel::BuildLayerPolygonLUT()
{
    m_layerOrder.clear();
    m_height2Index.clear();
    std::set<Height> heights;
    for (size_t i = 0; i < m_ranges.size(); ++i) {
        if (EMaterialId::noMaterial == m_materials.at(i)) continue;
        heights.emplace(m_ranges.at(i).first);
        heights.emplace(m_ranges.at(i).second);
        auto iter = m_powerBlocks.find(i);
        if (iter != m_powerBlocks.cend()) {
            heights.emplace(iter->second.range.first);
            heights.emplace(iter->second.range.second);
        }
    }
    m_layerOrder = std::vector(heights.begin(), heights.end());
    std::reverse(m_layerOrder.begin(), m_layerOrder.end());
    for (size_t i = 0; i < m_layerOrder.size(); ++i)
        m_height2Index.emplace(m_layerOrder.at(i), i);

    m_lyrPolygons.clear();
    for (size_t i = 0; i < m_polygons.size(); ++i) {

        const auto & range = m_ranges.at(i);
        size_t sLayer = m_height2Index.at(range.first);
        size_t eLayer = std::min(TotalLayers(), m_height2Index.at(range.second));
        for (size_t layer = sLayer; layer < eLayer; ++layer) {
            auto iter = m_lyrPolygons.find(layer);
            if (iter == m_lyrPolygons.cend())
                iter = m_lyrPolygons.emplace(layer, std::vector<size_t>{}).first;
            iter->second.emplace_back(i);
        }
    }

    for (auto & bw : m_bondwires) {
        bw.layer.front() = m_height2Index.at(GetHeight(bw.heights.front()));
        bw.layer.back() = m_height2Index.at(GetHeight(bw.heights.back()));
    }
}

ECAD_INLINE size_t ELayerCutModel::TotalLayers() const
{
    return m_layerOrder.size() - 1;
}

ECAD_INLINE bool ELayerCutModel::hasPolygon(size_t layer) const
{
    return m_lyrPolygons.count(layer);
}

ECAD_INLINE bool ELayerCutModel::GetLayerHeightThickness(size_t layer, EFloat & elevation, EFloat & thickness) const
{
    if (layer >= TotalLayers()) return false;
    elevation = EFloat(m_layerOrder.at(layer)) / m_vScale2Int;
    thickness = elevation - EFloat(m_layerOrder.at(layer + 1)) / m_vScale2Int;
    return true;
}

ECAD_INLINE size_t ELayerCutModel::GetLayerIndexByHeight(Height height) const
{
    auto iter = m_height2Index.find(height);
    if (iter == m_height2Index.cend()) return invalidIndex;
    return iter->second;
}

ECAD_INLINE const EPolygonData & ELayerCutModel::GetLayoutBoundary() const
{
    return m_polygons.front();
}

ECAD_INLINE ELayerCutModel::Height ELayerCutModel::GetHeight(EFloat height) const
{
    return std::round(height * m_vScale2Int);
}

ECAD_INLINE ELayerCutModel::LayerRange ELayerCutModel::GetLayerRange(EFloat elevation, EFloat thickness) const
{
    return LayerRange{GetHeight(elevation), GetHeight(elevation - thickness)};
}

}//namespace ecad::model::geometry
