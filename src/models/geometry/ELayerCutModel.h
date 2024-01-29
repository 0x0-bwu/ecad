#pragma once
#include "ECadCommon.h"
#include "interfaces/IModel.h"

namespace ecad::model {
namespace utils { class ELayerCutModelBuilder; }
class ECAD_API ELayerCutModel : public IModel
{
public:
    friend class utils::ELayerCutModelBuilder;
    using Height = int;
    using LayerRange = std::pair<Height, Height>;
    struct PowerBlock
    {
        size_t polygon;
        LayerRange range;
        EFloat powerDensity;
        PowerBlock(size_t polygon, LayerRange range, EFloat powerDensity);
    };

    struct Bondwire
    {
        ENetId netId;
        EFloat radius{0};
        EMaterialId matId;
        EFloat current{0};
        std::array<size_t, 2> layer;
        std::vector<EFloat> heights;
        std::vector<EPoint2D> pt2ds;
    };
    
    virtual ~ELayerCutModel() = default;
    bool WriteImgView(std::string_view filename, size_t width = 512) const;

    void BuildLayerPolygonLUT();

    size_t TotalLayers() const;
    bool hasPolygon(size_t layer) const;
    bool GetLayerHeightThickness(size_t layer, EFloat & elevation, EFloat & thickness) const;
    size_t GetLayerIndexByHeight(Height height) const;
    const EPolygonData & GetLayoutBoundary() const;

    virtual EModelType GetModelType() const { return EModelType::LayerCut; }

    static bool isValid(const LayerRange & range) { return range.first > range.second; }

protected:
    //data
    std::vector<ENetId> m_nets;
    std::vector<LayerRange> m_ranges; 
    std::vector<Bondwire> m_bondwires;
    std::vector<EMaterialId> m_materials;
    std::vector<EPolygonData> m_polygons;
    std::vector<EPoint2D> m_steinerPoints;
    std::unordered_map<size_t, PowerBlock> m_powerBlocks;

    std::unordered_map<size_t, std::vector<size_t> > m_lyrPolygons;
    std::unordered_map<Height, size_t> m_height2Index;
    std::vector<Height> m_layerOrder;
};
} // namespace ecad::model 