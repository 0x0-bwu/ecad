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

    const FPoint3D & GetElementVertexPoint(size_t index, size_t iv) const;
    FPoint2D GetElementVertexPoint2D(size_t index, size_t iv) const;
    FPoint2D GetElementCenterPoint2D(size_t index) const;

    /// unit: m
    EValue GetElementCenterDist2Side(size_t index, size_t ie) const;
    EValue GetElementEdgeLength(size_t index, size_t ie) const;
    EValue GetElementSideArea(size_t index, size_t ie) const;
    EValue GetElementTopBotArea(size_t index) const;
    EValue GetElementVolume(size_t index) const;
    EValue GetElementHeight(size_t index) const;

private:
    const EPrismaThermalModel & m_model;
};

ECAD_ALWAYS_INLINE std::array<ESimVal, 3> EPrismaThermalNetworkBuilder::GetMaterialK(EMaterialId matId, ESimVal refT) const
{
    ECAD_UNUSED(matId)
    ECAD_UNUSED(refT)
    return {400, 400, 400};//wbtest, todo
}

ECAD_ALWAYS_INLINE ESimVal EPrismaThermalNetworkBuilder::GetMaterialRho(EMaterialId matId, ESimVal refT) const
{
    ECAD_UNUSED(matId)
    ECAD_UNUSED(refT)
    return 8850;//wbtest, todo
}

ECAD_ALWAYS_INLINE ESimVal EPrismaThermalNetworkBuilder::GetMaterialC(EMaterialId matId, ESimVal refT) const
{
    ECAD_UNUSED(matId)
    ECAD_UNUSED(refT)
    return 380;//wbtest, todo
}
} // namespace ecad::esolver

