#pragma once
#include "model/thermal/EGridThermalModel.h"
#include "solver/thermal/network/ThermalNetwork.h"
namespace ecad {
namespace solver {

using namespace ecad::model;
using namespace thermal::model;

struct EGridThermalNetworkBuildSummary
{
    size_t totalNodes = 0;
    size_t fixedTNodes = 0;
    size_t boundaryNodes = 0;
    double iHeatFlow = 0, oHeatFlow = 0;
    void Reset() { *this = EGridThermalNetworkBuildSummary{}; }
};

class ECAD_API EGridThermalNetworkBuilder
{
    enum class Axis { X = 0, Y = 1, Z = 2};
public:
    using ModelType = EGridThermalModel;
    mutable EGridThermalNetworkBuildSummary summary;
    enum class Orientation { Top, Bot, Left, Right, Front, End };
    explicit EGridThermalNetworkBuilder(const ModelType & model);
    virtual ~EGridThermalNetworkBuilder() = default;

    UPtr<ThermalNetwork<EFloat> > Build(const std::vector<EFloat> & iniT) const;

private:
    void ApplyHeatFlowForLayer(const std::vector<EFloat> & iniT, const EGridDataTable & dataTable, size_t layer, ThermalNetwork<EFloat> & network) const;
    void ApplyUniformBoundaryConditionForLayer(const EThermalBondaryCondition & bc, size_t layer, ThermalNetwork<EFloat> & network) const;
    void ApplyBlockBoundaryConditionForLayer(const EThermalBondaryCondition & bc, size_t layer, const ESize2D & ll, const ESize2D & ur, ThermalNetwork<EFloat> & network) const;

public:
    EFloat GetMetalComposite(const ESize3D & index) const;
    EFloat GetCap(EFloat c, EFloat rho, EFloat z, EFloat area) const;
    EFloat GetRes(EFloat k1, EFloat z1, EFloat k2, EFloat z2, EFloat area) const;
    std::array<EFloat, 3> GetCompositeMatK(const ESize3D & index, EFloat refT) const;
    std::array<EFloat, 3> GetConductingMatK(const ESize3D & index, EFloat refT) const;
    std::array<EFloat, 3> GetDielectricMatK(const ESize3D & index, EFloat refT) const;
    std::array<EFloat, 3> GetConductingMatK(size_t layer, EFloat refT) const;
    std::array<EFloat, 3> GetDielectircMatK(size_t layer, EFloat refT) const;
    std::array<EFloat, 3> GetDefaultAirK() const;

    EFloat GetCompositeMatC(const ESize3D & index, EFloat z, EFloat area, EFloat refT) const;
    EFloat GetConductingMatC(const ESize3D & index, EFloat refT) const;
    EFloat GetDielectricMatC(const ESize3D & index, EFloat refT) const;
    EFloat GetConductingMatRho(const ESize3D & index, EFloat refT) const;
    EFloat GetDielectricMatRho(const ESize3D & index, EFloat refT) const;

public:
    EFloat GetXGridLength() const;
    EFloat GetYGridLength() const;
    EFloat GetZGridLength(size_t layer) const;
    EFloat GetXGridArea(size_t layer) const;
    EFloat GetYGridArea(size_t layer) const;
    EFloat GetZGridArea() const;
    size_t GetFlattenIndex(const ESize3D & index) const;
    size_t GetFlattenNeighbor(size_t index, Orientation o) const;
    size_t GetFlattenNeighbor(ESize3D index, Orientation o) const;
    ESize3D GetGridIndex(size_t index) const;
    ESize3D GetNeighbor(size_t index, Orientation o) const;
    ESize3D GetNeighbor(ESize3D index, Orientation o) const;
    static bool isValid(const ESize3D & index);

private:
    const ModelType & m_model;
    const ESize3D m_size;
};

ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder::GetXGridLength() const
{
    return m_model.GetResolution().at(0) * m_model.GetScaleH();
}

ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder::GetYGridLength() const
{
    return m_model.GetResolution().at(1) * m_model.GetScaleH();
}

ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder::GetZGridLength(size_t layer) const
{
    return m_model.GetLayers().at(layer).GetThickness();
}

ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder::GetXGridArea(size_t layer) const
{
    return GetZGridLength(layer) * GetYGridLength();
}

ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder::GetYGridArea(size_t layer) const
{
    return GetZGridLength(layer) * GetXGridLength();
}

ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder::GetZGridArea() const
{
    return GetXGridLength() * GetYGridLength();
}

ECAD_ALWAYS_INLINE size_t EGridThermalNetworkBuilder::GetFlattenIndex(const ESize3D & index) const
{
    return m_model.GetFlattenIndex(index);
}

ECAD_ALWAYS_INLINE ESize3D EGridThermalNetworkBuilder::GetGridIndex(size_t index) const
{
    return m_model.GetGridIndex(index);
}

ECAD_ALWAYS_INLINE bool EGridThermalNetworkBuilder::isValid(const ESize3D & index)
{
    return index.x != invalidIndex && index.y != invalidIndex && index.z != invalidIndex;
}


// class ECAD_API EGridSubThermalNetworkBuilder
// {
// public:
//     explicit EGridSubThermalNetworkBuilder(const EGridThermalModel & model);
//     virtual ~EGridSubThermalNetworkBuilder();

//     UPtr<ThermalNetwork<EFloat> > Build(size_t layer, const ESize2D & index, const std::vector<EFloat> & iniT) const;
// private:
//     const EGridThermalModel & m_model;
//     const ESize3D m_size;
// };
}//namesapce solver
}//namespace ecad