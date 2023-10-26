#include "models/thermal/EGridThermalModel.h"
#include "interfaces/IMaterialDef.h"

#include <boost/math/interpolators/pchip.hpp>
namespace ecad::emodel::etherm {

using namespace eutils;

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

ECAD_INLINE bool EGridDataTable::AddSample(ESimVal key, EGridData data)
{
    if(data.Width() != m_size.x || data.Height() != m_size.y) return false;

    ResetInterpolater();
    m_dataTable.insert(std::make_pair(key, std::move(data)));
    return true;
}

ECAD_INLINE ESimVal EGridDataTable::Query(ESimVal key, size_t x, size_t y, bool * success) const
{
    if(success) *success = true;
    if(x >= m_size.x || y >= m_size.y) {
        if(success) *success = false;
        return 0;
    }
    else if(m_dataTable.empty()) {
        if(success) *success = false;
        return 0;
    }
    else if(m_dataTable.size() == 1) {
        return (m_dataTable.cbegin()->second)(x, y);
    }
    else if(math::LE<ESimVal>(key, m_dataTable.cbegin()->first)) {
        return (m_dataTable.cbegin()->second)(x, y);
    }
    else if(math::GE<ESimVal>(key, m_dataTable.crbegin()->first)) {
        return (m_dataTable.crbegin()->second)(x, y);
    }
    else {
        BuildInterpolater();
        return (*(*m_interpolator)(x, y))(key);
    }
}

ECAD_INLINE std::list<ESimVal> EGridDataTable::GetAllKeys() const
{
    std::list<ESimVal> keys;
    for(const auto & sample : m_dataTable)
        keys.push_back(sample.first);
    return keys;
}

ECAD_INLINE CPtr<EGridData> EGridDataTable::GetTable(ESimVal key) const
{
    auto iter = m_dataTable.find(key);
    if(iter != m_dataTable.cend()) return &(iter->second);
    return nullptr;
}

ECAD_INLINE std::pair<ESimVal, ESimVal> EGridDataTable::GetRange() const
{
    ESimVal min = std::numeric_limits<ESimVal>::max(), max = -min;
    for(const auto & sample : m_dataTable) {
        min = std::min(min, sample.second.MaxOccupancy(std::less<ESimVal>()));
        max = std::max(max, sample.second.MaxOccupancy(std::greater<ESimVal>()));
    }
    return std::make_pair(min, max);
}

ECAD_INLINE bool EGridDataTable::NeedInterpolation() const
{
    if(m_dataTable.size() <= 1) return false;
    
    auto start = m_dataTable.cbegin();
    auto iter = start; iter++;
    for(; iter != m_dataTable.cend(); ++iter) {
        if(start->second != iter->second)
            return true;
    }
    return false;
} 

ECAD_INLINE void EGridDataTable::BuildInterpolater() const
{
    if(m_interpolator) return;
    m_interpolator.reset(new EGridInterpolator(m_size.x, m_size.y, nullptr));

    std::vector<ESimVal> x, y(GetSampleSize());
    x.reserve(GetSampleSize());
    for (const auto & data : m_dataTable)
        x.push_back(data.first);
    
    for (size_t i = 0; i < m_size.x; ++i) {
        for (size_t j = 0; j < m_size.y; ++j) {
            size_t k = 0;
            for (const auto & data : m_dataTable){
                y[k++] = (data.second)(i, j);
            }
            (*m_interpolator)(i, j) = std::make_shared<Interpolator>(std::vector<ESimVal>(x), std::vector<ESimVal>(y));
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
}

ECAD_INLINE EGridThermalLayer::~EGridThermalLayer()
{
}

ECAD_INLINE const std::string & EGridThermalLayer::GetName() const
{
    return m_name;
}

ECAD_INLINE void EGridThermalLayer::SetIsMetal(bool isMetal)
{
    m_isMetal = isMetal;
}
ECAD_INLINE bool EGridThermalLayer::isMetalLayer() const
{
    return m_isMetal;
}

ECAD_INLINE void EGridThermalLayer::SetThickness(FCoord thickness)
{
    m_thickness = thickness;
}

ECAD_INLINE FCoord EGridThermalLayer::GetThickness() const
{
    return m_thickness;
}

ECAD_INLINE void EGridThermalLayer::SetTopLayer(const std::string & name)
{
    m_topLayer = name;
}

ECAD_INLINE const std::string & EGridThermalLayer::GetTopLayer() const
{
    return m_topLayer;
}

ECAD_INLINE void EGridThermalLayer::SetBotLayer(const std::string & name)
{
    m_botLayer = name;
}

ECAD_INLINE const std::string & EGridThermalLayer::GetBotLayer() const
{
    return m_botLayer;
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

ECAD_INLINE bool EGridThermalLayer::SetMetalFraction(SPtr<ELayerMetalFraction> mf)
{
    if(nullptr == mf) return false;
    if(ESize2D(mf->Width(), mf->Height()) != GetSize()) return false;
    m_metalFraction = mf;
    return true;
}

ECAD_INLINE SPtr<ELayerMetalFraction> EGridThermalLayer::GetMetalFraction() const
{
    return m_metalFraction;
}

ECAD_INLINE ESimVal EGridThermalLayer::GetMetalFraction(size_t x, size_t y) const
{
    return (*m_metalFraction)(x, y);
}

ECAD_INLINE ESize2D EGridThermalLayer::GetSize() const
{
    if(nullptr == m_metalFraction) return ESize2D{};
    return ESize2D(m_metalFraction->Width(), m_metalFraction->Height());
}

ECAD_INLINE EGridThermalModel::EGridThermalModel(const ESize2D & size, const FPoint2D & ref, FCoord elevation)
 : m_size(size), m_ref(ref), m_elevation(elevation)
{

}

ECAD_INLINE EGridThermalModel::~EGridThermalModel()
{
}

ECAD_INLINE FCoord EGridThermalModel::TotalThickness() const
{
    FCoord thickness = 0.0;
    for(const auto & layer : m_stackupLayers)
        thickness += layer.GetThickness();
    return thickness;
}

ECAD_INLINE FBox2D EGridThermalModel::GetRegion(bool scaled) const
{
    FPoint2D ur = m_ref;
    for(size_t i = 0; i < 2; ++i)
        ur[i] += scaled ? m_scaleH * m_resolution[i] * m_size[i] : m_resolution[i] * m_size[i];
    return FBox2D(m_ref, ur);
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

ECAD_INLINE bool EGridThermalModel::SetScaleH(FCoord scaleH)
{
    if(math::LT<FCoord>(scaleH, 0)) return false;
    m_scaleH = scaleH;
    return true;
}

ECAD_INLINE FCoord EGridThermalModel::GetScaleH() const
{
    return m_scaleH;
}

ECAD_INLINE bool EGridThermalModel::SetResolution(FCoord x, FCoord y)
{
    if(math::LT<FCoord>(x, 0) || math::LT<FCoord>(y, 0)) return false;
    m_resolution = std::array<FCoord, 2>{x, y};
    return true;
}

ECAD_INLINE void EGridThermalModel::GetResolution(FCoord & x, FCoord & y, bool scaled) const
{
    x = scaled ? m_scaleH * m_resolution[0] : m_resolution[0];
    y = scaled ? m_scaleH * m_resolution[1] : m_resolution[1];
}

ECAD_INLINE std::array<FCoord, 2> EGridThermalModel::GetResolution(bool scaled) const
{
    auto res = m_resolution;
    if(scaled) {
        res[0] *= m_scaleH;
        res[1] *= m_scaleH;
    }
    return res;
}

ECAD_INLINE bool EGridThermalModel::AppendLayer(EGridThermalLayer layer)
{
    if(math::LT<FCoord>(layer.GetThickness(), 0)) return false;
    if(layer.GetSize() != m_size) return false;
    if(!m_stackupLayers.empty()) {
        auto & botLayer = m_stackupLayers.back();
        botLayer.SetBotLayer(layer.GetName());
        layer.SetTopLayer(botLayer.GetName());
    }
    m_stackupLayers.emplace_back(std::move(layer));
    return true;
}

ECAD_INLINE std::vector<EGridThermalLayer> & EGridThermalModel::GetLayers()
{
    return m_stackupLayers;
}

ECAD_INLINE const std::vector<EGridThermalLayer> & EGridThermalModel::GetLayers() const
{
    return m_stackupLayers;
}

ECAD_INLINE bool EGridThermalModel::SetPowerModel(size_t layer, SPtr<EGridPowerModel> pwrModel)
{
    if(layer >= m_stackupLayers.size()) return false;
    return m_stackupLayers.at(layer).SetPowerModel(pwrModel);
}

ECAD_INLINE CPtr<EGridPowerModel> EGridThermalModel::GetPowerModel(size_t layer) const
{
    if(layer >= m_stackupLayers.size()) return nullptr;
    return m_stackupLayers.at(layer).GetPowerModel();
}

ECAD_INLINE bool EGridThermalModel::SetTopBotBCModel(SPtr<EGridBCModel> top, SPtr<EGridBCModel> bot)
{
    if(top && top->GetTableSize() != m_size) return false;
    if(bot && bot->GetTableSize() != m_size) return false;
    m_bcTopBot[0] = top; m_bcTopBot[1] = bot;
    return true;
}

ECAD_INLINE void EGridThermalModel::GetTopBotBCModel(SPtr<EGridBCModel> & top, SPtr<EGridBCModel> & bot) const
{
    top = m_bcTopBot[0];
    bot = m_bcTopBot[1];
}

ECAD_INLINE void EGridThermalModel::SetTopBotBCType(BCType top, BCType bot)
{
    m_bcTypeTopBot[0] = top;
    m_bcTypeTopBot[1] = bot;
}

ECAD_INLINE void EGridThermalModel::GetTopBotBCType(BCType & top, BCType & bot) const
{
    top = m_bcTypeTopBot[0];
    bot = m_bcTypeTopBot[1];
}

ECAD_INLINE bool EGridThermalModel::NeedIteration() const
{
    for(auto bc : m_bcTopBot)
        if(bc && bc->NeedInterpolation()) return true;

    for(const auto & layer : m_stackupLayers) {
        auto pwr = layer.GetPowerModel();
        if(pwr && pwr->NeedInterpolation()) return true;
    }
    return false;
}

} //namespace ecad::emodel::etherm