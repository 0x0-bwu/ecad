#pragma once
#include "models/thermal/EGridThermalModel.h"
#include "thermal/model/ThermalNetwork.hpp"
namespace ecad {
namespace esolver {

using namespace emodel::etherm;
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
    mutable EGridThermalNetworkBuildSummary summary;
    enum class Orientation { Top, Bot, Left, Right, Front, End };
    explicit EGridThermalNetworkBuilder(const EGridThermalModel & model);
    virtual ~EGridThermalNetworkBuilder();

    UPtr<ThermalNetwork<ESimVal> > Build(const std::vector<ESimVal> & iniT) const;

private:
    void ApplyBoundaryConditionForLayer(const std::vector<ESimVal> & iniT, const EGridDataTable & dataTable, EGridThermalModel::BCType type, size_t layer, ThermalNetwork<ESimVal> & network) const;
    void ApplyUniformBoundaryConditionForLayer(ESimVal value, EGridThermalModel::BCType type, size_t layer, ThermalNetwork<ESimVal> & network) const;

public:
    ESimVal GetMetalComposite(const ESize3D & index) const;
    ESimVal GetCap(ESimVal c, ESimVal rho, FCoord z, FCoord area) const;
    ESimVal GetRes(ESimVal k1, FCoord z1, ESimVal k2, FCoord z2, FCoord area) const;
    std::array<ESimVal, 3> GetCompositeMatK(const ESize3D & index, ESimVal refT) const;
    std::array<ESimVal, 3> GetConductingMatK(const ESize3D & index, ESimVal refT) const;
    std::array<ESimVal, 3> GetDielectricMatK(const ESize3D & index, ESimVal refT) const;
    std::array<ESimVal, 3> GetConductingMatK(size_t layer, ESimVal refT) const;
    std::array<ESimVal, 3> GetDielectircMatK(size_t layer, ESimVal refT) const;
    std::array<ESimVal, 3> GetDefaultAirK() const;

    ESimVal GetCompositeMatC(const ESize3D & index, FCoord z, FCoord area, ESimVal refT) const;
    ESimVal GetConductingMatC(const ESize3D & index, ESimVal refT) const;
    ESimVal GetDielectricMatC(const ESize3D & index, ESimVal refT) const;
    ESimVal GetConductingMatRho(const ESize3D & index, ESimVal refT) const;
    ESimVal GetDielectricMatRho(const ESize3D & index, ESimVal refT) const;

public:
    FCoord GetXGridLength() const;
    FCoord GetYGridLength() const;
    FCoord GetZGridLength(size_t layer) const;
    FCoord GetXGridArea(size_t layer) const;
    FCoord GetYGridArea(size_t layer) const;
    FCoord GetZGridArea() const;
    size_t GetFlattenIndex(const ESize3D & index) const;
    size_t GetFlattenNeighbor(size_t index, Orientation o) const;
    size_t GetFlattenNeighbor(ESize3D index, Orientation o) const;
    ESize3D GetGridIndex(size_t index) const;
    ESize3D GetNeighbor(size_t index, Orientation o) const;
    ESize3D GetNeighbor(ESize3D index, Orientation o) const;
    static bool isValid(const ESize3D & index);

private:
    const EGridThermalModel & m_model;
    const ESize3D m_size;
};

ECAD_ALWAYS_INLINE FCoord EGridThermalNetworkBuilder::GetXGridLength() const
{
    return m_model.GetResolution().at(0) * m_model.GetScaleH();
}

ECAD_ALWAYS_INLINE FCoord EGridThermalNetworkBuilder::GetYGridLength() const
{
    return m_model.GetResolution().at(1) * m_model.GetScaleH();
}

ECAD_ALWAYS_INLINE FCoord EGridThermalNetworkBuilder::GetZGridLength(size_t layer) const
{
    return m_model.GetLayers().at(layer).GetThickness();
}

ECAD_ALWAYS_INLINE FCoord EGridThermalNetworkBuilder::GetXGridArea(size_t layer) const
{
    return GetZGridLength(layer) * GetYGridLength();
}

ECAD_ALWAYS_INLINE FCoord EGridThermalNetworkBuilder::GetYGridArea(size_t layer) const
{
    return GetZGridLength(layer) * GetXGridLength();
}

ECAD_ALWAYS_INLINE FCoord EGridThermalNetworkBuilder::GetZGridArea() const
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

//     UPtr<ThermalNetwork<ESimVal> > Build(size_t layer, const ESize2D & index, const std::vector<ESimVal> & iniT) const;
// private:
//     const EGridThermalModel & m_model;
//     const ESize3D m_size;
// };
}//namesapce esolver
}//namespace ecad