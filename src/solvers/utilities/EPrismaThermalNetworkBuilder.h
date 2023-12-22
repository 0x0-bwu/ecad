 #pragma once
#include "models/thermal/EPrismaThermalModel.h"
#include "thermal/model/ThermalNetwork.hpp"
namespace ecad::esolver {

using namespace emodel::etherm;
using namespace thermal::model;

struct EPrismaThermalNetworkBuildSummary
{
    size_t totalNodes = 0;
    size_t fixedTNodes = 0;
    size_t boundaryNodes = 0;
    double iHeatFlow = 0, oHeatFlow = 0;
    void Reset() { *this = EPrismaThermalNetworkBuildSummary{}; }
};

class ECAD_API EPrismaThermalNetworkBuilder
{
public:
    mutable EPrismaThermalNetworkBuildSummary summary;
    explicit EPrismaThermalNetworkBuilder(const EPrismaThermalModel & model);
    virtual ~EPrismaThermalNetworkBuilder() = default;

    UPtr<ThermalNetwork<ESimVal> > Build(const std::vector<ESimVal> & iniT) const;

private:
    std::array<ESimVal, 3> GetMaterialK(EMaterialId matId, ESimVal refT) const;
    ESimVal GetMaterialRho(EMaterialId matId, ESimVal refT) const;
    ESimVal GetMaterialC(EMaterialId matId, ESimVal refT) const;

    /// global index
    const FPoint3D & GetPrismaVertexPoint(size_t index, size_t iv) const;
    FPoint2D GetPrismaVertexPoint2D(size_t index, size_t iv) const;
    FPoint2D GetPrismaCenterPoint2D(size_t index) const;

    /// unit: m
    EValue GetPrismaCenterDist2Side(size_t index, size_t ie) const;
    EValue GetPrismaEdgeLength(size_t index, size_t ie) const;
    EValue GetPrismaSideArea(size_t index, size_t ie) const;
    EValue GetPrismaTopBotArea(size_t index) const;
    EValue GetPrismaVolume(size_t index) const;
    EValue GetPrismaHeight(size_t index) const;
    EValue GetLineVolume(size_t index) const;
    EValue GetLineLength(size_t index) const;
    EValue GetLineArea(size_t index) const;

private:
    const EPrismaThermalModel & m_model;
};
} // namespace ecad::esolver

