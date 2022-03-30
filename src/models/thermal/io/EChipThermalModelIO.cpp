#ifndef ECAD_HEADER_ONLY
#include "models/thermal/io/EChipThermalModelIO.h"
#endif

#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"
namespace ecad {
namespace emodel {
namespace etherm {
namespace io {
namespace ctm {

using namespace generic::str;
using namespace generic::format;
using namespace generic::filesystem;

ECAD_API UPtr<EChipThermalModelV1> makeChipThermalModelFromCTMv1File(const std::string & filename, std::string * err)
{
    using namespace detail;
    auto dir = UntarCTMv1File(filename, err);
    if(dir.empty()) {
        if(err) *err = Format2String("Error: failed to unarchive the file %1%", filename);
        return nullptr;
    }

    auto model = std::make_unique<EChipThermalModelV1>();

    //header
    auto headerFile = dir + GENERIC_FOLDER_SEPS + "CTM_header.txt";
    if(!ParseCTMv1HeaderFile(headerFile, model->header, err)) return nullptr;
    
    //power
    auto tiles = model->header.tiles;
    model->powers = std::make_shared<EGridPowerModel>(tiles);
    for(size_t i = 0; i < model->header.temperatures.size(); ++i) {
        auto temperature = model->header.temperatures.at(i);
        auto powerFile = dir + GENERIC_FOLDER_SEPS + Format2String("power_T[%1%].ctm", i + 1);
        EGridData powers(tiles.x, tiles.y);
        if(!ParseCTMv1PowerFile(powerFile, powers, err)) return nullptr;
        model->powers->AddSample(temperature, std::move(powers));
    }

    //density
    std::vector<SPtr<EGridData> > density;
    for(size_t i = 0; i < model->header.layers.size(); ++i)
        density.push_back(std::make_shared<EGridData>(tiles.x, tiles.y));
    auto densityFile = dir + GENERIC_FOLDER_SEPS + "metal_density.ctm";
    if(!ParseCTMv1DensityFile(densityFile, tiles.x * tiles.y, density, err)) return nullptr;
    for(size_t i = 0; i < model->header.layers.size(); ++i)
        model->densities.insert(std::make_pair(model->header.layers[i].name, density[i]));

    return model;
}

#ifdef BOOST_GIL_IO_PNG_SUPPORT
ECAD_INLINE bool GenerateCTMv1ImageProfiles(const EChipThermalModelV1 & model, const std::string & dirName, std::string * err)
{
    using namespace detail;
    if(!PathExists(dirName)) MakeDir(dirName);

    //power
    if(model.powers) {
        size_t size = std::min(model.header.temperatures.size(), model.powers->GetSampleSize());
        for(size_t i = 0; i < size; ++i) {
            auto t = model.header.temperatures.at(i);
            auto table = model.powers->GetTable(t);
            if(nullptr == table) continue;
            std::string filename = dirName + GENERIC_FOLDER_SEPS + "power_" + std::to_string(t) + "c.png";
            if(!GenerateImageProfile(filename, *table)) continue;
        }
    }
    //density
    for(const auto & density : model.densities) {
        if(nullptr == density.second) continue;
        std::string filename = dirName + GENERIC_FOLDER_SEPS + "layer_" + density.first + ".png";
        if(!GenerateImageProfile(filename, *(density.second))) continue;
    }
    return true;
}
#endif//BOOST_GIL_IO_PNG_SUPPORT

namespace detail {
ECAD_INLINE std::string UntarCTMv1File(const std::string & filename, std::string * err)
{
    if(!FileExists(filename)) {
        if(err) *err = Format2String("Error: file %1% not exists!", filename);
        return std::string{};
    }

    auto dir = DirName(filename);
    if(!isDirWritable(dir)) dir = "/tmp";
    auto baseName = BaseName(filename);
    
    //unzip ctm file
    auto untarDir = dir + GENERIC_FOLDER_SEPS + baseName;
    if(PathExists(untarDir))
        RemoveDir(untarDir);
    MakeDir(untarDir);

    std::string cmd = "tar --overwrite -zxf" + filename + " -C " + untarDir;
    int res = std::system(cmd.c_str());
    return res == 0 ? untarDir : std::string{};
}

ECAD_INLINE bool ParseCTMv1HeaderFile(const std::string & filename, ECTMv1Header & header, std::string * err)
{
    std::ifstream fp(filename.c_str(), std::ios::in);
    if(!fp.good()) {
        if(err) *err = Format2String("Error: failed to open file %1%!", filename);
        return false;
    }

    auto decrypt = [err](const std::string & s, size_t len) {
        auto tmp = s;
        std::for_each(tmp.begin(), tmp.end(), [&](char & c){ c -= 3 * len + 22; });
        double value = std::stod(tmp);
        bool ok = math::GE(value, 0.0);
        if(!ok && err) *err += Format2String("\nWarning: decrypting for %1% failed, 0.0 will be used instead.", s);
        return ok ? value : 0.0;
    };

    size_t count = 0;
    std::string line;
    std::vector<std::string> items;
    const size_t maxTileNum = 1e5;
    while(!fp.eof()) {
        count++;
        line.clear();
        std::getline(fp, line);
        if(line.empty()) continue;

        //#Version
        if(StartsWith(line, "#Version")) {
            header.head = line;
            if(Contains<CaseInsensitive>(header.head, "Encrypted"))
                header.encrypted = true;
        }
        //LAYER
        else if(StartsWith(line, "LAYER")) {
            Split(line, items);
            if(items.size() < 2) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            size_t lyrNum = std::stol(items[1]);
            if(items.size() != (lyrNum * 2 + 2)) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            for(size_t i = 0; i < lyrNum; ++i) {
                ECTMv1Layer layer;
                layer.name = std::move(items[i * 2 + 2]);
                layer.thickness = header.encrypted ? decrypt(items[i * 2 + 3], layer.name.size()) : std::stod(items[i * 2 + 3]);
                header.layers.emplace_back(std::move(layer));
            }
        }
        //DIE and DIE_ORIG
        else if(StartsWith(line, "DIE")) {
            Split(line, items);
            if(items.size() != 5) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            FBox2D box;
            for(size_t i = 0; i < 4; ++i)
                box[i / 2][i % 2] = std::stod(items[i + 1]);

            if(StartsWith(line, "DIE_ORIG"))
                header.origin = box;
            else header.size = box;
        }
        //TEMP
        else if(StartsWith(line, "TEMP")) {
            Split(line, items);
            for(size_t i = 1; i < items.size(); ++i)
                header.temperatures.push_back(std::stod(items[i]));
            if(header.temperatures.size() < 5) {
                if(err) *err = Format2String("Error: at least 5 temperature points in line %1%!", count);
                return false;
            }
        }
        //TILE
        else if(StartsWith(line, "TILE")) {
            Split(line, items);
            if(items.size() != 3) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            header.tiles.x = std::stol(items[1]);
            header.tiles.y = std::stol(items[2]);
            if(header.tiles.x > maxTileNum || header.tiles.y > maxTileNum) {
                if(err) *err = Format2String("Error: exceeded maxinum number of tiles: %1%.", maxTileNum);
                return false;
            }
        }
        //RESOLUTION
        else if(StartsWith(line, "RESOLUTION")) {
            Split(line, items);
            if(items.size() != 2) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            header.resolution = std::stod(items[1]);
        }
        //SCALE_FACTOR
        else if(StartsWith(line, "SCALE_FACTOR")) {
            Split(line, items);
            if(items.size() != 2) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            header.scale = std::stod(items[1]);
        }
        //ALL_LAYERS
        else if(StartsWith(line, "ALL_LAYERS")) {
            Split(line, items);
            if(items.size() < 2) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            size_t lyrNum = std::stol(items[1]);
            if(items.size() != (lyrNum + 2)) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            for(size_t i = 0; i < lyrNum; ++i)
                header.techLayers.emplace_back(std::move(items[i + 2]));
        }
        //METAL_LAYERS
        else if(StartsWith(line, "METAL_LAYERS")) {
            while(!fp.eof()) {
                count++;
                line.clear();
                std::getline(fp, line);
                if(line.empty()) continue;
                if(StartsWith(line, "#")) continue;
                if(StartsWith(Trim(line), "}")) break;

                Split(line, items);
                if(items.size() != 3) {
                    if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                    return false;
                }

                ECTMv1MetalLayer mLayer;
                mLayer.name = std::move(items[0]);
                mLayer.thickness = header.encrypted ? decrypt(items[1], mLayer.name.size()) : std::stod(items[1]);
                mLayer.elevation = header.encrypted ? decrypt(items[2], mLayer.name.size()) : std::stod(items[2]);
                header.metalLayers.emplace_back(std::move(mLayer));
            }
        }
        //VIA_LAYERS
        else if(StartsWith(line, "VIA_LAYERS")) {
            while(!fp.eof()) {
                count++;
                line.clear();
                std::getline(fp, line);
                if(line.empty()) continue;
                if(StartsWith(line, "#")) continue;
                if(StartsWith(Trim(line), "}")) break;

                Split(line, items);
                if(items.size() != 3) {
                    if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                    return false;
                }

                ECTMv1ViaLayer vLayer;
                vLayer.name = std::move(items[0]);
                vLayer.botLayer = std::move(items[1]);
                vLayer.topLayer = std::move(items[2]);
                header.viaLayers.emplace_back(std::move(vLayer));
            }
        }  
    }

    fp.close();
    return true;
}

ECAD_INLINE bool ParseCTMv1PowerFile(const std::string & filename, EGridData & powers, std::string * err)
{
    std::ifstream fp(filename.c_str(), std::ios::in | std::ios::binary);
    if(!fp.good()) {
        if(err) *err = Format2String("Error: failed to open file %1%!", filename);
        return false;
    }

    float f;
    size_t i = 0, size = powers.Size();
    while(fp.read(reinterpret_cast<char*>(&f), sizeof(float)) && i < size) { powers[i++] = f; }
    
    fp.close();
    return true;
}

ECAD_API bool ParseCTMv1DensityFile(const std::string & filename, const size_t size, std::vector<SPtr<EGridData> > & density, std::string * err)
{
    std::ifstream fp(filename.c_str(), std::ios::in | std::ios::binary);
    if(!fp.good()) {
        if(err) *err = Format2String("Error: failed to open file %1%!", filename);
        return false;
    }

    int id;
    float f;
    size_t i = 0;
    while(!fp.eof() && i < size) {
        //id
        fp.read(reinterpret_cast<char*>(&id), sizeof(int));

        //4 float number for tile box
        for(size_t j = 0; j < 4; ++j)
            fp.read(reinterpret_cast<char*>(&f), sizeof(float));
        
        //density of all layers in LAYER section
        for(size_t j = 0; j < density.size(); ++j) {
            fp.read(reinterpret_cast<char*>(&f), sizeof(float));
            (*density[j])[i] = f;
        }

        i++;
    }

    fp.close();
    return true;
}

#ifdef BOOST_GIL_IO_PNG_SUPPORT
ECAD_INLINE bool GenerateImageProfile(const std::string & filename, const EGridData & data)
{
    using NumType = typename EGridData::ResultType;
    auto min = data.MaxOccupancy(std::less<NumType>());
    auto max = data.MaxOccupancy(std::greater<NumType>());
    auto range = max - min;
    auto rgbaFunc = [&min, &range](NumType d) {
        int r, g, b, a = 255;
        generic::color::RGBFromScalar((d - min) / range, r, g, b);
        return std::make_tuple(r, g, b, a);
    };

    return data.WriteImgProfile(filename, rgbaFunc);
}
#endif//BOOST_GIL_IO_PNG_SUPPORT

}//namespace detail
}//namespace ctm
}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad