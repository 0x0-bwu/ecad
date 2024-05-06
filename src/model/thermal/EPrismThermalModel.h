#pragma once

#include "EThermalModel.h"
#include "basic/EShape.h"

#include "generic/geometry/Triangulation.hpp"
namespace ecad {

using namespace generic::geometry::tri;
class IComponent;
class ILayoutView;
class IMaterialDef;
class IMaterialDefCollection;

class ELookupTable1D;

namespace utils {
class ELayoutRetriever;
} // namespace utils

namespace model {

class ELayerCutModel;
namespace utils { class EPrismThermalModelQuery; }

struct ECAD_API LineElement
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("net_id", netId);
        ar & boost::serialization::make_nvp("material_id", matId);
        ar & boost::serialization::make_nvp("radius", radius);
        ar & boost::serialization::make_nvp("current", current);
        ar & boost::serialization::make_nvp("id", id);
        ar & boost::serialization::make_nvp("scenario", scenario);
        ar & boost::serialization::make_nvp("end_points", endPoints);
        ar & boost::serialization::make_nvp("neighbors", neighbors);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    ENetId netId;
    EMaterialId matId;
    EFloat radius{0};
    EFloat current{0};
    size_t id{invalidIndex};
    size_t scenario{invalidIndex};
    std::array<size_t, 2> endPoints;
    std::array<std::vector<size_t>, 2> neighbors;//global index
};

struct PrismElement
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("net_id", netId);
        ar & boost::serialization::make_nvp("material_id", matId);
        ar & boost::serialization::make_nvp("power_ratio", powerRatio);
        ar & boost::serialization::make_nvp("id", id);
        ar & boost::serialization::make_nvp("template_id", templateId);
        ar & boost::serialization::make_nvp("power_scenario", powerScenario);
        ar & boost::serialization::make_nvp("power_lut", powerLut);
        ar & boost::serialization::make_nvp("neighbors", neighbors);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    ENetId netId;
    EMaterialId matId;
    EFloat powerRatio{0};
    size_t id{invalidIndex};
    size_t templateId{invalidIndex};
    size_t powerScenario{invalidIndex};
    SPtr<ELookupTable1D> powerLut{nullptr};
    inline static constexpr size_t TOP_NEIGHBOR_INDEX = 3;
    inline static constexpr size_t BOT_NEIGHBOR_INDEX = 4;
    std::array<size_t, 5> neighbors = {noNeighbor, noNeighbor, noNeighbor, noNeighbor, noNeighbor};//[edge1, edge2, edge3, top, bot];
};

struct PrismLayer
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("id", id);
        ar & boost::serialization::make_nvp("elevation", elevation);
        ar & boost::serialization::make_nvp("thickness", thickness);
        ar & boost::serialization::make_nvp("elements", elements);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    size_t id;
    EFloat elevation;
    EFloat thickness;
    std::vector<PrismElement> elements;
    explicit PrismLayer(size_t layer) : id(layer) {}
    PrismElement & operator[] (size_t index) { return elements[index]; }
    const PrismElement & operator[] (size_t index) const { return elements.at(index); }

    PrismElement & AddElement(size_t templateId)
    {
        auto & ele = elements.emplace_back(PrismElement{});
        ele.id = elements.size() - 1;
        ele.templateId = templateId;
        return ele;
    }

    size_t TotalElements() const { return elements.size(); }
protected:
    PrismLayer() = default;
};

struct ContactInstance
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("index", index);
        ar & boost::serialization::make_nvp("ratio", ratio);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    size_t index{invalidIndex};
    EFloat ratio{0};
    ContactInstance() = default;
    ContactInstance(size_t index, EFloat ratio) : index(index), ratio(ratio) {}
};

using ContactInstances = std::vector<ContactInstance>;
struct PrismInstance
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("layer", layer);
        ar & boost::serialization::make_nvp("element", element);
        ar & boost::serialization::make_nvp("vertices", vertices);
        ar & boost::serialization::make_nvp("neighbors", neighbors);
        ar & boost::serialization::make_nvp("contact_instances", contactInstances);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    CPtr<PrismLayer> layer{nullptr};
    CPtr<PrismElement> element{nullptr};
    std::array<size_t, 6> vertices;//top, bot
    std::array<size_t, 5> neighbors = {noNeighbor, noNeighbor, noNeighbor, noNeighbor, noNeighbor};//[edge1, edge2, edge3, top, bot];
    std::array<ContactInstances, 2> contactInstances;//[top, bot], only used for stackup prism model
};

class ECAD_API EPrismThermalModel : public EThermalModel
{
protected:
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EPrismThermalModel() = default;
public:
    friend class utils::EPrismThermalModelQuery;
    using BlockBC = std::pair<EBox2D, EThermalBondaryCondition>;
    using PrismTemplate = tri::Triangulation<EPoint2D>;
    
    explicit EPrismThermalModel(CPtr<ILayoutView> layout);
    virtual ~EPrismThermalModel() = default;

    void SetLayerPrismTemplate(size_t layer, SPtr<PrismTemplate> prismTemplate);
    SPtr<PrismTemplate> GetLayerPrismTemplate(size_t layer) const;

    CPtr<IMaterialDefCollection> GetMaterialLibrary() const;

