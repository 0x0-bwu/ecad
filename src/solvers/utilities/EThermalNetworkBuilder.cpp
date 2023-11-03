#include "solvers/utilities/EThermalNetworkBuilder.h"

namespace ecad {
namespace esolver {

using namespace emodel;

ECAD_INLINE EGridThermalNetworkBuilder::EGridThermalNetworkBuilder(const EGridThermalModel & model)
 : m_model(model), m_size(model.ModelSize())
{
}

ECAD_INLINE EGridThermalNetworkBuilder::~EGridThermalNetworkBuilder()
{
}

ECAD_INLINE UPtr<ThermalNetwork<ESimVal> > EGridThermalNetworkBuilder::Build(const std::vector<ESimVal> & iniT) const
{
    const size_t size = m_model.TotalGrids(); 
    if(iniT.size() != size) return nullptr;

    summary.Reset();
    summary.totalNodes = size;
    auto network = std::make_unique<ThermalNetwork<ESimVal> >(size);

    //power
    const auto & layers = m_model.GetLayers();
    for(size_t z = 0; z < layers.size(); ++z) {
        const auto & layer = layers.at(z);
        auto pwrModel = layer.GetPowerModel();
        if(nullptr == pwrModel) continue;
        ApplyBoundaryConditionForLayer(iniT, *pwrModel, EGridThermalModel::BCType::HeatFlow, z, *network);
    }

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
        auto k = GetConductingMatK(std::get<0>(jc), iniT.at(index1))[0];
        network->SetR(index1, index2, k * std::get<2>(jc));
    }
    

    //bc
    SPtr<EGridBCModel> topBC = nullptr, botBC = nullptr;
    m_model.GetTopBotBCModel(topBC, botBC);

    EGridThermalModel::BCType topT, botT;
    m_model.GetTopBotBCType(topT, botT);

    if(topBC) ApplyBoundaryConditionForLayer(iniT, *topBC, topT, 0, *network);
    if(botBC) ApplyBoundaryConditionForLayer(iniT, *botBC, botT, m_size.z - 1, *network);

    return network;
}

ECAD_INLINE void EGridThermalNetworkBuilder::ApplyBoundaryConditionForLayer(const std::vector<ESimVal> & iniT, const EGridDataTable & dataTable, EGridThermalModel::BCType type, size_t layer, ThermalNetwork<ESimVal> & network) const
{
    bool success;
    for(size_t x = 0; x < m_size.x; ++x) {
        for(size_t y = 0; y < m_size.y; ++y) {
            auto grid = ESize3D(x, y, layer);
            auto index = GetFlattenIndex(grid);
            auto val = dataTable.Query(iniT.at(index), x, y, &success);
            if(!success) continue;
            switch(type) {
                case EGridThermalModel::BCType::HTC : {
                    summary.boundaryNodes += 1;
                    network.SetHTC(index, GetZGridArea() * val);
                    break;
                }
                case EGridThermalModel::BCType::HeatFlow : {
                    if(val > 0) summary.iHeatFlow += val;
                    else summary.oHeatFlow -= val;
                    network.SetHF(index, val);
                    break;
                }
                case EGridThermalModel::BCType::Temperature : {
                    summary.fixedTNodes += 1;
                    network.SetT(index, val);
                    break;
                }
            }
        }
    }
}

ECAD_INLINE ESimVal EGridThermalNetworkBuilder::GetMetalComposite(const ESize3D & index) const
{
    return m_model.GetLayers().at(index.z).GetMetalFraction(index.x, index.y);
}

ECAD_INLINE ESimVal EGridThermalNetworkBuilder::GetCap(ESimVal c, ESimVal rho, FCoord z, FCoord area) const
{
    return c * rho * z * area;
}

ECAD_INLINE ESimVal EGridThermalNetworkBuilder::GetRes(ESimVal k1, FCoord z1, ESimVal k2, FCoord z2, FCoord area) const
{
    return (z1 / k1 + z2 / k2) / area;
}

ECAD_INLINE std::array<ESimVal, 3> EGridThermalNetworkBuilder::GetCompositeMatK(const ESize3D & index, ESimVal refT) const
{
    auto mK = GetConductingMatK(index, refT);
    auto dK = GetDielectricMatK(index, refT);
    auto cp = GetMetalComposite(index);
    std::array<ESimVal, 3> k;
    for(size_t i = 0; i < 3; ++i) {
        k[i] = cp * mK[i] + (1 - cp) * dK[i];
    }
    return k;
}

ECAD_INLINE std::array<ESimVal, 3> EGridThermalNetworkBuilder::GetConductingMatK(const ESize3D & index, ESimVal refT) const
{
    ECAD_UNUSED(refT)
    return GetConductingMatK(index.z, refT);
}

ECAD_INLINE std::array<ESimVal, 3> EGridThermalNetworkBuilder::GetDielectricMatK(const ESize3D & index, ESimVal refT) const
{
    ECAD_UNUSED(refT)
    return GetDielectircMatK(index.z, refT);
}

ECAD_INLINE std::array<ESimVal, 3> EGridThermalNetworkBuilder::GetConductingMatK(size_t layer, ESimVal refT) const
{
    //todo
    ECAD_UNUSED(refT)
    return std::array<ESimVal, 3>{400, 400, 400};
}

ECAD_INLINE std::array<ESimVal, 3> EGridThermalNetworkBuilder::GetDielectircMatK(size_t layer, ESimVal refT) const
{
    //todo
    ECAD_UNUSED(refT)
    return std::array<ESimVal, 3>{70, 70, 70};
}

ECAD_INLINE std::array<ESimVal, 3> EGridThermalNetworkBuilder::GetDefaultAirK() const
{
    //todo
    return std::array<ESimVal, 3>{0.026, 0.026, 0.026};
}

ECAD_INLINE ESimVal EGridThermalNetworkBuilder::GetCompositeMatC(const ESize3D & index, FCoord z, FCoord area, ESimVal refT) const
{
    auto mCap = GetCap(GetConductingMatC(index, refT), GetConductingMatRho(index, refT), z, area);
    auto dCap = GetCap(GetDielectricMatC(index, refT), GetConductingMatRho(index, refT), z, area);
    auto cp = GetMetalComposite(index);
    return cp * mCap + (1 - cp) * dCap;
}

ECAD_INLINE ESimVal EGridThermalNetworkBuilder::GetConductingMatC(const ESize3D & index, ESimVal refT) const
{
    //todo
    return 380;//J/(KG.K)
}

ECAD_INLINE ESimVal EGridThermalNetworkBuilder::GetDielectricMatC(const ESize3D & index, ESimVal refT) const
{
    //todo
    return 691;//J(KG.K)
}

ECAD_INLINE ESimVal EGridThermalNetworkBuilder::GetConductingMatRho(const ESize3D & index, ESimVal refT) const
{
    //todo
    return 8850;//Kg/m^3
}
ECAD_INLINE ESimVal EGridThermalNetworkBuilder::GetDielectricMatRho(const ESize3D & index, ESimVal refT) const
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

}//namespace esolver
}//namespace ecad