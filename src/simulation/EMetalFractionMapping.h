#ifndef ECAD_ESIM_EMATELFRACTIONMAPPING_H
#define ECAD_ESIM_EMATELFRACTIONMAPPING_H
#include "generic/geometry/OccupancyGridMap.hpp"
#include "generic/tools/FileSystem.hpp"
#include "ECadSettings.h"
#include <vector>
namespace ecad {

class ILayoutView;
namespace esim {

using namespace generic::geometry;

using IntPolygon = Polygon2D<ECoord>;
using MapCtrl = OccupancyGridMappingFactory::GridCtrl<ECoord>;
using LayoutMetalFraction = OccupancyGridMap<std::vector<float> >;
class ECAD_API LayerMetalFractionMapper
{
public:
    using Property = void*;//not used now
    using Factory = OccupancyGridMappingFactory;
    using Product = typename Factory::Product<Property>;
    using Setting = EMetalFractionMappingSettings;
    explicit LayerMetalFractionMapper(const Setting & settings, LayoutMetalFraction & fraction, ELayerId layerId, bool isMetal);
    virtual ~LayerMetalFractionMapper();

    void GenerateMetalFractionMapping(CPtr<ILayoutView> layout, const MapCtrl & ctrl);

private:
    void Mapping(const MapCtrl & ctrl);

private:
    ELayerId m_id;
    bool m_bMetal;
    const Setting & m_settings;
    LayoutMetalFraction & m_fraction;
    std::vector<IntPolygon> m_solids;
    std::vector<IntPolygon> m_holes;
};

struct StackupLayerInfo
{
    bool isMetal;
    FCoord elevation;
    FCoord thickness;
    std::string name;
};

struct MetalFractionInfo
{
    EBox2D origin;
    EBox2D extension;
    ECoordUnits coordUnits;
    std::array<size_t, 2> grid;
    std::array<ECoord, 2> stride;
    std::vector<StackupLayerInfo> layers;
};

class ECAD_API LayoutMetalFractionMapper
{
    static constexpr char suffix[] = ".mf";
public:
    explicit LayoutMetalFractionMapper(EMetalFractionMappingSettings settings);
    virtual ~LayoutMetalFractionMapper();

    bool GenerateMetalFractionMapping(Ptr<ILayoutView> layout);
    CPtr<LayoutMetalFraction> GetLayoutMetalFraction() const;
    CPtr<MetalFractionInfo> GetMetalFractionInfo() const;

private:
    bool WriteResult2File(double scale);

private:
    EMetalFractionMappingSettings m_settings;
    UPtr<LayoutMetalFraction> m_result = nullptr;
    UPtr<MetalFractionInfo> m_mfInfo = nullptr;
    filesystem::FileHelper m_fileHelper;
};

ECAD_API bool WriteThermalProfile(const MetalFractionInfo & info, const LayoutMetalFraction & mf, const std::string & filename);

}//namespace esim
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EMetalFractionMapping.cpp"
#endif

#endif//ECAD_ESIM_EMATELFRACTIONMAPPING_H