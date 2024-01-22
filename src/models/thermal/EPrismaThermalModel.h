#pragma once

#include "generic/geometry/Triangulation.hpp"
#include <boost/geometry/index/rtree.hpp>

#include "EThermalModel.h"
#include "EShape.h"

namespace ecad {

using namespace generic::geometry::tri;
class IComponent;
class ILayoutView;
class IMaterialDef;
class IMaterialDefCollection;

namespace utils {
class ELayoutRetriever;
} // namespace utils

namespace model {

struct ECAD_API ECompactLayout
{
    using Height = int;
    using LayerRange = std::pair<Height, Height>;
    static bool isValid(const LayerRange & range) { return range.first > range.second; }
    struct PowerBlock
    {
        size_t polygon;
        Height position;
        LayerRange range;
        EFloat powerDensity;
        PowerBlock(size_t polygon, Height position, LayerRange range, EFloat powerDensity);
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

    std::vector<ENetId> nets;
    std::vector<LayerRange> ranges; 
    std::vector<Bondwire> bondwires;
    std::vector<EMaterialId> materials;
    std::vector<EPolygonData> polygons;
    std::vector<EPoint2D> steinerPoints;
    bool addCircleCenterAsSteinerPoints{false};
    std::unordered_map<size_t, PowerBlock> powerBlocks;

    explicit ECompactLayout(CPtr<ILayoutView> layout, EFloat vScale2Int);
    virtual ~ECompactLayout() = default;
    void AddShape(ENetId netId, EMaterialId solidMat, EMaterialId holeMat, CPtr<EShape> shape, EFloat elevation, EFloat thickness);
    size_t AddPolygon(ENetId netId, EMaterialId matId, EPolygonData polygon, bool isHole, EFloat elevation, EFloat thickness);
    bool AddPowerBlock(EMaterialId matId, EPolygonData polygon, EFloat totalP, EFloat elevation, EFloat thickness, EFloat position = 0.1);
    void AddComponent(CPtr<IComponent> component);    
    bool WriteImgView(std::string_view filename, size_t width = 512) const;

    void BuildLayerPolygonLUT();

    size_t TotalLayers() const;
    bool hasPolygon(size_t layer) const;
    size_t SearchPolygon(size_t layer, const EPoint2D & pt) const;
    bool GetLayerHeightThickness(size_t layer, EFloat & elevation, EFloat & thickness) const;
    const EPolygonData & GetLayoutBoundary() const;
private:
    Height GetHeight(EFloat height) const;
    LayerRange GetLayerRange(EFloat elevation, EFloat thickness) const;
private:
    using RtVal = std::pair<EBox2D, size_t>;
    using Rtree = boost::geometry::index::rtree<RtVal, boost::geometry::index::rstar<8>>;
    std::unordered_map<size_t, std::shared_ptr<Rtree> > m_rtrees;
    std::unordered_map<size_t, std::vector<size_t> > m_lyrPolygons;
    std::unordered_map<Height, size_t> m_height2Index;
    std::vector<Height> m_layerOrder;
    EFloat m_vScale2Int;

    CPtr<ILayoutView> m_layout;
    std::unique_ptr<utils::ELayoutRetriever> m_retriever;
};

ECAD_API UPtr<ECompactLayout> makeCompactLayout(CPtr<ILayoutView> layout);

class ECAD_API EPrismaThermalModel : public EThermalModel
{
public:
    struct LineElement
    {
        ENetId netId;
        EMaterialId matId;
        EFloat radius{0};
        EFloat current{0};
        size_t id{invalidIndex};
        std::array<size_t, 2> endPoints;
        std::array<std::vector<size_t>, 2> neighbors;//global index
    };

    using PrismaTemplate = tri::Triangulation<EPoint2D>;
    struct PrismaElement
    {
        ENetId netId;
        EMaterialId matId;
        EFloat avePower{0};
        size_t id{invalidIndex};
        size_t templateId{invalidIndex};
        inline static constexpr size_t TOP_NEIGHBOR_INDEX = 3;
        inline static constexpr size_t BOT_NEIGHBOR_INDEX = 4;
        std::array<size_t, 5> neighbors = {noNeighbor, noNeighbor, noNeighbor, noNeighbor, noNeighbor};//[edge1, edge2, edge3, top, bot];
    };

    struct PrismaLayer
    {
        size_t id;
        EFloat elevation;
        EFloat thickness;
        std::vector<PrismaElement> elements;
        
        PrismaElement & operator[] (size_t index) { return elements[index]; }
        const PrismaElement & operator[] (size_t index) const { return elements.at(index); }

        PrismaElement & AddElement(size_t templateId)
        {
            auto & ele = elements.emplace_back(PrismaElement{});
            ele.id = elements.size() - 1;
            ele.templateId = templateId;
            return ele;
        }

