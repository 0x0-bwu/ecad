#pragma once
#include "EThermalModel.h"
namespace ecad {

class IMaterialDef;
namespace model {

using namespace ecad::utils;

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

    void SetThickness(EFloat thickness);
    EFloat GetThickness() const;

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
    EFloat GetMetalFraction(size_t x, size_t y) const;

    ESize2D GetSize() const;

private:
    bool m_isMetal = false;
    EFloat m_thickness = 0;//unit: m
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
    using BlockBC = EPair<std::array<ESize2D, 2>, EThermalBondaryCondition>;
    explicit EGridThermalModel(const ESize2D & size, const FPoint2D & ref = FPoint2D(0, 0), EFloat elevation = 0);
    virtual ~EGridThermalModel();

    EFloat TotalThickness() const;
    FBox2D GetRegion(bool scaled = false) const;
    
    size_t TotalLayers() const;
    size_t TotalGrids() const;
    ESize3D ModelSize() const;
    const ESize2D & GridSize() const;

    bool SetScaleH(EFloat scaleH);
    EFloat GetScaleH() const;

    bool SetResolution(EFloat x, EFloat y);
    void GetResolution(EFloat & x, EFloat & y, bool scaled = false) const;
    std::array<FCoord, 2> GetResolution(bool scaled = false) const;

    size_t AppendLayer(EGridThermalLayer layer);

    void AppendJumpConnection(ESize3D start, ESize3D end, EFloat alpha); //alpha = pi * r * r / L
    const std::vector<std::tuple<ESize3D, ESize3D, EFloat> > & GetJumpConnections() const;

    std::vector<EGridThermalLayer> & GetLayers();
    const std::vector<EGridThermalLayer> & GetLayers() const; 

    bool AddPowerModel(size_t layer, SPtr<EThermalPowerModel> pwrModel);
    const std::vector<SPtr<EThermalPowerModel> > & GetPowerModels(size_t layer) const;

    void AddBlockBC(EOrientation orient, ESize2D ll, ESize2D ur, EThermalBondaryCondition bc);
    const std::vector<BlockBC> & GetBlockBC(EOrientation orient) const;

    size_t GetFlattenIndex(const ESize3D & index) const;
    ESize3D GetGridIndex(size_t index) const;
    bool isValid(const ESize2D & index) const;
    bool isValid(const ESize3D & index) const;

    bool NeedIteration() const;

    EModelType GetModelType() const { return EModelType::ThermalGrid; }

protected:
    ///Copy
    virtual Ptr<EGridThermalModel> CloneImp() const override { return new EGridThermalModel(*this); }

private:
    ESize2D m_size;
    FPoint2D m_ref;
    EFloat m_elevation;
    EFloat m_scaleH = 1.0;//only apply for horizontal
    std::array<FCoord, 2> m_resolution = {0, 0};//unit: m
    std::vector<EGridThermalLayer> m_stackupLayers;
    std::vector<std::tuple<ESize3D, ESize3D, EFloat> > m_jumpConnects;
    std::vector<BlockBC> m_topBlockBCs;
    std::vector<BlockBC> m_botBlockBCs;
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

}//namesapce model
}//namespace ecad