#pragma once

#include "generic/geometry/Triangulation.hpp"
#include <boost/geometry/index/rtree.hpp>

#include "EThermalModel.h"
#include "EShape.h"

namespace ecad {

using namespace generic::geometry::tri;
class ILayoutView;
class IMaterialDef;
namespace emodel::etherm {

struct ECAD_API ECompactLayout
{
    std::vector<ENetId> nets;
    std::vector<ELayerId> layers;
    std::vector<EMaterialId> materials;
    std::vector<EPolygonData> polygons;
    std::unordered_map<size_t, ESimVal> powerBlocks;
    virtual ~ECompactLayout() = default;

    void AddShape(ENetId netId, ELayerId layerId, EMaterialId solidMat, EMaterialId holeMat, CPtr<EShape> shape);
    size_t AddPolygon(ENetId netId, ELayerId layerId, EMaterialId matId, EPolygonData polygon, bool isHole);
    void AddPowerBlock(ELayerId layerId, EMaterialId matId, EPolygonData polygon, ESimVal totalP);
    bool WriteImgView(std::string_view filename, size_t width = 512) const;

    void BuildLayerPolygonLUT();
    bool hasPolygon(ELayerId layerId) const;
    const std::vector<size_t> & GetLayerPolygons(ELayerId layerId) const;
    size_t SearchPolygon(ELayerId layerId, const EPoint2D & pt) const;
    const EPolygonData & GetLayoutBoundary() const;
private:
    
private:
    using RtVal = std::pair<EBox2D, size_t>;
    using Rtree = boost::geometry::index::rtree<RtVal, boost::geometry::index::rstar<8>>;
    std::unordered_map<ELayerId, std::shared_ptr<Rtree> > m_rtrees;
    std::unordered_map<ELayerId, std::vector<size_t> > m_lyrPolygons;
};

ECAD_API UPtr<ECompactLayout> makeCompactLayout(CPtr<ILayoutView> layout);

class ECAD_API EPrismaThermalModel : public EThermalModel
{
public:
    using PrismaTemplate = tri::Triangulation<EPoint2D>;
    struct PrismaElement
    {
        ENetId netId;
        EMaterialId matId;
        ESimVal avePower{0};
        size_t id{invalidIndex};
        size_t templateId{invalidIndex};
        inline static constexpr size_t TOP_NEIGHBOR_INDEX = 3;
        inline static constexpr size_t BOT_NEIGHBOR_INDEX = 4;
        std::array<size_t, 5> neighbors = {noNeighbor, noNeighbor, noNeighbor, noNeighbor, noNeighbor};//[edge1, edge2, edge3, top, bot];
    };

    struct PrismaLayer
    {
        ELayerId layerId;
        FCoord elevation;
        FCoord thickness;
        EMaterialId conductingMatId;
        EMaterialId dielectricMatId;
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
    
    ESimVal uniformBcSide{0};
    BCType sideBCType{BCType::HTC};
    PrismaTemplate prismaTemplate;
    std::vector<PrismaLayer> layers;
    PrismaLayer & AppendLayer(PrismaLayer layer);

    void Build(EValue scaleH2Unit, EValue scale2Meter);  
    EValue Scale2Meter() const;  
    size_t TotalLayers() const;
    size_t TotalElements() const;
    size_t GlobalIndex(size_t lyrIndex, size_t eleIndex) const;
    std::pair<size_t, size_t> LocalIndex(size_t index) const;//[lyrIndex, eleIndex]

    const std::vector<FPoint3D> GetPoints() const { return m_points; }
    const PrismaInstance & operator[] (size_t index) const { return m_prismas[index]; }

    bool NeedIteration() const { return false; } //wbtest,todo
    bool isTopLayer(size_t lyrIndex) const;
    bool isBotLayer(size_t lyrIndex) const;
    FPoint3D GetPoint(size_t lyrIndex, size_t eleIndex, size_t vtxIndex) const;

private:
    EValue m_scaleH2Unit;
    EValue m_scale2Meter;
    std::vector<FPoint3D> m_points;
    std::vector<PrismaInstance> m_prismas;
    std::vector<size_t> m_indexOffset;
};

ECAD_ALWAYS_INLINE EValue EPrismaThermalModel::Scale2Meter() const
{
    return m_scale2Meter;
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::TotalLayers() const
{
    return layers.size();
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::TotalElements() const
{
    return m_indexOffset.back();
}

ECAD_ALWAYS_INLINE size_t EPrismaThermalModel::GlobalIndex(size_t lyrIndex, size_t eleIndex) const
{
    return m_indexOffset[lyrIndex] + eleIndex;
}

ECAD_ALWAYS_INLINE std::pair<size_t, size_t> EPrismaThermalModel::LocalIndex(size_t index) const
{
    size_t lyrIdex = 0;
    while (not (m_indexOffset[lyrIdex] <= index && index < m_indexOffset[lyrIdex + 1])) lyrIdex++;
    return std::make_pair(lyrIdex, index - m_indexOffset[lyrIdex]);
}

ECAD_ALWAYS_INLINE bool EPrismaThermalModel::isTopLayer(size_t lyrIndex) const
{
    return 0 == lyrIndex;
}

ECAD_ALWAYS_INLINE bool EPrismaThermalModel::isBotLayer(size_t lyrIndex) const
{
    return lyrIndex + 1 == TotalLayers();
}

} // namespace emodel::etherm
} // namespace ecad