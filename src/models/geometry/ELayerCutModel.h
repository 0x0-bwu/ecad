#pragma once
#include "ECadCommon.h"
#include "EShape.h"
#include "interfaces/IModel.h"

namespace ecad::model {
namespace utils { 
class ELayerCutModelQuery;
class ELayerCutModelBuilder;
} // namespace utils
class ECAD_API ELayerCutModel : public IModel
{
public:
    friend class utils::ELayerCutModelQuery;
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
    const std::unordered_map<size_t, PowerBlock> & GetAllPowerBlocks() const { return m_powerBlocks; }
    const std::vector<EPolygonData> & GetAllPolygonData() const { return m_polygons; }
    const std::vector<EPoint2D> & GetSteinerPoints() const { return m_steinerPoints; }
    const std::vector<Bondwire> GetAllBondwires() const { return m_bondwires; }
    EMaterialId GetMaterialId(size_t pid) const { return m_materials.at(pid); }
    ENetId GetNetId(size_t pid) const { return m_nets.at(pid); }

    Height GetHeight(EFloat height) const;
    LayerRange GetLayerRange(EFloat elevation, EFloat thickness) const;

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
    EFloat m_vScale2Int;
};
} // namespace ecad::model 