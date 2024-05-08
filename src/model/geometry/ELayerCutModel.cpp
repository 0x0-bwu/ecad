#include "ELayerCutModel.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::model::ELayerCutModel)

#include "generic/geometry/GeometryIO.hpp"

namespace ecad::model {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ELayerCutModel::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayerCutModel, IModel>();
    ar & boost::serialization::make_nvp("settings", m_settings);
    ar & boost::serialization::make_nvp("nets", m_nets);
    ar & boost::serialization::make_nvp("ranges", m_ranges);
    ar & boost::serialization::make_nvp("bondwires", m_bondwires);
    ar & boost::serialization::make_nvp("materials", m_materials);
    ar & boost::serialization::make_nvp("polygons", m_polygons);
    ar & boost::serialization::make_nvp("steiner_points", m_steinerPoints);
    ar & boost::serialization::make_nvp("power_blocks", m_powerBlocks);
    ar & boost::serialization::make_nvp("layer_polygons", m_lyrPolygons);
    ar & boost::serialization::make_nvp("height_to_index", m_height2Index);
    ar & boost::serialization::make_nvp("layer_order", m_layerOrder);
    ar & boost::serialization::make_nvp("vertical_scale_to_int", m_vScale2Int);
}
    
ECAD_SERIALIZATION_FUNCTIONS_IMP(ELayerCutModel)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ELayerCutModel::PowerBlock::PowerBlock(size_t polygon, LayerRange range, EScenarioId scen, SPtr<ELookupTable1D> power)
 : polygon(polygon), range(std::move(range)), scen(scen), power(power)
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
        const auto & range = m_ranges.at(i);
        if (not range.isValid()) continue;
        heights.emplace(range.high);
        heights.emplace(range.low);
        auto iter = m_powerBlocks.find(i);
        if (iter != m_powerBlocks.cend()) {
            heights.emplace(iter->second.range.high);
            heights.emplace(iter->second.range.low);
        }
    }
    m_layerOrder = std::vector(heights.begin(), heights.end());
    std::reverse(m_layerOrder.begin(), m_layerOrder.end());
    for (size_t i = 0; i < m_layerOrder.size(); ++i)
        m_height2Index.emplace(m_layerOrder.at(i), i);

    m_lyrPolygons.clear();
    for (size_t i = 0; i < m_polygons.size(); ++i) {
        const auto & range = m_ranges.at(i);
        if (not range.isValid()) continue;
        size_t sLayer = m_height2Index.at(range.high);
        size_t eLayer = std::min(TotalLayers(), m_height2Index.at(range.low));
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
            m_layerOrder.emplace_back(range.high);
        m_layerOrder.emplace_back(ranges.back().low);
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

ECAD_INLINE std::vector<EPolygonData> ELayerCutModel::GetLayerPolygons(size_t layer) const
{
    auto indices = GetLayerPolygonIndices(layer);
    std::vector<EPolygonData> polygons; polygons.reserve(indices->size());
    std::transform(indices->begin(), indices->end(), std::back_inserter(polygons), [&](auto i){ return m_polygons.at(i); });
    return polygons;
}

ECAD_INLINE bool ELayerCutModel::SliceOverheightLayers(std::list<LayerRange> & ranges, EFloat ratio)
{
    auto slice = [](const LayerRange & r)
    {
        Height mid = std::round(0.5 * (r.high + r.low));
        auto res = std::make_pair(r, r);
        res.first.low = mid;
        res.second.high = mid;
        return res;
    };

    bool sliced = false;
    auto curr = ranges.begin();
    for (;curr != ranges.end();){
        auto currH = curr->Thickness(); ECAD_ASSERT(currH > 0)
        if (curr != ranges.begin()) {
            auto prev = curr; prev--;
            auto prevH = prev->Thickness();
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
            auto nextH = next->Thickness(); ECAD_ASSERT(nextH > 0)
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
