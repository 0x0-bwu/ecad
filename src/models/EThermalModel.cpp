#ifndef ECAD_HEADER_ONLY
#include "models/EThermalModel.h"
#endif

#include "interfaces/IMaterialDef.h"
namespace ecad {
namespace emodel {

ECAD_INLINE EGridDataTable::EGridDataTable(const ESize2D & size)
 : m_size(size)
{
}

ECAD_INLINE EGridDataTable::~EGridDataTable()
{
}

ECAD_INLINE size_t EGridDataTable::GetSampleSize() const
{
    return m_dataTable.size();
}

ECAD_INLINE const ESize2D & EGridDataTable::GetTableSize() const
{
    return m_size;
}

ECAD_INLINE bool EGridDataTable::AddSample(EGridDataNumType key, EGridData data)
{
    if(data.Width() != m_size.x || data.Height() != m_size.y) return false;

    ResetInterpolater();
    m_dataTable.insert(std::make_pair(key, std::move(data)));
    return true;
}

ECAD_INLINE EGridDataNumType EGridDataTable::Query(EGridDataNumType key, size_t x, size_t y, bool * success) const
{
    if(success) *success = true;
    if(x >= m_size.x || y >= m_size.y) {
        if(success) *success = false;
        return EGridDataNumType(0);
    }
    else if(m_dataTable.empty()) {
        if(success) *success = false;
        return EGridDataNumType(0);
    }
    else if(m_dataTable.size() == 1) {
        return (m_dataTable.cbegin()->second)(x, y);
    }
    else {
        BuildInterpolater();
        return ((*m_interpolator)(x, y))(key);
    }
}

ECAD_INLINE void EGridDataTable::BuildInterpolater() const
{
    if(m_interpolator) return;
    m_interpolator.reset(new EGridInterpolator(m_size.x, m_size.y));

    std::vector<EGridDataNumType> x, y;
    x.reserve(GetSampleSize());
    for(const auto & data : m_dataTable)
        x.push_back(data.first);
    
    y.resize(GetSampleSize());
    for(size_t i = 0; i < m_size.x; ++i) {
        for(size_t j = 0; j < m_size.y; ++j) {
            size_t k = 0;
            for(const auto & data : m_dataTable){
                y[k++] = (data.second)(i, j);
            }
            ((*m_interpolator)(i, j)).SetSamples(x, y);
        }
    }
}

ECAD_INLINE void EGridDataTable::ResetInterpolater()
{
    m_interpolator.reset(nullptr);
}

ECAD_INLINE EGridThermalLayer::EGridThermalLayer(std::string name, SPtr<ELayerMetalFraction> metalFraction)
 : m_name(std::move(name)), m_metalFraction(metalFraction)
{
    GENERIC_ASSERT(m_metalFraction != nullptr)
}

ECAD_INLINE EGridThermalLayer::~EGridThermalLayer()
{
}

ECAD_INLINE void EGridThermalLayer::SetThickness(FCoord thickness)
{
    m_thickness = thickness;
}

ECAD_INLINE FCoord EGridThermalLayer::GetThickness() const
{
    return m_thickness;
}

ECAD_INLINE void EGridThermalLayer::SetConductingMaterial(CPtr<IMaterialDef> material)
{
    m_conductingMat = material;
}

ECAD_INLINE CPtr<IMaterialDef> EGridThermalLayer::GetConductingMaterial() const
{
    return m_conductingMat;
}

ECAD_INLINE void EGridThermalLayer::SetDielectricMaterial(CPtr<IMaterialDef> material)
{
    m_dielectricMat = material;
}

ECAD_INLINE CPtr<IMaterialDef> EGridThermalLayer::GetDielectricMaterial() const
{
    return m_dielectricMat;
}

ECAD_INLINE bool EGridThermalLayer::SetPowerModel(SPtr<EGridPowerModel> pwrModel)
{
    if(nullptr == pwrModel) return false;
    if(GetSize() != pwrModel->GetTableSize()) return false;
    m_powerModel = pwrModel;
    return true;
}

ECAD_INLINE CPtr<EGridPowerModel> EGridThermalLayer::GetPowerModel() const
{
    if(nullptr == m_powerModel) return nullptr;
    return m_powerModel.get();
}

ECAD_INLINE ESize2D EGridThermalLayer::GetSize() const
{
    auto w = m_metalFraction->Width();
    auto h = m_metalFraction->Height();
    return ESize2D{w, h};
}

ECAD_INLINE EThermalModel::~EThermalModel()
{
}

ECAD_INLINE EGridThermalModel::EGridThermalModel(const ESize2D & size, const FPoint2D & ref, FCoord elevation)
 : m_size(size), m_ref(ref), m_elevation(elevation)
{

}

ECAD_INLINE EGridThermalModel::~EGridThermalModel()
{
}

ECAD_INLINE size_t EGridThermalModel::TotalLayers() const
{
    return m_stackupLayers.size();
}

ECAD_INLINE size_t EGridThermalModel::TotalGrids() const
{
    return m_size.x * m_size.y * TotalLayers();
}

ECAD_INLINE ESize3D EGridThermalModel::ModelSize() const
{
    return ESize3D(m_size.x, m_size.y, TotalLayers());
}

ECAD_INLINE const ESize2D & EGridThermalModel::GridSize() const
{
    return m_size;
}

ECAD_INLINE bool EGridThermalModel::SetScale(FCoord scale)
{
    if(math::LT<FCoord>(scale, 0)) return false;
    m_scale = scale;
    return true;
}

ECAD_INLINE FCoord EGridThermalModel::GetSCale() const
{
    return m_scale;
}

ECAD_INLINE bool EGridThermalModel::SetResolution(FCoord x, FCoord y)
{
    if(math::LT<FCoord>(x, 0) || math::LT<FCoord>(y, 0)) return false;
    m_resolution = std::array<FCoord, 2>{x, y};
    return true;
}

ECAD_INLINE void EGridThermalModel::GetResolution(FCoord & x, FCoord & y) const
{
    x = m_resolution[0];
    y = m_resolution[1];
}

ECAD_INLINE bool EGridThermalModel::AppendLayer(EGridThermalLayer layer)
{
    if(math::LT<FCoord>(layer.GetThickness(), 0)) return false;
    if(layer.GetSize() != m_size) return false;
    m_stackupLayers.emplace_back(std::move(layer));
    return true;
}

ECAD_INLINE const std::vector<EGridThermalLayer> & EGridThermalModel::GetLayers() const
{
    return m_stackupLayers;
}

ECAD_INLINE bool EGridThermalModel::SetTopBotBCModel(SPtr<EGridBCModel> top, SPtr<EGridBCModel> bot)
{
    if(top && top->GetTableSize() != m_size) return false;
    if(bot && bot->GetTableSize() != m_size) return false;
    m_bcTopBot[0] = top; m_bcTopBot[1] = bot;
    return true;
}

ECAD_INLINE void EGridThermalModel::SetTopBotBCType(BCType top, BCType bot)
{
    m_bcTypeTopBot[0] = top;
    m_bcTypeTopBot[1] = bot;
}

}//namespace emodel
}//namespace ecad
