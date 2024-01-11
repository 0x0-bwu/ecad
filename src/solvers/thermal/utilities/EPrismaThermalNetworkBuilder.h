 #pragma once
#include "models/thermal/EPrismaThermalModel.h"
#include "solvers/thermal/network/ThermalNetwork.hpp"
namespace ecad::solver {

using namespace model;
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
    using ModelType = EPrismaThermalModel;
    mutable EPrismaThermalNetworkBuildSummary summary;
    explicit EPrismaThermalNetworkBuilder(const ModelType & model);
    virtual ~EPrismaThermalNetworkBuilder() = default;

    UPtr<ThermalNetwork<EFloat> > Build(const std::vector<EFloat> & iniT, size_t threads = 1) const;

private:
    void BuildPrismaElement(const std::vector<EFloat> & iniT, Ptr<ThermalNetwork<EFloat> > network, size_t start, size_t end) const;
    void BuildLineElement(const std::vector<EFloat> & iniT, Ptr<ThermalNetwork<EFloat> > network) const;
    std::array<EFloat, 3> GetMatThermalConductivity(EMaterialId matId, EFloat refT) const;
    EFloat GetMatMassDensity(EMaterialId matId, EFloat refT) const;
    EFloat GetMatSpecificHeat(EMaterialId matId, EFloat refT) const;
    EFloat GetMatResistivity(EMaterialId matId, EFloat refT) const;

    /// global index
    const FPoint3D & GetPrismaVertexPoint(size_t index, size_t iv) const;
    FPoint2D GetPrismaVertexPoint2D(size_t index, size_t iv) const;
    FPoint2D GetPrismaCenterPoint2D(size_t index) const;

    /// unit: SI
    EFloat GetPrismaCenterDist2Side(size_t index, size_t ie) const;
    EFloat GetPrismaEdgeLength(size_t index, size_t ie) const;
    EFloat GetPrismaSideArea(size_t index, size_t ie) const;
    EFloat GetPrismaTopBotArea(size_t index) const;
    EFloat GetPrismaVolume(size_t index) const;
    EFloat GetPrismaHeight(size_t index) const;

    EFloat GetLineJouleHeat(size_t index, EFloat refT) const;
    EFloat GetLineVolume(size_t index) const;
    EFloat GetLineLength(size_t index) const;
    EFloat GetLineArea(size_t index) const;

private:
    const ModelType & m_model;
};
} // namespace ecad::solver