    void AddBlockBC(EOrientation orient, EBox2D block, EThermalBondaryCondition bc);
    const std::vector<BlockBC> & GetBlockBCs(EOrientation orient) const { return m_blockBCs.at(orient); }

    PrismLayer & AppendLayer(PrismLayer layer);
    LineElement & AddLineElement(FPoint3D start, FPoint3D end, ENetId netId, EMaterialId matId, EFloat radius, EFloat current, size_t scenario);

    void BuildPrismModel(EFloat scaleH2Unit, EFloat scale2Meter);
    void AddBondWiresFromLayerCutModel(CPtr<ELayerCutModel> lcm);
    EFloat CoordScale2Meter(int order = 1) const;
    EFloat UnitScale2Meter(int order = 1) const;  
    size_t TotalLayers() const;
    size_t TotalElements() const;
    size_t TotalLineElements() const;
    size_t TotalPrismElements() const;
    size_t GlobalIndex(size_t lineIdx) const;
    size_t GlobalIndex(size_t lyrIndex, size_t eleIndex) const;
    std::pair<size_t, size_t> PrismLocalIndex(size_t index) const;//[lyrIndex, eleIndex]
    size_t LineLocalIndex(size_t index) const;
    bool isPrima(size_t index) const;

    const std::vector<FPoint3D> & GetPoints() const { return m_points; }
    const FPoint3D & GetPoint(size_t index) const { return m_points.at(index); }
    const PrismInstance & GetPrism(size_t index) const { return m_prisms.at(index); }
    const LineElement & GetLine(size_t index) const { return m_lines.at(index); }

    bool NeedIteration() const { return true; }
    bool isTopLayer(size_t lyrIndex) const;
    bool isBotLayer(size_t lyrIndex) const;
    size_t AddPoint(FPoint3D point);
    FPoint3D GetPoint(size_t lyrIndex, size_t eleIndex, size_t vtxIndex) const;

    virtual void SearchElementIndices(const std::vector<FPoint3D> & monitors, std::vector<size_t> & indices) const override;
    virtual EModelType GetModelType() const override { return EModelType::ThermalPrism; }

protected:
    ///Copy
    virtual Ptr<EPrismThermalModel> CloneImp() const override { return new EPrismThermalModel(*this); }

public:
    std::vector<PrismLayer> layers;

protected:
    EFloat m_scaleH2Unit;
    EFloat m_scale2Meter;
    CPtr<ILayoutView> m_layout{nullptr};
    std::vector<FPoint3D> m_points;
    std::vector<LineElement> m_lines;
    std::vector<PrismInstance> m_prisms;
    std::vector<size_t> m_indexOffset;
    std::unordered_map<EOrientation, std::vector<BlockBC>> m_blockBCs;
    std::unordered_map<size_t, SPtr<PrismTemplate> > m_prismTemplates;
};

ECAD_ALWAYS_INLINE EFloat EPrismThermalModel::CoordScale2Meter(int order) const
{
    return std::pow(m_scaleH2Unit * m_scale2Meter, order);
}

ECAD_ALWAYS_INLINE EFloat EPrismThermalModel::UnitScale2Meter(int order) const
{
    return std::pow(m_scale2Meter, order);
}

ECAD_ALWAYS_INLINE size_t EPrismThermalModel::TotalLayers() const
{
    return layers.size();
}

ECAD_ALWAYS_INLINE size_t EPrismThermalModel::TotalElements() const
{
    return TotalLineElements() + TotalPrismElements();
}

ECAD_ALWAYS_INLINE size_t EPrismThermalModel::TotalLineElements() const
{
    return m_lines.size();
}

ECAD_ALWAYS_INLINE size_t EPrismThermalModel::TotalPrismElements() const
{
    return m_indexOffset.back();
}

ECAD_ALWAYS_INLINE size_t EPrismThermalModel::GlobalIndex(size_t lineIdx) const
{
    return m_indexOffset.back() + lineIdx;
}

ECAD_ALWAYS_INLINE size_t EPrismThermalModel::GlobalIndex(size_t lyrIndex, size_t eleIndex) const
{
    return m_indexOffset[lyrIndex] + eleIndex;
}

ECAD_ALWAYS_INLINE std::pair<size_t, size_t> EPrismThermalModel::PrismLocalIndex(size_t index) const
{
    size_t lyrIdex = 0;
    while (not (m_indexOffset[lyrIdex] <= index && index < m_indexOffset[lyrIdex + 1])) lyrIdex++;
    return std::make_pair(lyrIdex, index - m_indexOffset[lyrIdex]);
}

ECAD_ALWAYS_INLINE size_t EPrismThermalModel::LineLocalIndex(size_t index) const
{
    ECAD_ASSERT(index >= m_indexOffset.back());
    return index - m_indexOffset.back();
}

ECAD_ALWAYS_INLINE bool EPrismThermalModel::isPrima(size_t index) const
{
    return index < m_indexOffset.back();
}

ECAD_ALWAYS_INLINE bool EPrismThermalModel::isTopLayer(size_t lyrIndex) const
{
    return 0 == lyrIndex;
}

ECAD_ALWAYS_INLINE bool EPrismThermalModel::isBotLayer(size_t lyrIndex) const
{
    return lyrIndex + 1 == TotalLayers();
}

} // namespace model
} // namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::model::EPrismThermalModel)