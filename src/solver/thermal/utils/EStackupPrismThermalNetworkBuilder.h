//todo refactor with EPrismThermalNetworkBuilder
#pragma once
#include "EPrismThermalNetworkBuilder.h"
#include "model/thermal/EStackupPrismThermalModel.h"
#include "solver/thermal/network/ThermalNetwork.h"
namespace ecad::solver {

using namespace ecad::model;

template <typename Scalar>
class ECAD_API EStackupPrismThermalNetworkBuilder : public EPrismThermalNetworkBuilder<Scalar>
{
public:
    using ModelType = EStackupPrismThermalModel;
    using Network = typename EPrismThermalNetworkBuilder<Scalar>::Network;
    explicit EStackupPrismThermalNetworkBuilder(const ModelType & model);
    virtual ~EStackupPrismThermalNetworkBuilder() = default;

private:
    void BuildPrismElement(const std::vector<Scalar> & iniT, Ptr<Network> network, size_t start, size_t end) const override;
    void ApplyBlockBCs(Ptr<Network> network) const override;
};
} // namespace ecad::solver

