#include "EGridThermalNetworkBuilder.h"

namespace ecad {
namespace solver {

using namespace ecad::model;
inline static constexpr EFloat THERMAL_RD = 0.01;
ECAD_INLINE EGridThermalNetworkBuilder::EGridThermalNetworkBuilder(const ModelType & model)
 : m_model(model), m_size(model.ModelSize())
{
}

ECAD_INLINE UPtr<ThermalNetwork<EFloat> > EGridThermalNetworkBuilder::Build(const std::vector<EFloat> & iniT) const
{
    const size_t size = m_model.TotalGrids(); 
    if(iniT.size() != size) return nullptr;

    summary.Reset();
    summary.totalNodes = size;
    auto network = std::make_unique<ThermalNetwork<EFloat> >(size);

    //r, c
    for(size_t index1 = 0; index1 < size; ++index1) {
        auto grid1 = GetGridIndex(index1);

        auto c = GetCompositeMatC(grid1, GetZGridLength(grid1.z), GetZGridArea(), iniT.at(index1));
        network->SetC(index1, c);

        auto k1 = GetCompositeMatK(grid1, iniT.at(index1));
        
        ESize3D grid2;
        //right
        grid2 = GetNeighbor(grid1, Orientation::Right);
        if(isValid(grid2)) {
            auto index2 = GetFlattenIndex(grid2);
            auto k2 = GetCompositeMatK(grid2, iniT.at(index2));
            // auto kx = 0.5 * k1[0] + 0.5 * k2[0];
            // auto r = kx * GetXGridArea(grid1.z) / GetXGridLength();
            auto r = GetRes(k1[0], 0.5 * GetXGridLength(), k2[0], GetXGridLength(), GetXGridArea(grid1.z));
            network->SetR(index1, index2, r);
        }
        //back
        grid2 = GetNeighbor(grid1, Orientation::End);
        if(isValid(grid2)) {
            auto index2 = GetFlattenIndex(grid2);
            auto k2 = GetCompositeMatK(grid2, iniT.at(index2));
            // auto ky = 0.5 * k1[1] + 0.5 * k2[1];
            // auto r = ky * GetYGridArea(grid1.z) / GetYGridLength();
            auto r = GetRes(k1[1], 0.5 * GetYGridLength(), k2[1], 0.5 * GetYGridLength(), GetYGridArea(grid1.z));
            network->SetR(index1, index2, r);
        }
        //bot
        grid2 = GetNeighbor(grid1, Orientation::Bot);
        if(isValid(grid2)) {
            auto index2 = GetFlattenIndex(grid2);
            auto k2 = GetCompositeMatK(grid2, iniT.at(index2));
            // auto z1 = GetZGridLength(grid1.z);
            // auto z2 = GetZGridLength(grid2.z);
            // auto a1 = 1.0 / z1, a2 = 1.0 / z2;
            // auto kz = (a1 * k1[2] + a2 * k2[2]) / (a1 + a2);
            // auto r = kz * GetZGridArea() / (0.5 * z1 + 0.5 * z2);
            auto r = GetRes(k1[2], 0.5 * GetZGridLength(grid1.z), k2[2], 0.5 * GetZGridLength(grid2.z), GetZGridArea());
            network->SetR(index1, index2, r);
        }
    }
    
    //bw
    for (const auto & jc : m_model.GetJumpConnections()) {
        auto index1 = GetFlattenIndex(std::get<0>(jc));
        auto index2 = GetFlattenIndex(std::get<1>(jc));
        if (index1 == index2) continue;
        auto k = GetConductingMatK(std::get<0>(jc), iniT.at(index1))[0];
        network->SetR(index1, index2, k * std::get<2>(jc));
    }

    //power
    const auto & layers = m_model.GetLayers();
    for(size_t z = 0; z < layers.size(); ++z) {
        const auto & layer = layers.at(z);
        auto pwrModels = layer.GetPowerModels();
        for (const auto & pwrModel : pwrModels) {
            if (auto model = dynamic_cast<CPtr<EGridPowerModel>>(pwrModel.get()); model)
                ApplyHeatFlowForLayer(iniT, model->GetTable(), z, *network);
            else if (auto model = dynamic_cast<CPtr<EBlockPowerModel>>(pwrModel.get()); model) {
                auto node = network->AppendNode();
                network->SetHF(node, model->totalPower);
                if(model->totalPower > 0)
                    summary.iHeatFlow += model->totalPower;
                else summary.oHeatFlow += model->totalPower;
                for (size_t x = model->ll.x; x <= model->ur.x; ++x) {
                    for (size_t y = model->ll.y; y <= model->ur.y; ++y) {
                        auto index = GetFlattenIndex(ESize3D(x, y, z));
                        network->SetR(index, node, THERMAL_RD);
                    }
                }
            }
        }
    }

    //bc
    if (auto topBC = m_model.GetUniformBC(EOrientation::Top); topBC && topBC->isValid())
        ApplyUniformBoundaryConditionForLayer(*topBC, 0, *network);
    if (auto botBC = m_model.GetUniformBC(EOrientation::Bot); botBC && botBC->isValid())
        ApplyUniformBoundaryConditionForLayer(*botBC, m_size.z - 1, *network);

    for (const auto & block : m_model.GetBlockBC(EOrientation::Top))
        ApplyBlockBoundaryConditionForLayer(block.second, 0, block.first[0], block.first[1], *network);
    for (const auto & block : m_model.GetBlockBC(EOrientation::Bot))
        ApplyBlockBoundaryConditionForLayer(block.second, m_size.z - 1, block.first[0], block.first[1], *network);
    
    return network;
}

ECAD_INLINE void EGridThermalNetworkBuilder::ApplyHeatFlowForLayer(const std::vector<EFloat> & iniT, const EGridDataTable & dataTable, size_t layer, ThermalNetwork<EFloat> & network) const
{
    bool success;
    for (size_t x = 0; x < m_size.x; ++x) {
        for (size_t y = 0; y < m_size.y; ++y) {
            auto grid = ESize3D(x, y, layer);
            auto index = GetFlattenIndex(grid);
            auto val = dataTable.Query(iniT.at(index), x, y, &success);
            if (not success) continue;
  
            if(val > 0) summary.iHeatFlow += val;
            else summary.oHeatFlow += val;
            network.SetHF(index, val);
        }
    }
}

ECAD_INLINE void EGridThermalNetworkBuilder::ApplyUniformBoundaryConditionForLayer(const EThermalBondaryCondition & bc, size_t layer, ThermalNetwork<EFloat> & network) const
{
    if (not bc.isValid()) return;
    auto value = bc.value;    
    for (size_t x = 0; x < m_size.x; ++x) {
        for (size_t y = 0; y < m_size.y; ++y) {
            auto grid = ESize3D(x, y, layer);
            auto index = GetFlattenIndex(grid);
            switch(bc.type) {
                case EThermalBondaryCondition::BCType::HTC : {
                    summary.boundaryNodes += 1;
                    network.SetHTC(index, GetZGridArea() * value);
                    break;
                }
                case EThermalBondaryCondition::BCType::HeatFlux : {
                    auto heatFlow = GetZGridArea() * value;
                    if(heatFlow > 0) summary.iHeatFlow += heatFlow;
                    else summary.oHeatFlow += heatFlow;
                    network.SetHF(index, heatFlow);
                    break;
                }
                default : {
                 GENERIC_ASSERT(false)
                }
            }
        }
    }
}

ECAD_INLINE void EGridThermalNetworkBuilder::ApplyBlockBoundaryConditionForLayer(const EThermalBondaryCondition & bc, size_t layer, const ESize2D & ll, const ESize2D & ur, ThermalNetwork<EFloat> & network) const
{
    if (not bc.isValid()) return;
    auto value = bc.value;
    for (size_t x = ll.x; x <= ur.x; ++x) {
        for (size_t y = ll.y; y <= ur.y; ++y) {
            auto grid = ESize3D(x, y, layer);
            auto index = GetFlattenIndex(grid);
            switch(bc.type) {
                case EThermalBondaryCondition::BCType::HTC : {
                    summary.boundaryNodes += 1;
                    network.SetHTC(index, GetZGridArea() * value);
                    break;
                }
                case EThermalBondaryCondition::BCType::HeatFlux : {
                    auto heatFlow = value * GetZGridArea();
                    if(heatFlow > 0) summary.iHeatFlow += heatFlow;
                    else summary.oHeatFlow += heatFlow;
                    network.SetHF(index, heatFlow);
                    break;
                }
                default : {
                 GENERIC_ASSERT(false)
                }
            }
        }
    }
}

ECAD_INLINE EFloat EGridThermalNetworkBuilder::GetMetalComposite(const ESize3D & index) const
{
    return m_model.GetLayers().at(index.z).GetMetalFraction(index.x, index.y);
}

ECAD_INLINE EFloat EGridThermalNetworkBuilder::GetCap(EFloat c, EFloat rho, EFloat z, EFloat area) const
{
    return c * rho * z * area;
}

ECAD_INLINE EFloat EGridThermalNetworkBuilder::GetRes(EFloat k1, EFloat z1, EFloat k2, EFloat z2, EFloat area) const
{
    return (z1 / k1 + z2 / k2) / area;
}

ECAD_INLINE std::array<EFloat, 3> EGridThermalNetworkBuilder::GetCompositeMatK(const ESize3D & index, EFloat refT) const
{
    auto mK = GetConductingMatK(index, refT);
    auto dK = GetDielectricMatK(index, refT);
    auto cp = GetMetalComposite(index);
    std::array<EFloat, 3> k;
    for(size_t i = 0; i < 3; ++i) {
        k[i] = cp * mK[i] + (1 - cp) * dK[i];
    }
    return k;
}

ECAD_INLINE std::array<EFloat, 3> EGridThermalNetworkBuilder::GetConductingMatK(const ESize3D & index, EFloat refT) const
{
    ECAD_UNUSED(refT)
    return GetConductingMatK(index.z, refT);
}

ECAD_INLINE std::array<EFloat, 3> EGridThermalNetworkBuilder::GetDielectricMatK(const ESize3D & index, EFloat refT) const
{
    ECAD_UNUSED(refT)
    return GetDielectircMatK(index.z, refT);
}

ECAD_INLINE std::array<EFloat, 3> EGridThermalNetworkBuilder::GetConductingMatK(size_t layer, EFloat refT) const
{
    //todo
    ECAD_UNUSED(refT)
    return std::array<EFloat, 3>{400, 400, 400};
}

ECAD_INLINE std::array<EFloat, 3> EGridThermalNetworkBuilder::GetDielectircMatK(size_t layer, EFloat refT) const
{
    //todo
    ECAD_UNUSED(refT)
    return std::array<EFloat, 3>{70, 70, 70};
}

ECAD_INLINE std::array<EFloat, 3> EGridThermalNetworkBuilder::GetDefaultAirK() const
{
    //todo
    return std::array<EFloat, 3>{0.026, 0.026, 0.026};
}

ECAD_INLINE EFloat EGridThermalNetworkBuilder::GetCompositeMatC(const ESize3D & index, EFloat z, EFloat area, EFloat refT) const
{
    auto mCap = GetCap(GetConductingMatC(index, refT), GetConductingMatRho(index, refT), z, area);
    auto dCap = GetCap(GetDielectricMatC(index, refT), GetConductingMatRho(index, refT), z, area);
    auto cp = GetMetalComposite(index);
    return cp * mCap + (1 - cp) * dCap;
}

ECAD_INLINE EFloat EGridThermalNetworkBuilder::GetConductingMatC(const ESize3D & index, EFloat refT) const
{
    //todo
    return 380;//J/(KG.K)
}

ECAD_INLINE EFloat EGridThermalNetworkBuilder::GetDielectricMatC(const ESize3D & index, EFloat refT) const
{
    //todo
    return 691;//J(KG.K)
}

ECAD_INLINE EFloat EGridThermalNetworkBuilder::GetConductingMatRho(const ESize3D & index, EFloat refT) const
{
    //todo
    return 8850;//Kg/m^3
}
ECAD_INLINE EFloat EGridThermalNetworkBuilder::GetDielectricMatRho(const ESize3D & index, EFloat refT) const
{
    //todo
    return 2400;//Kg/m^3
}

ECAD_INLINE ESize3D EGridThermalNetworkBuilder::GetNeighbor(size_t index, Orientation o) const
{
    return GetNeighbor(GetGridIndex(index), o);
}

ECAD_INLINE ESize3D EGridThermalNetworkBuilder::GetNeighbor(ESize3D index, Orientation o) const
{
    switch(o) {
        case Orientation::Top : {
            if(index.z == 0)
                index.z = invalidIndex;
            else index.z -= 1;
            break;
        }
        case Orientation::Bot : {
            if(index.z == (m_size.z - 1))
                index.z = invalidIndex;
            else index.z += 1;
            break;
        }
        case Orientation::Left : {
            if(index.x == 0)
                index.x = invalidIndex;
            else index.x -= 1;
            break;
        }
        case Orientation::Right : {
            if(index.x == (m_size.x - 1))
                index.x = invalidIndex;
            else index.x += 1;
            break;
        }
        case Orientation::Front : {
            if(index.y == 0)
                index.y = invalidIndex;
            else index.y -= 1;
            break;
        }
        case Orientation::End : {
            if(index.y == (m_size.y - 1))
                index.y = invalidIndex;
            else index.y += 1;
            break;
        }
    }
    return index;
}

ECAD_INLINE size_t EGridThermalNetworkBuilder::GetFlattenNeighbor(size_t index, Orientation o) const
{
    return GetFlattenNeighbor(GetGridIndex(index), o);
}

ECAD_INLINE size_t EGridThermalNetworkBuilder::GetFlattenNeighbor(ESize3D index, Orientation o) const
{
    return GetFlattenIndex(GetNeighbor(index, o));
}

}//namespace solver
}//namespace ecad