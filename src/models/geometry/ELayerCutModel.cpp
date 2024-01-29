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

ECAD_INLINE void ELayerCutModel::BuildLayerPolygonLUT(EFloat vTransitionRatio)
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
                iter = m_lyrPolygons.emplace(layer, new std::vector<size_t>{}).first;
            iter->second->emplace_back(i);
        }
    }

    if (generic::math::GT<EFloat>(vTransitionRatio, 1)) {
        std::list<LayerRange> ranges;
        for (size_t i = 0; i < m_layerOrder.size() - 1; ++i)
            ranges.emplace_back(m_layerOrder.at(i), m_layerOrder.at(i + 1));

        bool sliced = true;
        while (sliced) {
            sliced = SliceOverheightLayers(ranges, vTransitionRatio);
        }
        m_layerOrder.clear();
        m_layerOrder.reserve(ranges.size() + 1);
        for (const auto & range : ranges)
            m_layerOrder.emplace_back(range.first);
        m_layerOrder.emplace_back(ranges.back().second);
        std::unordered_map<size_t, SPtr<std::vector<size_t>> > lyrPolygons;
        for (size_t i = 0; i < m_layerOrder.size() - 1; ++i) {
            auto iter = m_height2Index.find(m_layerOrder.at(i));
            if (iter != m_height2Index.cend())
                lyrPolygons.emplace(i, m_lyrPolygons.at(iter->second));
            else lyrPolygons.emplace(i, lyrPolygons.at(i - 1));
        }
        std::swap(m_lyrPolygons, lyrPolygons);
        m_height2Index.clear();
        for (size_t i = 0; i < m_layerOrder.size(); ++i)
            m_height2Index.emplace(m_layerOrder.at(i), i);
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

ECAD_INLINE bool ELayerCutModel::SliceOverheightLayers(std::list<LayerRange> & ranges, EFloat ratio)
{
    auto slice = [](const LayerRange & r)
    {
        Height mid = std::round(0.5 * (r.first + r.second));
        auto res = std::make_pair(r, r);
        res.first.second = mid;
        res.second.first = mid;
        return res;
    };

    bool sliced = false;
    auto curr = ranges.begin();
    for (;curr != ranges.end();){
        auto currH = Thickness(*curr); ECAD_ASSERT(currH > 0)
        if (curr != ranges.begin()) {
            auto prev = curr; prev--;
            auto prevH = Thickness(*prev);
            auto r = currH / (EFloat)prevH;
            if (generic::math::GT<EFloat>(r, ratio)){
                auto [top, bot] = slice(*curr);
                curr = ranges.erase(curr);
                curr = ranges.insert(curr, bot);
                curr = ranges.insert(curr, top);
                sliced = true;
                curr++;
            }
        }
        auto next = curr; next++;
        if (next != ranges.end()){
            auto nextH = Thickness(*next); ECAD_ASSERT(nextH > 0)
            auto r = currH / (EFloat)nextH;
            if (generic::math::GT<EFloat>(r, ratio)) {
                auto [top, bot] = slice(*curr);
                curr = ranges.erase(curr);
                curr = ranges.insert(curr, bot);
                curr = ranges.insert(curr, top);
                sliced = true;
                curr++;
            }
        }
        curr++;
    }
    return sliced;   
}

}//namespace ecad::model::geometry
