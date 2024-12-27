#include "EStackupPrismThermalNetworkBuilder.h"
#include "model/thermal/utils/EStackupPrismThermalModelQuery.h"
#include "basic/ELookupTable.h"

#include "interface/IMaterialDefCollection.h"
#include "interface/IMaterialProp.h"
#include "interface/IMaterialDef.h"

#include "generic/thread/ThreadPool.hpp"
namespace ecad::solver {

using namespace ecad::model;

template <typename Scalar>
EStackupPrismThermalNetworkBuilder<Scalar>::EStackupPrismThermalNetworkBuilder(const ModelType & model)
 : EPrismThermalNetworkBuilder<Scalar>(model)
{
}

template <typename Scalar>
void EStackupPrismThermalNetworkBuilder<Scalar>::BuildPrismElement(const std::vector<Scalar> & iniT, Ptr<Network> network, size_t start, size_t end) const
{
    const auto & model = this->m_model;
    auto topBC = model.GetUniformBC(EOrientation::Top);
    auto botBC = model.GetUniformBC(EOrientation::Bot);
    
    auto & summary = EThermalNetworkBuilder::summary;
    for (size_t i = start; i < end; ++i) {
        const auto & inst = model.GetPrism(i);
        const auto & element = model.GetPrismElement(inst.layer, inst.element);
        if (const auto & lut = element.powerLut; lut) {
            auto p = lut->Lookup(iniT.at(i));
            p *= element.powerRatio;
            summary.iHeatFlow += p;
            network->AddHF(i, p);
            network->SetScenario(i, element.powerScenario);
        }

        auto c = this->GetMatSpecificHeat(element.matId, iniT.at(i));
        auto rho = this->GetMatMassDensity(element.matId, iniT.at(i));
        auto vol = this->GetPrismVolume(i);
        network->SetC(i, c * rho * vol);

        auto k = this->GetMatThermalConductivity(element.matId, iniT.at(i));
        auto ct = this->GetPrismCenterPoint2D(i);

        const auto & neighbors = inst.neighbors;
        //edges
        for (size_t ie = 0; ie < 3; ++ie) {
            auto vArea = this->GetPrismSideArea(i, ie);
            if (auto nid = neighbors.at(ie); tri::noNeighbor == nid) {
                //todo, side bc
            }
            else if (i < nid) { //one way
                const auto & nb = model.GetPrism(nid);
                const auto & nbEle = model.GetPrismElement(nb.layer, nb.element);
                auto ctNb = this->GetPrismCenterPoint2D(nid);
                auto vec = ctNb - ct;
                auto dist = vec.Norm2() * model.UnitScale2Meter();
                auto kxy = 0.5 * (k[0] + k[1]);
                auto dist2edge = this->GetPrismCenterDist2Side(i, ie);
                auto r1 = dist2edge / kxy / vArea;

                auto kNb = this->GetMatThermalConductivity(nbEle.matId, iniT.at(nid));
                auto kNbXY = 0.5 * (kNb[0] + kNb[1]);
                auto r2 = (dist - dist2edge) / kNbXY / vArea;
                network->SetR(i, nid, r1 + r2);
            }
        }
        auto height = this->GetPrismHeight(i);
        auto hArea = this->GetPrismTopBotArea(i);
        //top
        auto nTop = neighbors.at(PrismElement::TOP_NEIGHBOR_INDEX);
        if (tri::noNeighbor == nTop) {
            if (nullptr != topBC && topBC->isValid()) {
                if (EThermalBoundaryCondition::BCType::HTC == topBC->type) {
                    network->SetHTC(i, topBC->value * hArea);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalBoundaryCondition::BCType::HeatFlux == topBC->type) {
                    auto heatFlow = topBC->value * hArea;
                    network->SetHF(i, heatFlow);
                    if (heatFlow > 0)
                        summary.iHeatFlow += heatFlow;
                    else summary.oHeatFlow += heatFlow;
                }
            }
        }
        else if (i == nTop) {
            EFloat ratio = 1.0;
            for (const auto & contact : inst.contactInstances.front()) {
                ratio -= contact.ratio;
                if (contact.index < i) continue;
                nTop = contact.index;
                const auto & nb = model.GetPrism(nTop);
                const auto & nbEle = model.GetPrismElement(nb.layer, nb.element);
                auto hNb = this->GetPrismHeight(nTop);
                auto kNb = this->GetMatThermalConductivity(nbEle.matId, iniT.at(nTop));
                auto area = hArea * contact.ratio;
                auto r =  (0.5 * height / k[2] + 0.5 * hNb / kNb[2]) / area;
                // auto r = 0.5 * height / k[2] / hArea + 0.5 * hNb / kNb[2] / GetPrismTopBotArea(nTop);
                network->SetR(i, nTop, r);
            }
            if (ratio > 0 && nullptr != topBC && topBC->isValid()) {
                if (EThermalBoundaryCondition::BCType::HTC == topBC->type) {
                    network->SetHTC(i, topBC->value * hArea * ratio);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalBoundaryCondition::BCType::HeatFlux == topBC->type) {
                    auto heatFlow = topBC->value * hArea * ratio;
                    network->SetHF(i, heatFlow);
                    if (heatFlow > 0)
                        summary.iHeatFlow += heatFlow;
                    else summary.oHeatFlow += heatFlow;
                }
            }
        }
        else {
            ECAD_ASSERT(false)
        }
        //bot
        auto nBot = neighbors.at(PrismElement::BOT_NEIGHBOR_INDEX);
        if (tri::noNeighbor == nBot) {
            if (nullptr != botBC && botBC->isValid()) {
                if (EThermalBoundaryCondition::BCType::HTC == botBC->type) {
                    network->SetHTC(i, botBC->value * hArea);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalBoundaryCondition::BCType::HeatFlux == botBC->type) {
                    network->SetHF(i, botBC->value);
                    if (botBC->value > 0)
                        summary.iHeatFlow += botBC->value;
                    else summary.oHeatFlow += botBC->value;
                }
            }
        }
        else if (i == nBot) {
            EFloat ratio = 1.0;
            for (const auto & contact : inst.contactInstances.back()) {
                ratio -= contact.ratio;
                if (contact.index < i) continue;
                nBot = contact.index;
                const auto & nb = model.GetPrism(nBot);
                const auto & nbEle = model.GetPrismElement(nb.layer, nb.element);
                auto hNb = this->GetPrismHeight(nBot);
                auto kNb = this->GetMatThermalConductivity(nbEle.matId, iniT.at(nBot));
                auto area = hArea * contact.ratio;
                auto r =  (0.5 * height / k[2] + 0.5 * hNb / kNb[2]) / area;
                // auto r = 0.5 * height / k[2] / hArea + 0.5 * hNb / kNb[2] / GetPrismTopBotArea(nBot);
                network->SetR(i, nBot, r);
            }
            if (ratio > 0 && nullptr != botBC && botBC->isValid()) {
                if (EThermalBoundaryCondition::BCType::HTC == botBC->type) {
                    network->SetHTC(i, botBC->value * hArea * ratio);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalBoundaryCondition::BCType::HeatFlux == botBC->type) {
                    auto heatFlow = botBC->value * hArea * ratio;
                    network->SetHF(i, heatFlow);
                    if (heatFlow > 0)
                        summary.iHeatFlow += heatFlow;
                    else summary.oHeatFlow += heatFlow;
                }
            }
        }
        else {
            ECAD_ASSERT(false)
        }
    }
}

template <typename Scalar>
void EStackupPrismThermalNetworkBuilder<Scalar>::ApplyBlockBCs(Ptr<Network> network) const
{
    const auto & model = this->m_model;
    const auto & topBCs = model.GetBlockBCs(EOrientation::Top);
    const auto & botBCs = model.GetBlockBCs(EOrientation::Bot);
    if (topBCs.empty() && botBCs.empty()) return;

    model::utils::EStackupPrismThermalModelQuery query(dynamic_cast<CPtr<EStackupPrismThermalModel>>(&model));
    using RtVal = model::utils::EStackupPrismThermalModelQuery::RtVal;
    
    auto & summary = EThermalNetworkBuilder::summary;
    auto applyBlockBC = [&](const auto & block, bool isTop)
    {
        std::vector<RtVal> results;
        if (not block.second.isValid()) return;
        auto value = block.second.value;
        if (EThermalBoundaryCondition::BCType::HeatFlux == block.second.type)
            value /= block.first.Area() * model.CoordScale2Meter(2);

        for (size_t lyr = 0; lyr <  model.TotalLayers(); ++lyr) {
            query.SearchPrismInstances(lyr, block.first, results);
            if (results.empty()) continue;
            for (const auto & result : results) {
                const auto & prism = model.GetPrism(result.second);
                const auto & element = model.GetPrismElement(prism.layer, prism.element);
                auto nid = isTop ? PrismElement::TOP_NEIGHBOR_INDEX : 
                                   PrismElement::BOT_NEIGHBOR_INDEX ;
                if (element.neighbors.at(nid) != noNeighbor) continue;
                auto area = this->GetPrismTopBotArea(result.second);
                if (EThermalBoundaryCondition::BCType::HeatFlux == block.second.type) {
                    auto heatFlow = value * area;
                    network->SetHF(result.second, heatFlow);
                    if (heatFlow > 0)
                        summary.iHeatFlow += heatFlow;
                    else summary.oHeatFlow += heatFlow;
                }
                else { 
                    network->SetHTC(result.second, value * area);
                    summary.boundaryNodes += 1;
                }
            }    
        }
    };

    for (const auto & block : topBCs)
        applyBlockBC(block, true);
    for (const auto & block : botBCs)
        applyBlockBC(block, false);
}

template class EStackupPrismThermalNetworkBuilder<Float32>;
template class EStackupPrismThermalNetworkBuilder<Float64>;
} // namespace ecad::solver