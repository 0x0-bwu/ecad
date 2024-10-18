#include "EStackupPrismThermalNetworkBuilder.h"
#include "model/thermal/utils/EStackupPrismThermalModelQuery.h"
#include "basic/ELookupTable.h"

#include "interface/IMaterialDefCollection.h"
#include "interface/IMaterialProp.h"
#include "interface/IMaterialDef.h"

#include "generic/thread/ThreadPool.hpp"
namespace ecad::solver {

using namespace ecad::model;
ECAD_INLINE EStackupPrismThermalNetworkBuilder::EStackupPrismThermalNetworkBuilder(const ModelType & model)
 : EPrismThermalNetworkBuilder(model)
{
}

ECAD_INLINE void EStackupPrismThermalNetworkBuilder::BuildPrismElement(const std::vector<EFloat> & iniT, Ptr<ThermalNetwork<EFloat> > network, size_t start, size_t end) const
{
    auto topBC = m_model.GetUniformBC(EOrientation::Top);
    auto botBC = m_model.GetUniformBC(EOrientation::Bot);
    
    for (size_t i = start; i < end; ++i) {
        const auto & inst = m_model.GetPrism(i);
        const auto & element = m_model.GetPrismElement(inst.layer, inst.element);
        if (const auto & lut = element.powerLut; lut) {
            auto p = lut->Lookup(iniT.at(i));
            p *= element.powerRatio;
            summary.iHeatFlow += p;
            network->AddHF(i, p);
            network->SetScenario(i, element.powerScenario);
        }

        auto c = GetMatSpecificHeat(element.matId, iniT.at(i));
        auto rho = GetMatMassDensity(element.matId, iniT.at(i));
        auto vol = GetPrismVolume(i);
        network->SetC(i, c * rho * vol);

        auto k = GetMatThermalConductivity(element.matId, iniT.at(i));
        auto ct = GetPrismCenterPoint2D(i);

        const auto & neighbors = inst.neighbors;
        //edges
        for (size_t ie = 0; ie < 3; ++ie) {
            auto vArea = GetPrismSideArea(i, ie);
            if (auto nid = neighbors.at(ie); tri::noNeighbor == nid) {
                //todo, side bc
            }
            else if (i < nid) { //one way
                const auto & nb = m_model.GetPrism(nid);
                const auto & nbEle = m_model.GetPrismElement(nb.layer, nb.element);
                auto ctNb = GetPrismCenterPoint2D(nid);
                auto vec = ctNb - ct;
                auto dist = vec.Norm2() * m_model.UnitScale2Meter();
                auto kxy = 0.5 * (k[0] + k[1]);
                auto dist2edge = GetPrismCenterDist2Side(i, ie);
                auto r1 = dist2edge / kxy / vArea;

                auto kNb = GetMatThermalConductivity(nbEle.matId, iniT.at(nid));
                auto kNbxy = 0.5 * (kNb[0] + kNb[1]);
                auto r2 = (dist - dist2edge) / kNbxy / vArea;
                network->SetR(i, nid, r1 + r2);
            }
        }
        auto height = GetPrismHeight(i);
        auto hArea = GetPrismTopBotArea(i);
        //top
        auto nTop = neighbors.at(PrismElement::TOP_NEIGHBOR_INDEX);
        if (tri::noNeighbor == nTop) {
            if (nullptr != topBC && topBC->isValid()) {
                if (EThermalBondaryCondition::BCType::HTC == topBC->type) {
                    network->SetHTC(i, topBC->value * hArea);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalBondaryCondition::BCType::HeatFlux == topBC->type) {
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
                const auto & nb = m_model.GetPrism(nTop);
                const auto & nbEle = m_model.GetPrismElement(nb.layer, nb.element);
                auto hNb = GetPrismHeight(nTop);
                auto kNb = GetMatThermalConductivity(nbEle.matId, iniT.at(nTop));
                auto area = hArea * contact.ratio;
                auto r =  (0.5 * height / k[2] + 0.5 * hNb / kNb[2]) / area;
                // auto r = 0.5 * height / k[2] / hArea + 0.5 * hNb / kNb[2] / GetPrismTopBotArea(nTop);
                network->SetR(i, nTop, r);
            }
            if (ratio > 0 && nullptr != topBC && topBC->isValid()) {
                if (EThermalBondaryCondition::BCType::HTC == topBC->type) {
                    network->SetHTC(i, topBC->value * hArea * ratio);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalBondaryCondition::BCType::HeatFlux == topBC->type) {
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
                if (EThermalBondaryCondition::BCType::HTC == botBC->type) {
                    network->SetHTC(i, botBC->value * hArea);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalBondaryCondition::BCType::HeatFlux == botBC->type) {
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
                const auto & nb = m_model.GetPrism(nBot);
                const auto & nbEle = m_model.GetPrismElement(nb.layer, nb.element);
                auto hNb = GetPrismHeight(nBot);
                auto kNb = GetMatThermalConductivity(nbEle.matId, iniT.at(nBot));
                auto area = hArea * contact.ratio;
                auto r =  (0.5 * height / k[2] + 0.5 * hNb / kNb[2]) / area;
                // auto r = 0.5 * height / k[2] / hArea + 0.5 * hNb / kNb[2] / GetPrismTopBotArea(nBot);
                network->SetR(i, nBot, r);
            }
            if (ratio > 0 && nullptr != botBC && botBC->isValid()) {
                if (EThermalBondaryCondition::BCType::HTC == botBC->type) {
                    network->SetHTC(i, botBC->value * hArea * ratio);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalBondaryCondition::BCType::HeatFlux == botBC->type) {
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

ECAD_INLINE void EStackupPrismThermalNetworkBuilder::ApplyBlockBCs(Ptr<ThermalNetwork<EFloat> > network) const
{
    const auto & topBCs = m_model.GetBlockBCs(EOrientation::Top);
    const auto & botBCs = m_model.GetBlockBCs(EOrientation::Bot);
    if (topBCs.empty() && botBCs.empty()) return;

    model::utils::EStackupPrismThermalModelQuery query(dynamic_cast<CPtr<EStackupPrismThermalModel>>(&m_model));
    using RtVal = model::utils::EStackupPrismThermalModelQuery::RtVal;

    auto applyBlockBC = [&](const auto & block, bool isTop)
    {
        std::vector<RtVal> results;
        if (not block.second.isValid()) return;
        auto value = block.second.value;
        if (EThermalBondaryCondition::BCType::HeatFlux == block.second.type)
            value /= block.first.Area() * m_model.CoordScale2Meter(2);

        for (size_t lyr = 0; lyr <  m_model.TotalLayers(); ++lyr) {
            query.SearchPrismInstances(lyr, block.first, results);
            if (results.empty()) continue;
            for (const auto & result : results) {
                const auto & prism = m_model.GetPrism(result.second);
                const auto & element = m_model.GetPrismElement(prism.layer, prism.element);
                auto nid = isTop ? PrismElement::TOP_NEIGHBOR_INDEX : 
                                   PrismElement::BOT_NEIGHBOR_INDEX ;
                if (element.neighbors.at(nid) != noNeighbor) continue;
                auto area = GetPrismTopBotArea(result.second);
                if (EThermalBondaryCondition::BCType::HeatFlux == block.second.type) {
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

} // namespace ecad::solver