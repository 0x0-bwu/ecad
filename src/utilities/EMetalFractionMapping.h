#ifndef ECAD_EUTILS_EMATELFRACTIONMAPPING_H
#define ECAD_EUTILS_EMATELFRACTIONMAPPING_H
#include "generic/geometry/OccupancyGridMap.hpp"
#include "generic/tools/FileSystem.hpp"
#include "ECadSettings.h"
#include <vector>
namespace ecad {

class ILayoutView;
namespace eutils {

using namespace generic::geometry;

using IntPolygon = Polygon2D<ECoord>;
using ELayerMetalFraction = OccupancyGridMap<ESimVal>;
using ELayoutMetalFraction = std::vector<SPtr<ELayerMetalFraction> >;
using ELayoutMetalFractionMapCtrl = typename OccupancyGridMappingFactory::GridCtrl<ECoord>;
class ECAD_API ELayerMetalFractionMapper
{
public:
    using Property = void*;//not used now
    using Factory = OccupancyGridMappingFactory;
    using MapCtrl = ELayoutMetalFractionMapCtrl;
    using Product = typename Factory::Product<Property>;
    using Setting = EMetalFractionMappingSettings;
    explicit ELayerMetalFractionMapper(const Setting & settings, ELayerMetalFraction & fraction, ELayerId layerId, bool isMetal);
    virtual ~ELayerMetalFractionMapper();

    void GenerateMetalFractionMapping(CPtr<ILayoutView> layout, const MapCtrl & ctrl);

private:
    void Mapping(const MapCtrl & ctrl);

private:
    bool m_bMetal;
    ELayerId m_layerId;
    const Setting & m_settings;
    ELayerMetalFraction & m_fraction;
    std::vector<IntPolygon> m_solids;
    std::vector<IntPolygon> m_holes;
};

struct EStackupLayerInfo
{
    bool isMetal;
    FCoord elevation;
    FCoord thickness;
    std::string name;
};

struct EMetalFractionInfo
{
    EBox2D origin;
    EBox2D extension;
    ECoordUnits coordUnits;
    std::array<size_t, 2> grid;
    std::array<ECoord, 2> stride;
    std::vector<EStackupLayerInfo> layers;
};

class ECAD_API ELayoutMetalFractionMapper
{
    using MapCtrl = ELayoutMetalFractionMapCtrl;
public:
    explicit ELayoutMetalFractionMapper(EMetalFractionMappingSettings settings);
    virtual ~ELayoutMetalFractionMapper();

    bool GenerateMetalFractionMapping(Ptr<ILayoutView> layout);
    CPtr<ELayoutMetalFraction> GetLayoutMetalFraction() const;
    CPtr<EMetalFractionInfo> GetMetalFractionInfo() const;

private:
    bool WriteResult2File(double scale);

private:
    EMetalFractionMappingSettings m_settings;
    UPtr<ELayoutMetalFraction> m_result = nullptr;
    UPtr<EMetalFractionInfo> m_mfInfo = nullptr;
    filesystem::FileHelper m_fileHelper;
};

ECAD_API bool WriteThermalProfile(const EMetalFractionInfo & info, const ELayoutMetalFraction & mf, const std::string & filename);

}//namespace eutils
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EMetalFractionMapping.cpp"
#endif

#endif//ECAD_EUTILS_EMATELFRACTIONMAPPING_H