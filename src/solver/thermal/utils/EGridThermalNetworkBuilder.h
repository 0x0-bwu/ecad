#pragma once
#include "EThermalNetworkBuilder.h"
#include "model/thermal/EGridThermalModel.h"
#include "solver/thermal/network/ThermalNetwork.h"
namespace ecad {
namespace solver {

using namespace ecad::model;

template <typename Scalar>
class ECAD_API EGridThermalNetworkBuilder : public EThermalNetworkBuilder
{
    enum class Axis { X = 0, Y = 1, Z = 2};
public:
    using ModelType = EGridThermalModel;
    using Network = thermal::model::ThermalNetwork<Scalar>;
    enum class Orientation { Top, Bot, Left, Right, Front, End };
    explicit EGridThermalNetworkBuilder(const ModelType & model);
    virtual ~EGridThermalNetworkBuilder() = default;

    UPtr<Network> Build(const std::vector<Scalar> & iniT) const;

private:
    void ApplyHeatFlowForLayer(const std::vector<Scalar> & iniT, const EGridDataTable & dataTable, size_t layer, Network & network) const;
    void ApplyUniformBoundaryConditionForLayer(const EThermalBoundaryCondition & bc, size_t layer, Network & network) const;
    void ApplyBlockBoundaryConditionForLayer(const EThermalBoundaryCondition & bc, size_t layer, const ESize2D & ll, const ESize2D & ur, Network & network) const;

public:
    EFloat GetMetalComposite(const ESize3D & index) const;
    EFloat GetCap(EFloat c, EFloat rho, EFloat z, EFloat area) const;
    EFloat GetRes(EFloat k1, EFloat z1, EFloat k2, EFloat z2, EFloat area) const;
    std::array<EFloat, 3> GetCompositeMatK(const ESize3D & index, EFloat refT) const;
    std::array<EFloat, 3> GetConductingMatK(const ESize3D & index, EFloat refT) const;
    std::array<EFloat, 3> GetDielectricMatK(const ESize3D & index, EFloat refT) const;
    std::array<EFloat, 3> GetConductingMatK(size_t layer, EFloat refT) const;
    std::array<EFloat, 3> GetDielectricMatK(size_t layer, EFloat refT) const;
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

template <typename Scalar>
ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder<Scalar>::GetXGridLength() const
{
    return m_model.GetResolution().at(0) * m_model.GetScaleH();
}

template <typename Scalar>
ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder<Scalar>::GetYGridLength() const
{
    return m_model.GetResolution().at(1) * m_model.GetScaleH();
}

template <typename Scalar>
ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder<Scalar>::GetZGridLength(size_t layer) const
{
    return m_model.GetLayers().at(layer).GetThickness();
}

template <typename Scalar>
ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder<Scalar>::GetXGridArea(size_t layer) const
{
    return GetZGridLength(layer) * GetYGridLength();
}

template <typename Scalar>
ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder<Scalar>::GetYGridArea(size_t layer) const
{
    return GetZGridLength(layer) * GetXGridLength();
}

template <typename Scalar>
ECAD_ALWAYS_INLINE EFloat EGridThermalNetworkBuilder<Scalar>::GetZGridArea() const
{
    return GetXGridLength() * GetYGridLength();
}

template <typename Scalar>
ECAD_ALWAYS_INLINE size_t EGridThermalNetworkBuilder<Scalar>::GetFlattenIndex(const ESize3D & index) const
{
    return m_model.GetFlattenIndex(index);
}

template <typename Scalar>
ECAD_ALWAYS_INLINE ESize3D EGridThermalNetworkBuilder<Scalar>::GetGridIndex(size_t index) const
{
    return m_model.GetGridIndex(index);
}

template <typename Scalar>
ECAD_ALWAYS_INLINE bool EGridThermalNetworkBuilder<Scalar>::isValid(const ESize3D & index)
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
}//namespace solver
}//namespace ecad