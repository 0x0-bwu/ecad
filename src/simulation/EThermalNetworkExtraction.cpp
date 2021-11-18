#ifndef ECAD_HEADER_ONLY
#include "EThermalNetworkExtraction.h"
#endif

#include "thermal/solver/ThermalNetworkSolver.hpp"
#include "simulation/EMetalFractionMapping.h"
#include "generic/tools/FileSystem.hpp"
#include "Interface.h"
namespace ecad {
namespace esim {

ECAD_INLINE bool EThermalNetworkExtraction::GenerateThermalNetwork(CPtr<ILayoutView> layout)
{
    std::string currentPath = generic::filesystem::CurrentPath();

    esim::EMetalFractionMappingSettings settings;
    settings.threads = 16;
    settings.resolution = 200;//um
    settings.regionExtTop = 470;//um
    settings.regionExtBot = 470;//um
    settings.regionExtLeft = 470;//um
    settings.regionExtRight = 470;//um
    settings.coordUnits = layout->GetCell()->GetDatabase()->GetCoordUnits();
    settings.outFile = currentPath + "/test/ecad/testdata/simulation/result.mf";

    LayoutMetalFractionMapper mapper(settings);
    if(!mapper.GenerateMetalFractionMapping(layout)) return false;

    auto mf = mapper.GetLayoutMetalFraction();
    auto mfInfo = mapper.GetMetalFractionInfo();
    if(nullptr == mf || nullptr == mfInfo) return false;

    //wbtest
    std::string densityFile = currentPath + "/test/ecad/testdata/simulation/density.txt";
    WriteThermalProfile(*mfInfo, *mf, densityFile);

    //wbtest
    std::vector<double> thickness(7);
    thickness[0] = 0.00003048 * 1e9;
    thickness[1] = 0.00020320 * 1e9;
    thickness[2] = 0.00003048 * 1e9;
    thickness[3] = 0.00020320 * 1e9;
    thickness[4] = 0.00003048 * 1e9;
    thickness[5] = 0.00020320 * 1e9;
    thickness[6] = 0.00003048 * 1e9;

    auto [nx, ny] = mfInfo->tiles;
    std::cout << "x: " << nx << ", y: " << ny << std::endl;//wbtest
    size_t nz = (mfInfo->layers).size();

    m_modelSize = {nx + 1, ny + 1, nz + 1};
    size_t size = m_modelSize.x * m_modelSize.y * m_modelSize.z;
    m_network.reset(new ThermalNetwork(size));

    double kCu = 400;//W/k.m
    double kFr4 = 0.294;//W/k.m
    auto equivK = [&kCu, &kFr4](double mf)
    {
        if(mf < 0.0) mf = 0.0;
        if(mf > 1.0) mf = 1.0;
        return mf * kCu + (1.0 - mf) * kFr4;
    };
    auto blockF = [&mf, &nz](const ModelIndex & mfIndex) { return (*mf)(mfIndex.x, mfIndex.y)[nz - mfIndex.z - 1]; };

    double k = 0;
    double area = 0;
    size_t count = 0;
    double scale = settings.coordUnits.Scale2Unit();
    double xyLen = settings.resolution * 1e-6;//um2m, wbtest
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
                auto zLen = thickness.at(nz - modelIndex.z - 1) * scale * 1e-6;//um2m,wbtest
                area += 0.25 * zLen * xyLen;
                count++;
            }
            if(mfIdx4.isValid()){
                k += equivK(blockF(mfIdx4));
                auto zLen = thickness.at(nz - modelIndex.z) * scale * 1e-6;//um2m,wbtest
                area += 0.25 * zLen * xyLen;
                count++;
            }
            if(mfIdx5.isValid()){
                k += equivK(blockF(mfIdx5));
                auto zLen = thickness.at(nz - modelIndex.z - 1) * scale * 1e-6;//um2m,wbtest
                area += 0.25 * zLen * xyLen;
                count++;
            }
            if(mfIdx8.isValid()){
                k += equivK(blockF(mfIdx8));
                auto zLen = thickness.at(nz - modelIndex.z) * scale * 1e-6;//um2m,wbtest
                area += 0.25 * zLen * xyLen;
                count++;
            }
            if(count != 0){
                k = k / count * xyLen / area;
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
                auto zLen = thickness.at(nz - modelIndex.z - 1) * scale * 1e-6;//um2m,wbtest
                area += 0.25 * zLen * xyLen;
                count++;
            }
            if(mfIdx2.isValid()){
                k += equivK(blockF(mfIdx2));
                auto zLen = thickness.at(nz - modelIndex.z - 1) * scale * 1e-6;//um2m,wbtest
                area += 0.25 * zLen * xyLen;
                count++;
            }
            if(mfIdx3.isValid()){
                k += equivK(blockF(mfIdx3));
                auto zLen = thickness.at(nz - modelIndex.z) * scale * 1e-6;//um2m,wbtest
                area += 0.25 * zLen * xyLen;
                count++;
            }
            if(mfIdx4.isValid()){
                k += equivK(blockF(mfIdx4));
                auto zLen = thickness.at(nz - modelIndex.z) * scale * 1e-6;//um2m,wbtest
                area += 0.25 * zLen * xyLen;
                count++;
            }
            if(count != 0){
                k = k / count * xyLen / area;
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
                area += 0.25 * xyLen * xyLen;
                count++;
            }
            if(mfIdx2.isValid()){
                k += equivK(blockF(mfIdx2));
                area += 0.25 * xyLen * xyLen;
                count++;
            }
            if(mfIdx6.isValid()){
                k += equivK(blockF(mfIdx6));
                area += 0.25 * xyLen * xyLen;
                count++;
            }
            if(mfIdx5.isValid()){
                k += equivK(blockF(mfIdx5));
                area += 0.25 * xyLen * xyLen;
                count++;
            }
            if(count != 0){
                auto zLen = thickness.at(nz - modelIndex.z - 1) * scale * 1e-6;//um2m,wbtest
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
    double aveHF = aveP / (xyLen * xyLen);
    for(size_t i = sx; i <= ex; ++i){
        for(size_t j = sy; j <= ey; ++j){
            ModelIndex modelIndex{i, j, m_modelSize.z - 1};
            size_t index = GetFlattenIndex(modelIndex);
            m_network->SetHF(index, aveHF);
        }
    }

    ECAD_EFFICIENCY_TRACK("thermal network solve");
    thermal::solver::ThermalNetworkSolver solver(*m_network);
    auto results = solver.Solve(20);

    auto htMap = std::unique_ptr<LayoutMetalFraction>(new LayoutMetalFraction(m_modelSize.x, m_modelSize.y));
    for(size_t i = 0; i < m_modelSize.x; ++i){
        for(size_t j = 0; j < m_modelSize.y; ++j){
            (*htMap)(i, j).resize(m_modelSize.z);
        }
    }

    auto min = *std::min_element(results.begin(), results.end());
    auto max = *std::max_element(results.begin(), results.end());
    std::cout << "min: " << min << ", max: " << max << std::endl;
    auto delta = max - min;
    for(size_t i = 0; i < m_network->Size(); ++i){
        auto modelIndex = GetModelIndex(i);
        (*htMap)(modelIndex.x, modelIndex.y)[m_modelSize.z - modelIndex.z - 1] = results[i];
        //(*htMap)(modelIndex.x, modelIndex.y)[modelIndex.z] = (results[i] - min) / delta;//to 0~1
    }

    auto htMapInfo = *mfInfo;
    htMapInfo.tiles.first += 1;
    htMapInfo.tiles.second += 1;
    std::string resultFile = currentPath + "/test/ecad/testdata/simulation/temperature.txt";
    WriteThermalProfile(htMapInfo, *htMap, resultFile);

#if defined(ECAD_DEBUG_MODE) && defined(BOOST_GIL_IO_PNG_SUPPORT)

    size_t index = 0;
    auto rgbaFunc = [&index](const std::vector<float> & d) {
        int r, g, b, a = 255;
        generic::color::RGBFromScalar(d[index], r, g, b);
        return std::make_tuple(r, g, b, a);
    };

    for(index = 0; index < mfInfo->layers.size(); ++index){
        std::string filepng = currentPath + "/test/ecad/testdata/simulation/heatmap/" + std::to_string(index) + ".png";
        htMap->WriteImgProfile(filepng, rgbaFunc);
    }
#endif
    return true;
}

}//namespace esim
}//namespace ecad