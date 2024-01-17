#include "EPrismaThermalNetworkBuilder.h"
#include "interfaces/IMaterialDefCollection.h"
#include "interfaces/IMaterialProp.h"
#include "interfaces/IMaterialDef.h"

#include "generic/thread/ThreadPool.hpp"
namespace ecad::solver {

using namespace ecad::model;
ECAD_INLINE EPrismaThermalNetworkBuilder::EPrismaThermalNetworkBuilder(const ModelType & model)
 : m_model(model)
{
}

ECAD_INLINE UPtr<ThermalNetwork<EFloat> > EPrismaThermalNetworkBuilder::Build(const std::vector<EFloat> & iniT, size_t threads) const
{
    const size_t size = m_model.TotalElements();
    if(iniT.size() != size) return nullptr;

    summary.Reset();
    summary.totalNodes = size;
    auto network = std::make_unique<ThermalNetwork<EFloat> >(size);

    if (threads > 1) {
        generic::thread::ThreadPool pool(threads);
        size_t size = m_model.TotalPrismaElements();
        size_t blocks = pool.Threads();
        size_t blockSize = size / blocks;

        size_t begin = 0;
        for(size_t i = 0; i < blocks && blockSize > 0; ++i){
            size_t end = begin + blockSize;
            pool.Submit(std::bind(&EPrismaThermalNetworkBuilder::BuildPrismaElement, this, std::ref(iniT), network.get(), begin, end));
            begin = end;
        }
        size_t end = size;
        if(begin != end)
            pool.Submit(std::bind(&EPrismaThermalNetworkBuilder::BuildPrismaElement, this, std::ref(iniT), network.get(), begin, end));        
    }
    else BuildPrismaElement(iniT, network.get(), 0, m_model.TotalPrismaElements());

    BuildLineElement(iniT, network.get());
    return network;
}

ECAD_INLINE void EPrismaThermalNetworkBuilder::BuildPrismaElement(const std::vector<EFloat> & iniT, Ptr<ThermalNetwork<EFloat> > network, size_t start, size_t end) const
{
    EThermalModel::BCType topType, botType;
    m_model.GetTopBotBCType(topType, botType);

    EFloat uniformTopBC, uniformBotBC;
    m_model.GetUniformTopBotBCValue(uniformTopBC, uniformBotBC);

    for (size_t i = start; i < end; ++i) {
        const auto & inst = m_model.GetPrisma(i);
        if (auto p = inst.element->avePower; p > 0) {
            summary.iHeatFlow += p;
            network->AddHF(i, p);
        }

        auto c = GetMatSpecificHeat(inst.element->matId, iniT.at(i));
        auto rho = GetMatMassDensity(inst.element->matId, iniT.at(i));
        auto vol = GetPrismaVolume(i);
        network->SetC(i, c * rho * vol);

        auto k = GetMatThermalConductivity(inst.element->matId, iniT.at(i));
        auto ct = GetPrismaCenterPoint2D(i);

        const auto & neighbors = inst.neighbors;
        //edges
        for (size_t ie = 0; ie < 3; ++ie) {
            auto vArea = GetPrismaSideArea(i, ie);
            if (auto nid = neighbors.at(ie); tri::noNeighbor == nid) {
                if (m_model.uniformBcSide != invalidFloat) {
                    if (EThermalModel::BCType::HTC == m_model.sideBCType) {
                        network->AddHTC(i, m_model.uniformBcSide * vArea);
                        summary.boundaryNodes += 1;
                    }
                    else if (EThermalModel::BCType::HeatFlow == m_model.sideBCType) {
                        network->AddHF(i, m_model.uniformBcSide);
                        if (m_model.uniformBcSide > 0)
                            summary.iHeatFlow += m_model.uniformBcSide;
                        else summary.oHeatFlow += m_model.uniformBcSide;
                    }
                }
            }
            else if (i < nid) { //one way
                const auto & nb = m_model.GetPrisma(nid);
                auto ctNb = GetPrismaCenterPoint2D(nid);
                auto vec = ctNb - ct;
                auto dist = vec.Norm2() * m_model.Scale2Meter();
                auto kxy = 0.5 * (k[0] + k[1]);
                auto dist2edge = GetPrismaCenterDist2Side(i, ie);
                auto r1 = dist2edge / kxy / vArea;

                auto kNb = GetMatThermalConductivity(nb.element->matId, iniT.at(nid));
                auto kNbxy = 0.5 * (kNb[0] + kNb[1]);
                auto r2 = (dist - dist2edge) / kNbxy / vArea;
                network->SetR(i, nid, r1 + r2);
            }
        }
        auto height = GetPrismaHeight(i);
        auto hArea = GetPrismaTopBotArea(i);
        //top
        auto nTop = neighbors.at(EPrismaThermalModel::PrismaElement::TOP_NEIGHBOR_INDEX);
        if (tri::noNeighbor == nTop) {
            if (uniformTopBC != invalidFloat) {
                if (EThermalModel::BCType::HTC == topType) {
                    network->AddHTC(i, uniformTopBC * hArea);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalModel::BCType::HeatFlow == topType) {
                    network->AddHF(i, uniformTopBC);
                    if (uniformTopBC > 0)
                        summary.iHeatFlow += uniformTopBC;
                    else summary.oHeatFlow += uniformTopBC;
                }
            }
        }
        else if (i < nTop) {
            const auto & nb = m_model.GetPrisma(nTop);
            auto hNb = GetPrismaHeight(nTop);
            auto kNb = GetMatThermalConductivity(nb.element->matId, iniT.at(nTop));
            auto r = (0.5 * height / k[2] + 0.5 * hNb / kNb[2]) / hArea;
            network->SetR(i, nTop, r);
        }
        //bot
        auto nBot = neighbors.at(EPrismaThermalModel::PrismaElement::BOT_NEIGHBOR_INDEX);
        if (tri::noNeighbor == nBot) {
            if (uniformBotBC != invalidFloat) {
                if (EThermalModel::BCType::HTC == botType) {
                    network->AddHTC(i, uniformBotBC * hArea);
                    summary.boundaryNodes += 1;
                }
                else if (EThermalModel::BCType::HeatFlow == botType) {
                    network->AddHF(i, uniformBotBC);
                    if (uniformTopBC > 0)
                        summary.iHeatFlow += uniformBotBC;
                    else summary.oHeatFlow += uniformBotBC;
                }
            }
        }
        else if (i < nBot) {
            const auto & nb = m_model.GetPrisma(nBot);
            auto hNb = GetPrismaHeight(nBot);
            auto kNb = GetMatThermalConductivity(nb.element->matId, iniT.at(nBot));
            auto r = (0.5 * height / k[2] + 0.5 * hNb / kNb[2]) / hArea;
            network->SetR(i, nBot, r);
        }
    }
}

ECAD_INLINE void EPrismaThermalNetworkBuilder::BuildLineElement(const std::vector<EFloat> & iniT, Ptr<ThermalNetwork<EFloat> > network) const
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

ECAD_INLINE const FPoint3D & EPrismaThermalNetworkBuilder::GetPrismaVertexPoint(size_t index, size_t iv) const
{
    return m_model.GetPoint(m_model.GetPrisma(index).vertices.at(iv));
}

ECAD_INLINE FPoint2D EPrismaThermalNetworkBuilder::GetPrismaVertexPoint2D(size_t index, size_t iv) const
{
    const auto & pt3d = GetPrismaVertexPoint(index, iv);
    return FPoint2D(pt3d[0], pt3d[1]);
}

ECAD_INLINE FPoint2D EPrismaThermalNetworkBuilder::GetPrismaCenterPoint2D(size_t index) const
{
    auto p0 = GetPrismaVertexPoint2D(index, 0);
    auto p1 = GetPrismaVertexPoint2D(index, 1);
    auto p2 = GetPrismaVertexPoint2D(index, 2);
    return generic::geometry::Triangle2D<FCoord>(p0, p1, p2).Center();
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetPrismaCenterDist2Side(size_t index, size_t ie) const
{
    ie %= 3;
    auto ct = GetPrismaCenterPoint2D(index);
    auto p1 = GetPrismaVertexPoint2D(index, ie);
    auto p2 = GetPrismaVertexPoint2D(index, (ie + 1) % 3);
    auto distSq = generic::geometry::PointLineDistanceSq(ct, p1, p2);
    return std::sqrt(distSq) * m_model.Scale2Meter();
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetPrismaEdgeLength(size_t index, size_t ie) const
{
    ie %= 3;
    auto p1 = GetPrismaVertexPoint2D(index, ie);
    auto p2 = GetPrismaVertexPoint2D(index, (ie + 1) % 3);
    auto dist = generic::geometry::Distance(p1, p2);
    return dist * m_model.Scale2Meter();
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetPrismaSideArea(size_t index, size_t ie) const
{
    auto h = GetPrismaHeight(index);
    auto w = GetPrismaEdgeLength(index, ie);
    return h * w;
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetPrismaTopBotArea(size_t index) const
{
    const auto & points = m_model.GetPoints();
    const auto & vs = m_model.GetPrisma(index).vertices;
    auto area = generic::geometry::Triangle3D<FCoord>(points.at(vs[0]), points.at(vs[1]), points.at(vs[2])).Area();
    return area * m_model.Scale2Meter() * m_model.Scale2Meter();
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetPrismaVolume(size_t index) const
{
    return GetPrismaTopBotArea(index) * GetPrismaHeight(index);
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetPrismaHeight(size_t index) const
{
    return m_model.GetPrisma(index).layer->thickness * m_model.Scale2Meter();
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetLineJouleHeat(size_t index, EFloat refT) const
{
    const auto & line = m_model.GetLine(m_model.LineLocalIndex(index));
    auto rho = GetMatResistivity(line.matId, refT);
    return rho * GetLineLength(index) * line.current * line.current / GetLineArea(index);
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetLineVolume(size_t index) const
{
    return GetLineArea(index) * GetLineLength(index);
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetLineLength(size_t index) const
{
    const auto & line = m_model.GetLine(m_model.LineLocalIndex(index));
    const auto & p1 = m_model.GetPoint(line.endPoints.front());
    const auto & p2 = m_model.GetPoint(line.endPoints.back());
    return generic::geometry::Distance(p1, p2) * m_model.Scale2Meter();
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetLineArea(size_t index) const
{
    const auto & line = m_model.GetLine(m_model.LineLocalIndex(index));
    return generic::math::pi * std::pow(line.radius * m_model.Scale2Meter(), 2);
}

ECAD_INLINE std::array<EFloat, 3> EPrismaThermalNetworkBuilder::GetMatThermalConductivity(EMaterialId matId, EFloat refT) const
{
    std::array<EFloat, 3> result;
    auto material = m_model.GetMaterialLibrary()->FindMaterialDefById(matId); { ECAD_ASSERT(material) }
    for (size_t i = 0; i < result.size(); ++i) {
        [[maybe_unused]] auto check = material->GetProperty(EMaterialPropId::ThermalConductivity)->GetAnsiotropicProperty(refT, i, result[i]);
        ECAD_ASSERT(check)
    }
    return result;
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetMatMassDensity(EMaterialId matId, EFloat refT) const
{
    EFloat result{0};
    auto material = m_model.GetMaterialLibrary()->FindMaterialDefById(matId); { ECAD_ASSERT(material) }
    [[maybe_unused]] auto check = material->GetProperty(EMaterialPropId::MassDensity)->GetSimpleProperty(refT, result);
    ECAD_ASSERT(check)
    return result;
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetMatSpecificHeat(EMaterialId matId, EFloat refT) const
{
    EFloat result{0};
    auto material = m_model.GetMaterialLibrary()->FindMaterialDefById(matId); { ECAD_ASSERT(material) }
    [[maybe_unused]] auto check = material->GetProperty(EMaterialPropId::SpecificHeat)->GetSimpleProperty(refT, result);
    ECAD_ASSERT(check)
    return result;
}

ECAD_INLINE EFloat EPrismaThermalNetworkBuilder::GetMatResistivity(EMaterialId matId, EFloat refT) const
{
    EFloat result{0};
    auto material = m_model.GetMaterialLibrary()->FindMaterialDefById(matId); { ECAD_ASSERT(material) }
    [[maybe_unused]] auto check = material->GetProperty(EMaterialPropId::Resistivity)->GetSimpleProperty(refT, result);
    ECAD_ASSERT(check)
    return result; 
}

} // namespace ecad::solver