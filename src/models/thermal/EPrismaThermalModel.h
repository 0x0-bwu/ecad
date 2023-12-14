#pragma once

#include <boost/geometry/index/rtree.hpp>
#include "EThermalModel.h"
#include "EShape.h"

namespace generic::geometry::tri {
template <typename> class Triangulation;
} //namespace generic::geometry::tri
namespace ecad {

class ILayoutView;
class IMaterialDef;
namespace emodel::etherm {

struct ECAD_API ECompactLayout
{
    std::vector<ENetId> nets;
    std::vector<ELayerId> layers;
    std::vector<EMaterialId> materials;
    std::vector<EPolygonData> polygons;//front is layout boundary
    virtual ~ECompactLayout() = default;

    void AddShape(ENetId netId, ELayerId layerId, EMaterialId matId, CPtr<EShape> shape);
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
        size_t id{invalidIndex};
        size_t templateId{invalidIndex};
        inline static constexpr size_t TOP_NEIGHBOR_INDEX = 3;
        inline static constexpr size_t BOT_NEIGHBOR_INDEX = 4;
        std::array<size_t, BOT_NEIGHBOR_INDEX + 1> neighbors = {invalidIndex, invalidIndex, invalidIndex, invalidIndex, invalidIndex};//[edge1, edge2, edge3, top, bot];
    };

    struct PrismaLayer
    {
        ELayerId layerId;
        FCoord elevation;
        FCoord thickness;
        EMaterialId conductingMatId{static_cast<EMaterialId>(0)};//wbtest
        EMaterialId dielectricMatId{static_cast<EMaterialId>(0)};//wbtest
        std::vector<PrismaElement> elements;
        
        PrismaElement & AddElement(size_t templateId)
        {
            auto & ele = elements.emplace_back(PrismaElement{});
            ele.id = elements.size() - 1;
            ele.templateId = templateId;
            return ele;
        }
    };
    std::vector<PrismaLayer> layers;

    PrismaLayer & AppendLayer(PrismaLayer layer);

    size_t TotalLayers() const { return layers.size(); }
};

} // namespace emodel::etherm
} // namespace ecad