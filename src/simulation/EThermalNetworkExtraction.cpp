#ifndef ECAD_HEADER_ONLY
#include "EThermalNetworkExtraction.h"
#endif

#include "thermal/solver/ThermalNetworkSolver.hpp"
#include "utilities/EMetalFractionMapping.h"
#include "generic/tools/FileSystem.hpp"
#include "Interface.h"
namespace ecad {
namespace esim {

using namespace emodel;
using namespace eutils;

ECAD_INLINE EGridThermalNetworkBuilder::EGridThermalNetworkBuilder(const EGridThermalModel & model)
 : m_model(model), m_size(model.ModelSize())
{
}

ECAD_INLINE EGridThermalNetworkBuilder::~EGridThermalNetworkBuilder()
{
}

ECAD_INLINE UPtr<ThermalNetwork<EThermalNetworkNumType> > EGridThermalNetworkBuilder::Build(const std::vector<float_t> & iniT)
{
    const size_t size = m_model.TotalGrids(); 
    if(iniT.size() != size) return nullptr;

    auto network = std::make_unique<ThermalNetwork<float_t> >(size);

    //power
    bool success;
    const auto & layers = m_model.GetLayers();
    for(size_t z = 0; z < layers.size(); ++z) {
        const auto & layer = layers.at(z);
        auto pwrModel = layer.GetPowerModel();
        if(nullptr == pwrModel) continue;
        for(size_t x = 0; x < m_size.x; ++x) {
            for(size_t y = 0; y < m_size.y; ++y) {
                ESize3D gridIndex {x, y, z};
                auto index = GetFlattenIndex(gridIndex);
                auto pwr = pwrModel->Query(iniT.at(index), x, y, &success);
                if(success) network->SetHF(index, pwr);
            }
        }
    }

    //r
    for(size_t index = 0; index < size; ++index) {
        auto gridIndex = GetGridIndex(index);
        auto topNb = GetNeighbor(gridIndex, Orientation::Top);
        if(isValid(topNb)){
            auto topIndex = GetFlattenIndex(topNb);

        }
    }
    return nullptr;//wbtest
}

ECAD_INLINE std::array<EThermalNetworkNumType, 3> EGridThermalNetworkBuilder::GetConductingMatK(size_t layer, float_t refT) const
{
    //todo
    return std::array<float_t, 3>{400, 400, 400};
}

ECAD_INLINE std::array<EThermalNetworkNumType, 3> EGridThermalNetworkBuilder::GetDielectircMatK(size_t layer, float_t refT) const
{
    //todo
    return std::array<float_t, 3>{0.294, 0.294, 0.294};
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
            else index.z += 1;
            break;
        }
        case Orientation::Bot : {
            if(index.z == (m_size.z - 1))
                index.z = invalidIndex;
            else index.z -= 1;
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
        case Orientation::Back : {
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

ECAD_INLINE void EThermalNetworkExtraction::SetExtractionSettings(EThermalNetworkExtractionSettings settings)
{
    m_settings = std::move(settings);
}

ECAD_INLINE bool EThermalNetworkExtraction::GenerateThermalNetwork(Ptr<ILayoutView> layout)
{
    ECAD_EFFICIENCY_TRACK("generate thermal network")
    std::string currentPath = generic::filesystem::CurrentPath();

    EMetalFractionMappingSettings settings;
    settings.threads = m_settings.threads;
    settings.grid = m_settings.grid;
    settings.regionExtTop = m_settings.regionExtTop;
    settings.regionExtBot = m_settings.regionExtBot;
    settings.regionExtLeft  = m_settings.regionExtLeft;
    settings.regionExtRight = m_settings.regionExtRight;
    settings.mergeGeomBeforeMapping = m_settings.mergeGeomBeforeMetalMapping;

    ELayoutMetalFractionMapper mapper(settings);
    if(!mapper.GenerateMetalFractionMapping(layout)) return false;

    auto mf = mapper.GetLayoutMetalFraction();
    auto mfInfo = mapper.GetMetalFractionInfo();
    if(nullptr == mf || nullptr == mfInfo) return false;

    if(!m_settings.outDir.empty() && m_settings.dumpDensityFile) {
        auto densityFile = m_settings.outDir + GENERIC_FOLDER_SEPS + "density.txt";
        WriteThermalProfile(*mfInfo, *mf, densityFile);
    }

    std::vector<Ptr<ILayer> > layers;
    layout->GetStackupLayers(layers);

    std::vector<FCoord> thickness;
    for(auto layer : layers) {
        auto stackupLayer = layer->GetStackupLayerFromLayer();
        thickness.push_back(stackupLayer->GetThickness());
    }

    auto [nx, ny] = mfInfo->grid;
    size_t nz = (mfInfo->layers).size();

    m_modelSize = {nx + 1, ny + 1, nz + 1};
    size_t size = m_modelSize.x * m_modelSize.y * m_modelSize.z;
    m_network.reset(new ThermalNetwork<float_t>(size));

    double kCu = 400;//W/k.m
    double kFr4 = 0.294;//W/k.m
    auto equivK = [&kCu, &kFr4](double composite)
    {
        if(composite < 0.0) composite = 0.0;
        if(composite > 1.0) composite = 1.0;
        return composite * kCu + (1.0 - composite) * kFr4;
    };
    auto blockF = [&mf, &nz](const ModelIndex & mfIndex) { return (*(mf->at(nz - mfIndex.z - 1)))(mfIndex.x, mfIndex.y); };

    double k = 0;
    double area = 0;
    size_t count = 0;
    auto m = generic::unit::Length::Meter;
    auto coordUnits = layout->GetCoordUnits();
    double xLen = coordUnits.toUnit(mfInfo->stride[0], m);
    double yLen = coordUnits.toUnit(mfInfo->stride[1], m);
    for(size_t i = 0; i < size; ++i){

        auto modelIndex = GetModelIndex(i);
        auto mfIdx1 = GetMetalFractionBlockIndex(i, Quadrant::I);
        auto mfIdx2 = GetMetalFractionBlockIndex(i, Quadrant::II);
        auto mfIdx3 = GetMetalFractionBlockIndex(i, Quadrant::III);
        auto mfIdx4 = GetMetalFractionBlockIndex(i, Quadrant::IV);
        auto mfIdx5 = GetMetalFractionBlockIndex(i, Quadrant::V);
        auto mfIdx6 = GetMetalFractionBlockIndex(i, Quadrant::VI);
        auto mfIdx7 = GetMetalFractionBlockIndex(i, Quadrant::VII);
        auto mfIdx8 = GetMetalFractionBlockIndex(i, Quadrant::VIII);

        //x
        k = 0;
        area = 0;
        count = 0;
        auto xNb = GetNeighbor(i, Axis::PX);
        if(invalidIndex != xNb){
            if(mfIdx1.isValid()){
                k += equivK(blockF(mfIdx1));
                auto zLen = coordUnits.toUnit(thickness.at(nz - modelIndex.z - 1), m);
                area += 0.25 * zLen * yLen;
                count++;
            }
            if(mfIdx4.isValid()){
                k += equivK(blockF(mfIdx4));
                auto zLen = coordUnits.toUnit(thickness.at(nz - modelIndex.z), m);
                area += 0.25 * zLen * yLen;
                count++;
            }
            if(mfIdx5.isValid()){
                k += equivK(blockF(mfIdx5));
                auto zLen = coordUnits.toUnit(thickness.at(nz - modelIndex.z - 1), m);
                area += 0.25 * zLen * yLen;
                count++;
            }
            if(mfIdx8.isValid()){
                k += equivK(blockF(mfIdx8));
                auto zLen = coordUnits.toUnit(thickness.at(nz - modelIndex.z), m);
                area += 0.25 * zLen * yLen;
                count++;
            }
            if(count != 0){
                k = k / count * xLen / area;
                m_network->SetR(i, xNb, k);
            }
        }

        //y
        k = 0;
        area = 0;
        count = 0;
        auto yNb = GetNeighbor(i, Axis::PY);
        if(invalidIndex != yNb){
            if(mfIdx1.isValid()){
                k += equivK(blockF(mfIdx1));
                auto zLen = coordUnits.toUnit(thickness.at(nz - modelIndex.z - 1), m);
                area += 0.25 * zLen * xLen;
                count++;
            }
            if(mfIdx2.isValid()){
                k += equivK(blockF(mfIdx2));
                auto zLen = coordUnits.toUnit(thickness.at(nz - modelIndex.z - 1), m);
                area += 0.25 * zLen * xLen;
                count++;
            }
            if(mfIdx3.isValid()){
                k += equivK(blockF(mfIdx3));
                auto zLen = coordUnits.toUnit(thickness.at(nz - modelIndex.z), m);
                area += 0.25 * zLen * xLen;
                count++;
            }
            if(mfIdx4.isValid()){
                k += equivK(blockF(mfIdx4));
                auto zLen = coordUnits.toUnit(thickness.at(nz - modelIndex.z), m);
                area += 0.25 * zLen * xLen;
                count++;
            }
            if(count != 0){
                k = k / count * yLen / area;
                m_network->SetR(i, yNb, k);
            }
        }

        //z
        k = 0;
        area = 0;
        count = 0;
        auto zNb = GetNeighbor(i, Axis::PZ);
        if(invalidIndex != zNb){
            if(mfIdx1.isValid()){
                k += equivK(blockF(mfIdx1));
                area += 0.25 * xLen * yLen;
                count++;
            }
            if(mfIdx2.isValid()){
                k += equivK(blockF(mfIdx2));
                area += 0.25 * xLen * yLen;
                count++;
            }
            if(mfIdx6.isValid()){
                k += equivK(blockF(mfIdx6));
                area += 0.25 * xLen * yLen;
                count++;
            }
            if(mfIdx5.isValid()){
                k += equivK(blockF(mfIdx5));
                area += 0.25 * xLen * yLen;
                count++;
            }
            if(count != 0){
                auto zLen = coordUnits.toUnit(thickness.at(nz - modelIndex.z - 1), m);
                k = k / count * zLen / area;
                m_network->SetR(i, zNb, k);
            }
        }
    }

    //htc
    double htc = 5;//w/k.m.m
    for(size_t i = 0; i < size; ++i){
        if(isBoundaryNode(i))
            m_network->SetHTC(i, htc * m_network->NodeFreedom(i));
    }

    //heat flux, wbtest
    double totalP = 0.1;//W
    size_t sx = 0.3 * m_modelSize.x, ex = 0.7 * m_modelSize.x;
    size_t sy = 0.3 * m_modelSize.y, ey = 0.7 * m_modelSize.y;
    size_t total = (ex - sx + 1) * (ey - sy + 1);
    double aveP = totalP / total;
    double aveHF = aveP / (xLen * yLen);
    for(size_t i = sx; i <= ex; ++i){
        for(size_t j = sy; j <= ey; ++j){
            ModelIndex modelIndex{i, j, m_modelSize.z - 1};
            size_t index = GetFlattenIndex(modelIndex);
            m_network->SetHF(index, aveHF);
        }
    }

    ECAD_EFFICIENCY_TRACK("thermal network solve")
    thermal::solver::ThermalNetworkSolver<float_t> solver(*m_network);
    auto results = solver.Solve(20.0);

    auto htMap = std::unique_ptr<ELayoutMetalFraction>(new ELayoutMetalFraction);
    for(size_t i = 0; i < m_modelSize.z; ++i)
        htMap->push_back(std::make_shared<ELayerMetalFraction>(m_modelSize.x, m_modelSize.y));

    for(size_t i = 0; i < m_network->Size(); ++i){
        auto modelIndex = GetModelIndex(i);
        auto lyrHtMap = htMap->at(m_modelSize.z - modelIndex.z - 1);
        (*lyrHtMap)(modelIndex.x, modelIndex.y) = results[i];
    }

    auto htMapInfo = *mfInfo;
    htMapInfo.grid[0] += 1;
    htMapInfo.grid[1] += 1;
    if(!m_settings.outDir.empty() && m_settings.dumpTemperatureFile) {
        auto tFile = m_settings.outDir + GENERIC_FOLDER_SEPS + "temperature.txt";
        WriteThermalProfile(htMapInfo, *htMap, tFile);
    }

#ifdef BOOST_GIL_IO_PNG_SUPPORT

    using ValueType = typename ELayerMetalFraction::ResultType;
    if(!m_settings.outDir.empty() && m_settings.dumpHotmaps) {        
        for(auto index = 0; index < mfInfo->layers.size(); ++index){
            auto lyr = htMap->at(index);
            auto min = lyr->MaxOccupancy(std::less<ValueType>());
            auto max = lyr->MaxOccupancy(std::greater<ValueType>());
            auto range = max - min;
            auto rgbaFunc = [&min, &range](ValueType d) {
                int r, g, b, a = 255;
                generic::color::RGBFromScalar((d - min) / range, r, g, b);
                return std::make_tuple(r, g, b, a);
            };
            std::cout << "layer: " << index + 1 << ", min: " << min << ", max: " << max << std::endl;   
            std::string filepng = m_settings.outDir + GENERIC_FOLDER_SEPS + std::to_string(index) + ".png";
            lyr->WriteImgProfile(filepng, rgbaFunc);
        }
    }
#endif//BOOST_GIL_IO_PNG_SUPPORT
    return true;
}

}//namespace esim
}//namespace ecad