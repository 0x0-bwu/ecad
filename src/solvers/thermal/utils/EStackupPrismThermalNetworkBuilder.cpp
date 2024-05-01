#include "EStackupPrismThermalNetworkBuilder.h"
#include "models/thermal/utils/EStackupPrismThermalModelQuery.h"
#include "ELookupTable.h"

#include "interfaces/IMaterialDefCollection.h"
#include "interfaces/IMaterialProp.h"
#include "interfaces/IMaterialDef.h"

#include "generic/thread/ThreadPool.hpp"
namespace ecad::solver {

using namespace generic;
using namespace ecad::model;
ECAD_INLINE EStackupPrismThermalNetworkBuilder::EStackupPrismThermalNetworkBuilder(const ModelType & model)
 : m_model(model)
{
}

ECAD_INLINE UPtr<ThermalNetwork<EFloat> > EStackupPrismThermalNetworkBuilder::Build(const std::vector<EFloat> & iniT, size_t threads) const
{
    const size_t size = m_model.TotalElements();
    if(iniT.size() != size) return nullptr;

    summary.Reset();
    summary.totalNodes = size;
    auto network = std::make_unique<ThermalNetwork<EFloat> >(size);

    if (threads > 1) {
        generic::thread::ThreadPool pool(threads);
        size_t size = m_model.TotalPrismElements();
        size_t blocks = pool.Threads();
        size_t blockSize = size / blocks;

        size_t begin = 0;
        for(size_t i = 0; i < blocks && blockSize > 0; ++i){
            size_t end = begin + blockSize;
            pool.Submit(std::bind(&EStackupPrismThermalNetworkBuilder::BuildPrismElement, this, std::ref(iniT), network.get(), begin, end));
            begin = end;
        }
        size_t end = size;
        if(begin != end)
            pool.Submit(std::bind(&EStackupPrismThermalNetworkBuilder::BuildPrismElement, this, std::ref(iniT), network.get(), begin, end));        
    }
    else BuildPrismElement(iniT, network.get(), 0, m_model.TotalPrismElements());
    
    BuildLineElement(iniT, network.get());
    ApplyBlockBCs(network.get());
    return network;
}

ECAD_INLINE void EStackupPrismThermalNetworkBuilder::BuildPrismElement(const std::vector<EFloat> & iniT, Ptr<ThermalNetwork<EFloat> > network, size_t start, size_t end) const
{
    auto topBC = m_model.GetUniformBC(EOrientation::Top);
    auto botBC = m_model.GetUniformBC(EOrientation::Bot);
    
    for (size_t i = start; i < end; ++i) {
        const auto & inst = m_model.GetPrism(i);
        if (const auto & lut = inst.element->powerLut; lut) {
            auto p = lut->Lookup(iniT.at(i));
            p *= inst.element->powerRatio;
            summary.iHeatFlow += p;
            network->AddHF(i, p);
            network->SetScenario(i, inst.element->powerScenario);
        }

        auto c = GetMatSpecificHeat(inst.element->matId, iniT.at(i));
        auto rho = GetMatMassDensity(inst.element->matId, iniT.at(i));
        auto vol = GetPrismVolume(i);
        network->SetC(i, c * rho * vol);

        auto k = GetMatThermalConductivity(inst.element->matId, iniT.at(i));
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
                auto ctNb = GetPrismCenterPoint2D(nid);
                auto vec = ctNb - ct;
                auto dist = vec.Norm2() * m_model.UnitScale2Meter();
                auto kxy = 0.5 * (k[0] + k[1]);
                auto dist2edge = GetPrismCenterDist2Side(i, ie);
                auto r1 = dist2edge / kxy / vArea;

                auto kNb = GetMatThermalConductivity(nb.element->matId, iniT.at(nid));
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
                auto hNb = GetPrismHeight(nTop);
                auto kNb = GetMatThermalConductivity(nb.element->matId, iniT.at(nTop));
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
                auto hNb = GetPrismHeight(nBot);
                auto kNb = GetMatThermalConductivity(nb.element->matId, iniT.at(nBot));
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

ECAD_INLINE void EStackupPrismThermalNetworkBuilder::BuildLineElement(const std::vector<EFloat> & iniT, Ptr<ThermalNetwork<EFloat> > network) const
{
    for (size_t i = 0; i < m_model.TotalLineElements(); ++i) {
        const auto & line = m_model.GetLine(i);
        auto index = m_model.GlobalIndex(i);
        auto rho = GetMatMassDensity(line.matId, iniT.at(index));
        auto c = GetMatSpecificHeat(line.matId, iniT.at(index));
        auto v = GetLineVolume(index);
        network->SetC(index, c * rho * v);

        auto jh = GetLineJouleHeat(index, iniT.at(index));
        network->AddHF(index, jh);
        network->SetScenario(index, line.scenario);
        summary.iHeatFlow += jh;

        auto k = GetMatThermalConductivity(line.matId, iniT.at(index));
        auto aveK = (k[0] + k[1] + k[2]) / 3;
        auto area = GetLineArea(index);
        auto l = GetLineLength(index);

        auto setR = [&](size_t nbIndex) {
            if (m_model.isPrima(nbIndex)) {
                auto r = 0.5 * l / aveK / area;
                network->SetR(nbIndex, index, r);
            }
            else if (i < nbIndex) {
                const auto & lineNb = m_model.GetLine(m_model.LineLocalIndex(nbIndex));
                auto kNb = GetMatThermalConductivity(lineNb.matId, iniT.at(index));
                auto avekNb = (kNb[0] + kNb[1] + kNb[2]) / 3;
                auto areaNb = GetLineArea(nbIndex);
                auto lNb = GetLineLength(nbIndex);
                auto r = 0.5 * l / aveK / area + 0.5 * lNb / avekNb / areaNb;
                network->SetR(i, nbIndex, r);
            }
        };
        for (auto nb : line.neighbors.front()) setR(nb);
        for (auto nb : line.neighbors.back()) setR(nb);
    }
}

ECAD_INLINE void EStackupPrismThermalNetworkBuilder::ApplyBlockBCs(Ptr<ThermalNetwork<EFloat> > network) const
{
    const auto & topBCs = m_model.GetBlockBCs(EOrientation::Top);
    const auto & botBCs = m_model.GetBlockBCs(EOrientation::Bot);
    if (topBCs.empty() && botBCs.empty()) return;

    model::utils::EStackupPrismThermalModelQuery query(&m_model);
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
                auto nid = isTop ? PrismElement::TOP_NEIGHBOR_INDEX : 
                                   PrismElement::BOT_NEIGHBOR_INDEX ;
                if (prism.element->neighbors.at(nid) != noNeighbor) continue;
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

ECAD_INLINE const FPoint3D & EStackupPrismThermalNetworkBuilder::GetPrismVertexPoint(size_t index, size_t iv) const
{
    return m_model.GetPoint(m_model.GetPrism(index).vertices.at(iv));
}

ECAD_INLINE FPoint2D EStackupPrismThermalNetworkBuilder::GetPrismVertexPoint2D(size_t index, size_t iv) const
{
    const auto & pt3d = GetPrismVertexPoint(index, iv);
    return FPoint2D(pt3d[0], pt3d[1]);
}

ECAD_INLINE FPoint2D EStackupPrismThermalNetworkBuilder::GetPrismCenterPoint2D(size_t index) const
{
    auto p0 = GetPrismVertexPoint2D(index, 0);
    auto p1 = GetPrismVertexPoint2D(index, 1);
    auto p2 = GetPrismVertexPoint2D(index, 2);
    return generic::geometry::Triangle2D<FCoord>(p0, p1, p2).Center();
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetPrismCenterDist2Side(size_t index, size_t ie) const
{
    ie %= 3;
    auto ct = GetPrismCenterPoint2D(index);
    auto p1 = GetPrismVertexPoint2D(index, ie);
    auto p2 = GetPrismVertexPoint2D(index, (ie + 1) % 3);
    auto distSq = generic::geometry::PointLineDistanceSq(ct, p1, p2);
    return std::sqrt(distSq) * m_model.UnitScale2Meter();
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetPrismEdgeLength(size_t index, size_t ie) const
{
    ie %= 3;
    auto p1 = GetPrismVertexPoint2D(index, ie);
    auto p2 = GetPrismVertexPoint2D(index, (ie + 1) % 3);
    auto dist = generic::geometry::Distance(p1, p2);
    return dist * m_model.UnitScale2Meter();
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetPrismSideArea(size_t index, size_t ie) const
{
    auto h = GetPrismHeight(index);
    auto w = GetPrismEdgeLength(index, ie);
    return h * w;
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetPrismTopBotArea(size_t index) const
{
    const auto & points = m_model.GetPoints();
    const auto & vs = m_model.GetPrism(index).vertices;
    auto area = generic::geometry::Triangle3D<FCoord>(points.at(vs[0]), points.at(vs[1]), points.at(vs[2])).Area();
    return area * m_model.UnitScale2Meter(2);
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetPrismVolume(size_t index) const
{
    return GetPrismTopBotArea(index) * GetPrismHeight(index);
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetPrismHeight(size_t index) const
{
    return m_model.GetPrism(index).layer->thickness * m_model.UnitScale2Meter();
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetLineJouleHeat(size_t index, EFloat refT) const
{
    const auto & line = m_model.GetLine(m_model.LineLocalIndex(index));
    if (generic::math::EQ<EFloat>(line.current, 0)) return 0;
    auto rho = GetMatResistivity(line.matId, refT);
    return rho * GetLineLength(index) * line.current * line.current / GetLineArea(index);
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetLineVolume(size_t index) const
{
    return GetLineArea(index) * GetLineLength(index);
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetLineLength(size_t index) const
{
    const auto & line = m_model.GetLine(m_model.LineLocalIndex(index));
    const auto & p1 = m_model.GetPoint(line.endPoints.front());
    const auto & p2 = m_model.GetPoint(line.endPoints.back());
    return generic::geometry::Distance(p1, p2) * m_model.UnitScale2Meter();
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetLineArea(size_t index) const
{
    const auto & line = m_model.GetLine(m_model.LineLocalIndex(index));
    return generic::math::pi * std::pow(line.radius * m_model.UnitScale2Meter(), 2);
}

ECAD_INLINE std::array<EFloat, 3> EStackupPrismThermalNetworkBuilder::GetMatThermalConductivity(EMaterialId matId, EFloat refT) const
{
    std::array<EFloat, 3> result;
    auto material = m_model.GetMaterialLibrary()->FindMaterialDefById(matId); { ECAD_ASSERT(material) }
    for (size_t i = 0; i < result.size(); ++i) {
        [[maybe_unused]] auto check = material->GetProperty(EMaterialPropId::ThermalConductivity)->GetAnsiotropicProperty(refT, i, result[i]);
        ECAD_ASSERT(check)
    }
    return result;
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetMatMassDensity(EMaterialId matId, EFloat refT) const
{
    EFloat result{0};
    auto material = m_model.GetMaterialLibrary()->FindMaterialDefById(matId); { ECAD_ASSERT(material) }
    [[maybe_unused]] auto check = material->GetProperty(EMaterialPropId::MassDensity)->GetSimpleProperty(refT, result);
    ECAD_ASSERT(check)
    return result;
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetMatSpecificHeat(EMaterialId matId, EFloat refT) const
{
    EFloat result{0};
    auto material = m_model.GetMaterialLibrary()->FindMaterialDefById(matId); { ECAD_ASSERT(material) }
    [[maybe_unused]] auto check = material->GetProperty(EMaterialPropId::SpecificHeat)->GetSimpleProperty(refT, result);
    ECAD_ASSERT(check)
    return result;
}

ECAD_INLINE EFloat EStackupPrismThermalNetworkBuilder::GetMatResistivity(EMaterialId matId, EFloat refT) const
{
    EFloat result{0};
    auto material = m_model.GetMaterialLibrary()->FindMaterialDefById(matId); { ECAD_ASSERT(material) }
    [[maybe_unused]] auto check = material->GetProperty(EMaterialPropId::Resistivity)->GetSimpleProperty(refT, result);
    ECAD_ASSERT(check)
    return result; 
}

} // namespace ecad::solver