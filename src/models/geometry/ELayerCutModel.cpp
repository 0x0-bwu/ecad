#include "ELayerCutMode"

namespace ecad::model {

ECAD_INLINE ELayerCutModel::PowerBlock::PowerBlock(size_t polygon, LayerRange range, EFloat powerDensity)
 : polygon(polygon), range(std::move(range)), powerDensity(powerDensity)
{
}

ECAD_INLINE bool ECompactLayout::WriteImgView(std::string_view filename, size_t width) const
{
    auto shapes = polygons;
    for (const auto & bw : bondwires)
        shapes.emplace_back(EPolygonData(bw.pt2ds));
    return generic::geometry::GeometryIO::WritePNG(filename, shapes.begin(), shapes.end(), width);
}

ECAD_INLINE void ECompactLayout::BuildLayerPolygonLUT()
{
    m_layerOrder.clear();
    m_height2Index.clear();
    std::set<Height> heights;
    for (size_t i = 0; i < ranges.size(); ++i) {
        if (EMaterialId::noMaterial == materials.at(i)) continue;
        heights.emplace(ranges.at(i).first);
        heights.emplace(ranges.at(i).second);
        auto iter = powerBlocks.find(i);
        if (iter != powerBlocks.cend()) {
            heights.emplace(iter->second.range.first);
            heights.emplace(iter->second.range.second);
        }
    }
    m_layerOrder = std::vector(heights.begin(), heights.end());
    std::reverse(m_layerOrder.begin(), m_layerOrder.end());
    for (size_t i = 0; i < m_layerOrder.size(); ++i)
        m_height2Index.emplace(m_layerOrder.at(i), i);

    m_rtrees.clear();
    m_lyrPolygons.clear();
    for (size_t i = 0; i < polygons.size(); ++i) {
        // if (EMaterialId::noMaterial == materials.at(i)) continue;

        const auto & range = ranges.at(i);
        size_t sLayer = m_height2Index.at(range.first);
        size_t eLayer = std::min(TotalLayers(), m_height2Index.at(range.second));
        for (size_t layer = sLayer; layer < eLayer; ++layer) {
            auto iter = m_lyrPolygons.find(layer);
            if (iter == m_lyrPolygons.cend())
                iter = m_lyrPolygons.emplace(layer, std::vector<size_t>{}).first;
            iter->second.emplace_back(i);
        }
    }

    for (auto & bw : bondwires) {
        bw.layer.front() = m_height2Index.at(GetHeight(bw.heights.front()));
        bw.layer.back() = m_height2Index.at(GetHeight(bw.heights.back()));
    }

    for (size_t layer = 0; layer < TotalLayers(); ++layer) {
        auto & rtree = m_rtrees.emplace(layer, std::make_shared<Rtree>()).first->second;
        for (auto i : m_lyrPolygons.at(layer)) {
            auto bbox = generic::geometry::Extent(polygons.at(i));
            rtree->insert(std::make_pair(bbox, i));
        }
    }
}

ECAD_INLINE size_t ECompactLayout::TotalLayers() const
{
    return m_layerOrder.size() - 1;
}

ECAD_INLINE bool ECompactLayout::hasPolygon(size_t layer) const
{
    return m_lyrPolygons.count(layer);
}

ECAD_INLINE size_t ECompactLayout::SearchPolygon(size_t layer, const EPoint2D & pt) const
{
    if (not hasPolygon(layer)) return invalidIndex;
    std::vector<RtVal> results;
    m_rtrees.at(layer)->query(boost::geometry::index::intersects(EBox2D(pt, pt)), std::back_inserter(results));
    if (results.empty()) return invalidIndex;
    auto cmp = [&](auto i1, auto i2){ return polygons.at(i1).Area() > polygons.at(i2).Area(); };
    std::priority_queue<size_t, std::vector<size_t>, decltype(cmp)> pq(cmp);
    for (const auto & result : results) {
        if (generic::geometry::Contains(polygons.at(result.second), pt))
            pq.emplace(result.second);
    }
    if (not pq.empty()) return pq.top();
    return invalidIndex;
}

ECAD_INLINE bool ECompactLayout::GetLayerHeightThickness(size_t layer, EFloat & elevation, EFloat & thickness) const
{
    if (layer >= TotalLayers()) return false;
    elevation = EFloat(m_layerOrder.at(layer)) / m_vScale2Int;
    thickness = elevation - EFloat(m_layerOrder.at(layer + 1)) / m_vScale2Int;
    return true;
}

ECAD_INLINE size_t ECompactLayout::GetLayerIndexByHeight(Height height) const
{
    auto iter = m_height2Index.find(height);
    if (iter == m_height2Index.cend()) return invalidIndex;
    return iter->second;
}

ECAD_INLINE const EPolygonData & ECompactLayout::GetLayoutBoundary() const
{
    return polygons.front();
}

ECAD_INLINE ECompactLayout::Height ECompactLayout::GetHeight(EFloat height) const
{
    return std::round(height * m_vScale2Int);
}

ECAD_INLINE ECompactLayout::LayerRange ECompactLayout::GetLayerRange(EFloat elevation, EFloat thickness) const
{
    return LayerRange{GetHeight(elevation), GetHeight(elevation - thickness)};
}

}//namespace ecad::model::geometry
