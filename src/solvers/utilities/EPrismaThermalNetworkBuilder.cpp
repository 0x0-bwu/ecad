#include "EPrismaThermalNetworkBuilder.h"

namespace ecad::esolver {

using namespace emodel;
ECAD_INLINE EPrismaThermalNetworkBuilder::EPrismaThermalNetworkBuilder(const EPrismaThermalModel & model)
 : m_model(model)
{
}

ECAD_INLINE UPtr<ThermalNetwork<ESimVal> > EPrismaThermalNetworkBuilder::Build(const std::vector<ESimVal> & iniT) const
{
    const size_t size = m_model.TotalElements();
    if(iniT.size() != size) return nullptr;

    summary.Reset();
    summary.totalNodes = size;
    auto network = std::make_unique<ThermalNetwork<ESimVal> >(size);

    EThermalModel::BCType topType, botType;
    m_model.GetTopBotBCType(topType, botType);

    ESimVal uniformTopBC, uniformBotBC;
    m_model.GetUniformTopBotBCValue(uniformTopBC, uniformBotBC);

    for (size_t i = 0; i < size; ++i) {
        const auto & inst = m_model[i];
        if (auto p = inst.element->avePower; p > 0) {
            summary.iHeatFlow += p;
            network->AddHF(i, p);
        }

        auto c = GetMaterialC(inst.element->matId, iniT.at(i));
        auto rho = GetMaterialRho(inst.element->matId, iniT.at(i));
        auto vol = GetElementVolume(i);
        network->SetC(i, c * rho * vol);

        auto k = GetMaterialK(inst.element->matId, iniT.at(i));
        auto ct = GetElementCenterPoint2D(i);

        const auto & neighbors = inst.neighbors;
        //edges
        for (size_t ie = 0; ie < 3; ++ie) {
            auto area = GetElementSideArea(i, ie);
            if (auto nid = neighbors.at(ie); tri::noNeighbor == nid) {
                if (m_model.uniformBcSide != invalidSimVal) {
                    if (EThermalModel::BCType::HTC == m_model.sideBCType) {
                        network->AddHTC(i, m_model.uniformBcSide * area);
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
            else if (true/*i < nid*/) { //one way
                const auto & nb = m_model[nid];
                auto ctNb = GetElementCenterPoint2D(nid);
                auto vec = ctNb - ct;
                auto dist = vec.Norm2();
                auto cos2 = std::pow(vec[0] / dist, 2);
                auto sin2 = std::pow(vec[1] / dist, 2);
                auto kxy = cos2 * k[0] + sin2 * k[1];
                auto dist2edge = GetElementCenterDist2Side(i, ie);
                auto r1 = dist2edge / kxy / area;

                auto kNb = GetMaterialK(nb.element->matId, iniT.at(nid));
                auto kNbxy = cos2 * kNb[0] + sin2 * kNb[1];
                auto r2 = (dist - dist2edge) / kNbxy / area;
                network->SetR(i, nid, r1 + r2);
            }
        }
        auto height = GetElementHeight(i);
        auto area = GetElementTopBotArea(i);
        //top
        auto nTop = neighbors.at(EPrismaThermalModel::PrismaElement::TOP_NEIGHBOR_INDEX);
        if (tri::noNeighbor == nTop) {
            if (uniformTopBC != invalidSimVal) {
                if (EThermalModel::BCType::HTC == topType) {
                    network->AddHTC(i, uniformTopBC * area);
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
        else if (true/*i < nTop*/) {
            const auto & nb = m_model[nTop];
            auto hNb = GetElementHeight(nTop);
            auto kNb = GetMaterialK(nb.element->matId, iniT.at(nTop));
            auto r = (0.5 * height / k[2] + 0.5 * hNb / kNb[2]) / area;
            network->SetR(i, nTop, r);
        }
        //bot
        auto nBot = neighbors.at(EPrismaThermalModel::PrismaElement::BOT_NEIGHBOR_INDEX);
        if (tri::noNeighbor == nBot) {
            if (uniformBotBC != invalidSimVal) {
                if (EThermalModel::BCType::HTC == botType) {
                    network->AddHTC(i, uniformBotBC * area);
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
        else if (true/*i < nBot*/) {
            const auto & nb = m_model[nBot];
            auto hNb = GetElementHeight(nBot);
            auto kNb = GetMaterialK(nb.element->matId, iniT.at(nBot));
            auto r = (0.5 * height / k[2] + 0.5 * hNb / kNb[2]) / area;
            network->SetR(i, nBot, r);
        }
    }
    return network;
}

ECAD_INLINE const FPoint3D & EPrismaThermalNetworkBuilder::GetElementVertexPoint(size_t index, size_t iv) const
{
    const auto & points = m_model.GetPoints();
    return points.at(m_model[index].vertices.at(iv));
}

ECAD_INLINE FPoint2D EPrismaThermalNetworkBuilder::GetElementVertexPoint2D(size_t index, size_t iv) const
{
    const auto & pt3d = GetElementVertexPoint(index, iv);
    return FPoint2D(pt3d[0], pt3d[1]);
}

ECAD_INLINE FPoint2D EPrismaThermalNetworkBuilder::GetElementCenterPoint2D(size_t index) const
{
    auto p0 = GetElementVertexPoint2D(index, 0);
    auto p1 = GetElementVertexPoint2D(index, 1);
    auto p2 = GetElementVertexPoint2D(index, 2);
    return generic::geometry::Triangle2D<FCoord>(p0, p1, p2).Center();
}

ECAD_INLINE EValue EPrismaThermalNetworkBuilder::GetElementCenterDist2Side(size_t index, size_t ie) const
{
    ie %= 3;
    auto ct = GetElementCenterPoint2D(index);
    auto p1 = GetElementVertexPoint2D(index, ie);
    auto p2 = GetElementVertexPoint2D(index, (ie + 1) % 3);
    auto distSq = generic::geometry::PointLineDistanceSq(ct, p1, p2);
    return std::sqrt(distSq) * m_model.Scale2Meter();
}

ECAD_INLINE EValue EPrismaThermalNetworkBuilder::GetElementEdgeLength(size_t index, size_t ie) const
{
    ie %= 3;
    auto p1 = GetElementVertexPoint2D(index, ie);
    auto p2 = GetElementVertexPoint2D(index, (ie + 1) % 3);
    auto dist = generic::geometry::Distance(p1, p2);
    return dist * m_model.Scale2Meter();
}

ECAD_INLINE EValue EPrismaThermalNetworkBuilder::GetElementSideArea(size_t index, size_t ie) const
{
    auto h = GetElementHeight(index);
    auto w = GetElementEdgeLength(index, ie);
    return h * w;
}

ECAD_INLINE EValue EPrismaThermalNetworkBuilder::GetElementTopBotArea(size_t index) const
{
    const auto & points = m_model.GetPoints();
    const auto & vs = m_model[index].vertices;
    auto area = generic::geometry::Triangle3D<FCoord>(points.at(vs[0]), points.at(vs[1]), points.at(vs[2])).Area();
    return area * m_model.Scale2Meter() * m_model.Scale2Meter();
}

ECAD_INLINE EValue EPrismaThermalNetworkBuilder::GetElementVolume(size_t index) const
{
    return GetElementTopBotArea(index) * GetElementHeight(index);
}

ECAD_INLINE EValue EPrismaThermalNetworkBuilder::GetElementHeight(size_t index) const
{
    return m_model[index].layer->thickness * m_model.Scale2Meter();
}
    
} // namespace ecad::esolver