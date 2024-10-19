#pragma once
#include "EThermalNetworkBuilder.h"
#include "model/thermal/EPrismThermalModel.h"
#include "solver/thermal/network/ThermalNetwork.h"
namespace ecad::solver {

using namespace model;
using namespace thermal::model;

template <typename Scalar>
class ECAD_API EPrismThermalNetworkBuilder : public EThermalNetworkBuilder
{
public:
    using ModelType = EPrismThermalModel;
    using Network = thermal::model::ThermalNetwork<Scalar>;
    explicit EPrismThermalNetworkBuilder(const ModelType & model);
    virtual ~EPrismThermalNetworkBuilder() = default;

    UPtr<Network > Build(const std::vector<Scalar> & iniT, size_t threads = 1) const;

protected:
    virtual void BuildPrismElement(const std::vector<Scalar> & iniT, Ptr<Network> network, size_t start, size_t end) const;
    virtual void ApplyBlockBCs(Ptr<Network> network) const;
    void BuildLineElement(const std::vector<Scalar> & iniT, Ptr<Network> network) const;

    std::array<EFloat, 3> GetMatThermalConductivity(EMaterialId matId, EFloat refT) const;
    EFloat GetMatMassDensity(EMaterialId matId, EFloat refT) const;
    EFloat GetMatSpecificHeat(EMaterialId matId, EFloat refT) const;
    EFloat GetMatResistivity(EMaterialId matId, EFloat refT) const;

    /// global index
    const FPoint3D & GetPrismVertexPoint(size_t index, size_t iv) const;
    FPoint2D GetPrismVertexPoint2D(size_t index, size_t iv) const;
    FPoint2D GetPrismCenterPoint2D(size_t index) const;

    /// unit: SI
    EFloat GetPrismCenterDist2Side(size_t index, size_t ie) const;
    EFloat GetPrismEdgeLength(size_t index, size_t ie) const;
    EFloat GetPrismSideArea(size_t index, size_t ie) const;
    EFloat GetPrismTopBotArea(size_t index) const;
    EFloat GetPrismVolume(size_t index) const;
    EFloat GetPrismHeight(size_t index) const;

    EFloat GetLineJouleHeat(size_t index, EFloat refT) const;
    EFloat GetLineVolume(size_t index) const;
    EFloat GetLineLength(size_t index) const;
    EFloat GetLineArea(size_t index) const;

protected:
    const ModelType & m_model;
};
} // namespace ecad::solver