        size_t TotalElements() const { return elements.size(); }
    };

    struct PrismaInstance
    {
        CPtr<PrismaLayer> layer{nullptr};
        CPtr<PrismaElement> element{nullptr};
        std::array<size_t, 6> vertices;//top, bot
        std::array<size_t, 5> neighbors = {noNeighbor, noNeighbor, noNeighbor, noNeighbor, noNeighbor};//[edge1, edge2, edge3, top, bot];
    };
    
    PrismaTemplate prismaTemplate;
    std::vector<PrismaLayer> layers;
    explicit EPrismaThermalModel(CPtr<ILayoutView> layout);
    virtual ~EPrismaThermalModel() = default;

    CPtr<IMaterialDefCollection> GetMaterialLibrary() const;

    PrismaLayer & AppendLayer(PrismaLayer layer);
    LineElement & AddLineElement(FPoint3D start, FPoint3D end, ENetId netId, EMaterialId matId, EFloat radius, EFloat current);

    void BuildPrismaModel(EFloat scaleH2Unit, EFloat scale2Meter);
    void AddBondWire(const ECompactLayout::Bondwire & bondwire);
    EFloat Scale2Meter() const;  
    size_t TotalLayers() const;
    size_t TotalElements() const;
    size_t TotalLineElements() const;
    size_t TotalPrismaElements() const;
    size_t GlobalIndex(size_t lineIdx) const;
    size_t GlobalIndex(size_t lyrIndex, size_t eleIndex) const;
    std::pair<size_t, size_t> PrismaLocalIndex(size_t index) const;//[lyrIndex, eleIndex]
    size_t LineLocalIndex(size_t index) const;
    bool isPrima(size_t index) const;

    const std::vector<FPoint3D> & GetPoints() const { return m_points; }
    const FPoint3D & GetPoint(size_t index) const { return m_points.at(index); }
    const PrismaInstance & GetPrisma(size_t index) const { return m_prismas.at(index); }
    const LineElement & GetLine(size_t index) const { return m_lines.at(index); }

    bool NeedIteration() const { return true; }
    bool isTopLayer(size_t lyrIndex) const;
    bool isBotLayer(size_t lyrIndex) const;
    size_t AddPoint(FPoint3D point);
    FPoint3D GetPoint(size_t lyrIndex, size_t eleIndex, size_t vtxIndex) const;

    std::vector<size_t> SearchPrismaInstances(size_t layer, const EPoint2D & pt) const;//todo, eff

    EModelType GetModelType() const { return EModelType::ThermalPrisma; }
private:
    EFloat m_scaleH2Unit;
    EFloat m_scale2Meter;
    CPtr<ILayoutView> m_layout;
    std::vector<FPoint3D> m_points;
    std::vector<LineElement> m_lines;
    std::vector<PrismaInstance> m_prismas;
    std::vector<size_t> m_indexOffset;
};

ECAD_ALWAYS_INLINE EFloat EPrismaThermalModel::Scale2Meter() const
{
    return m_scale2Meter;
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::TotalLayers() const
{
    return layers.size();
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::TotalElements() const
{
    return TotalLineElements() + TotalPrismaElements();
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::TotalLineElements() const
{
    return m_lines.size();
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::TotalPrismaElements() const
{
    return m_indexOffset.back();
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::GlobalIndex(size_t lineIdx) const
{
    return m_indexOffset.back() + lineIdx;
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::GlobalIndex(size_t lyrIndex, size_t eleIndex) const
{
    return m_indexOffset[lyrIndex] + eleIndex;
}

ECAD_ALWAYS_INLINE std::pair<size_t, size_t> EPrismaThermalModel::PrismaLocalIndex(size_t index) const
{
    size_t lyrIdex = 0;
    while (not (m_indexOffset[lyrIdex] <= index && index < m_indexOffset[lyrIdex + 1])) lyrIdex++;
    return std::make_pair(lyrIdex, index - m_indexOffset[lyrIdex]);
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::LineLocalIndex(size_t index) const
{
    ECAD_ASSERT(index >= m_indexOffset.back());
    return index - m_indexOffset.back();
}

ECAD_ALWAYS_INLINE bool EPrismaThermalModel::isPrima(size_t index) const
{
    return index < m_indexOffset.back();
}

ECAD_ALWAYS_INLINE bool EPrismaThermalModel::isTopLayer(size_t lyrIndex) const
{
    return 0 == lyrIndex;
}

ECAD_ALWAYS_INLINE bool EPrismaThermalModel::isBotLayer(size_t lyrIndex) const
{
    return lyrIndex + 1 == TotalLayers();
}

} // namespace model
} // namespace ecad