#pragma once
#include "EThermalModel.h"

namespace ecad {

class IMaterialDef;
namespace emodel {
namespace etherm {

using namespace eutils;

using EGridBCModel = EGridDataTable;

namespace utils {
class EGridThermalModelReduction;
}//namespace utils;

class ECAD_API EGridThermalLayer
{
    friend class utils::EGridThermalModelReduction;
public:
    explicit EGridThermalLayer(std::string name, SPtr<ELayerMetalFraction> metalFraction = nullptr);
    virtual ~EGridThermalLayer();

    const std::string & GetName() const;

    void SetIsMetal(bool isMetal);
    bool isMetalLayer() const;

    void SetThickness(FCoord thickness);
    FCoord GetThickness() const;

    void SetTopLayer(const std::string & name);
    const std::string & GetTopLayer() const;

    void SetBotLayer(const std::string & name);
    const std::string & GetBotLayer() const;

    void SetConductingMaterial(CPtr<IMaterialDef> material);
    CPtr<IMaterialDef> GetConductingMaterial() const;

    void SetDielectricMaterial(CPtr<IMaterialDef> material);
    CPtr<IMaterialDef> GetDielectricMaterial() const;

    bool AddPowerModel(SPtr<EThermalPowerModel> pwrModel);
    const std::vector<SPtr<EThermalPowerModel> > & GetPowerModels() const;

    bool SetMetalFraction(SPtr<ELayerMetalFraction> mf);
    SPtr<ELayerMetalFraction> GetMetalFraction() const;
    ESimVal GetMetalFraction(size_t x, size_t y) const;

    ESize2D GetSize() const;

private:
    bool m_isMetal = false;
    FCoord m_thickness = 0;//unit: m
    std::string m_name;
    std::string m_topLayer;
    std::string m_botLayer;
    CPtr<IMaterialDef> m_conductingMat = nullptr;
    CPtr<IMaterialDef> m_dielectricMat = nullptr;
    std::vector<SPtr<EThermalPowerModel> > m_powerModel;
    SPtr<ELayerMetalFraction> m_metalFraction = nullptr;
};

class ECAD_API EGridThermalModel : public EThermalModel
{
    friend class utils::EGridThermalModelReduction;
public:
    enum class BCType { HTC, HeatFlow, Temperature };
    explicit EGridThermalModel(const ESize2D & size, const FPoint2D & ref = FPoint2D(0, 0), FCoord elevation = 0);
    virtual ~EGridThermalModel();

    FCoord TotalThickness() const;
    FBox2D GetRegion(bool scaled = false) const;
    
    size_t TotalLayers() const;
    size_t TotalGrids() const;
    ESize3D ModelSize() const;
    const ESize2D & GridSize() const;

    bool SetScaleH(FCoord scaleH);
    FCoord GetScaleH() const;

    bool SetResolution(FCoord x, FCoord y);
    void GetResolution(FCoord & x, FCoord & y, bool scaled = false) const;
    std::array<FCoord, 2> GetResolution(bool scaled = false) const;

    size_t AppendLayer(EGridThermalLayer layer);

    void AppendJumpConnection(ESize3D start, ESize3D end, FCoord alpha); //alpha = pi * r * r / L
    const std::vector<std::tuple<ESize3D, ESize3D, FCoord> > & GetJumpConnections() const;

    std::vector<EGridThermalLayer> & GetLayers();
    const std::vector<EGridThermalLayer> & GetLayers() const; 

    bool AddPowerModel(size_t layer, SPtr<EThermalPowerModel> pwrModel);
    const std::vector<SPtr<EThermalPowerModel> > & GetPowerModels(size_t layer) const;

    void SetUniformTopBotBCValue(ESimVal top, ESimVal bot);
    void GetUniformTopBotBCValue(ESimVal & t, ESimVal & b) const;
    bool SetTopBotBCModel(SPtr<EGridBCModel> top, SPtr<EGridBCModel> bot);
    void GetTopBotBCModel(SPtr<EGridBCModel> & top, SPtr<EGridBCModel> & bot) const;

    void SetTopBotBCType(BCType top, BCType bot);
    void GetTopBotBCType(BCType & top, BCType & bot) const;

    size_t GetFlattenIndex(const ESize3D & index) const;
    ESize3D GetGridIndex(size_t index) const;
    bool isValid(const ESize2D & index) const;
    bool isValid(const ESize3D & index) const;

    bool NeedIteration() const;

private:
    ESize2D m_size;
    FPoint2D m_ref;
    FCoord m_elevation;
    FCoord m_scaleH = 1.0;//only apply for horizontal
    std::array<FCoord, 2> m_resolution = {0, 0};//unit: m
    std::vector<EGridThermalLayer> m_stackupLayers;
    std::array<SPtr<EGridBCModel>, 2> m_bcTopBot = {nullptr, nullptr};
    std::array<BCType, 2> m_bcTypeTopBot = {BCType::HTC, BCType::HTC};
    std::vector<std::tuple<ESize3D, ESize3D, FCoord> > m_jumpConnects;
    std::array<ESimVal, 2> m_uniformBcTopBot{invalidSimVal, invalidSimVal};
};

ECAD_ALWAYS_INLINE size_t EGridThermalModel::GetFlattenIndex(const ESize3D & index) const
{
    return m_size.x * m_size.y * index.z + m_size.y * index.x + index.y;
}

ECAD_ALWAYS_INLINE ESize3D EGridThermalModel::GetGridIndex(size_t index) const
{
    ESize3D gridIndex;
    size_t tmp = m_size.x * m_size.y;
    gridIndex.z = index / tmp;
    tmp = index % tmp;
    gridIndex.y = tmp % m_size.y;
    gridIndex.x = tmp / m_size.y;
    return gridIndex;
}

ECAD_ALWAYS_INLINE bool EGridThermalModel::isValid(const ESize2D & index) const
{
    return index.x < m_size.x && index.y < m_size.y;
}

ECAD_ALWAYS_INLINE bool EGridThermalModel::isValid(const ESize3D & index) const
{
    return index.x < m_size.x && index.y < m_size.y && index.z < m_stackupLayers.size();
}

}//namespace etherm
}//namesapce emodel
}//namespace ecad