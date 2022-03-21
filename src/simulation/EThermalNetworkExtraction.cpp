#ifndef ECAD_HEADER_ONLY
#include "EThermalNetworkExtraction.h"
#endif

#include "thermal/solver/ThermalNetworkSolver.hpp"
#include "simulation/EMetalFractionMapping.h"
#include "generic/tools/FileSystem.hpp"
#include "Interface.h"
namespace ecad {
namespace esim {

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
    m_network.reset(new ThermalNetwork(size));

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
    thermal::solver::ThermalNetworkSolver solver(*m_network);
    auto results = solver.Solve(20);

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

    if(!m_settings.outDir.empty() && m_settings.dumpHotmaps) {
        auto min = *std::min_element(results.begin(), results.end());
        auto max = *std::max_element(results.begin(), results.end());
        auto delta = max - min;
        for(size_t i = 0; i < m_network->Size(); ++i){
            auto modelIndex = GetModelIndex(i);
            auto lyrHtMap = htMap->at(modelIndex.z);
            (*lyrHtMap)(modelIndex.x, modelIndex.y) = (results[i] - min) / delta;//to 0~1
        }

        auto rgbaFunc = [](float d) {
            int r, g, b, a = 255;
            generic::color::RGBFromScalar(d, r, g, b);
            return std::make_tuple(r, g, b, a);
        };
        
        size_t index = 0;
        for(index = 0; index < mfInfo->layers.size(); ++index){
            std::string filepng = m_settings.outDir + GENERIC_FOLDER_SEPS + std::to_string(index) + ".png";
            htMap->at(index)->WriteImgProfile(filepng, rgbaFunc);
        }
    }
#endif//BOOST_GIL_IO_PNG_SUPPORT
    return true;
}

}//namespace esim
}//namespace ecad