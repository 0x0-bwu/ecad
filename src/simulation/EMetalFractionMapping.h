#ifndef ECAD_ESIM_EMATELFRACTIONMAPPING_H
#define ECAD_ESIM_EMATELFRACTIONMAPPING_H
#include "generic/geometry/OccupancyGridMap.hpp"
#include "generic/tools/FileSystem.hpp"
#include "ESimulationSettings.h"
#include "ECadCommon.h"
#include "ECadDef.h"
#include <vector>
namespace ecad {

class ILayoutView;
namespace esim {

using namespace generic::geometry;

using IntPolygon = Polygon2D<ECoord>;
using MapCtrl = OccupancyGridMappingFactory::GridCtrl<ECoord>;
using LayoutMetalFraction = OccupancyGridMap<std::vector<float> >;
class ECAD_API LayereMetalFractionMapper
{
public:
    using Property = void*;//not used now
    using Factory = OccupancyGridMappingFactory;
    using Product = typename Factory::Product<Property>;
    using Setting = EMetalFractionMappingSettings;
    explicit LayereMetalFractionMapper(const Setting & settings, LayoutMetalFraction & fraction, ELayerId layerId, bool isMetal);
    virtual ~LayereMetalFractionMapper();

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
    ECoord resolution;
    ECoordUnits coordUnits;
    std::pair<size_t, size_t> tiles;
    std::vector<StackupLayerInfo> layers;
};

class ECAD_API LayoutMetalFractionMapper
{
    static constexpr char suffix[] = ".mf";
public:
    explicit LayoutMetalFractionMapper(EMetalFractionMappingSettings settings);
    virtual ~LayoutMetalFractionMapper();

    bool GenerateMetalFractionMapping(CPtr<ILayoutView> layout);
    CPtr<LayoutMetalFraction> GetLayoutMetalFraction() const;
    CPtr<MetalFractionInfo> GetMetalFractionInfo() const;

private:
    bool WriteResult2File();

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