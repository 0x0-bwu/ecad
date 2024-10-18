//todo refactor with EPrismThermalNetworkBuilder
#pragma once
#include "EPrismThermalNetworkBuilder.h"
#include "model/thermal/EStackupPrismThermalModel.h"
#include "solver/thermal/network/ThermalNetwork.h"
namespace ecad::solver {

using namespace model;
using namespace thermal::model;

class ECAD_API EStackupPrismThermalNetworkBuilder : public EPrismThermalNetworkBuilder
{
public:
    using ModelType = EStackupPrismThermalModel;
    explicit EStackupPrismThermalNetworkBuilder(const ModelType & model);
    virtual ~EStackupPrismThermalNetworkBuilder() = default;

private:
    void BuildPrismElement(const std::vector<EFloat> & iniT, Ptr<ThermalNetwork<EFloat> > network, size_t start, size_t end) const override;
    void ApplyBlockBCs(Ptr<ThermalNetwork<EFloat> > network) const override;
};
} // namespace ecad::solver

