#include "models/thermal/io/EChipThermalModelIO.h"

#include "generic/tools/StringHelper.hpp"
#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"

namespace ecad {
namespace model {
namespace io {

using namespace generic::str;
using namespace generic::fmt;
using namespace generic::fs;

ECAD_API UPtr<EChipThermalModelV1> makeChipThermalModelFromCTMv1File(std::string_view filename, std::string * err)
{
    using namespace detail;
    auto dir = UntarCTMv1File(filename, err);
    if(dir.empty()) {
        if(err) *err = Fmt2Str("Error: failed to unarchive the file %1%", filename);
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
        auto powerFile = dir + GENERIC_FOLDER_SEPS + Fmt2Str("power_T[%1%].ctm", i + 1);
        EGridData powers(tiles.x, tiles.y);
        if(!ParseCTMv1PowerFile(powerFile, powers, err)) return nullptr;
        model->powers->GetTable().AddSample(temperature, std::move(powers));
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

ECAD_INLINE bool GenerateCTMv1FileFromChipThermalModelV1(const EChipThermalModelV1 & model, std::string_view dirName, std::string_view filename, std::string * err)
{
    auto ctmDir = std::string(dirName) + GENERIC_FOLDER_SEPS + std::string(filename);
    if (not CreateDir(ctmDir)) {
        if(err) *err = Fmt2Str("Error: unwritable folder: %1%.", ctmDir);
        return false;
    }
    
    using namespace detail;
    if(model.header.temperatures.size() < 5) {
        if(err) *err = "Error: at least 5 temperature points needed in ctm header!";
        return false;
    }

    //header
    std::string headerFile = ctmDir + GENERIC_FOLDER_SEPS + "CTM_header.txt";
    if(!WriteCTMv1HeaderFile(headerFile, model.header, err)) return false;

    //power
    if(!model.powers) {
        if(err) *err = "Error: missing power data in ctm model!";
        return false;
    }
    const auto & powers = *(model.powers);
    for(size_t i = 0; i < model.header.temperatures.size(); ++i) {
        auto temperature = model.header.temperatures.at(i);
        auto table = powers.GetTable().GetTable(temperature);
        if(nullptr == table) {
            if(err) *err = Fmt2Str("Error: failed to get power table at temperature: %1%", temperature);
            return false;
        }
        std::string powerFile = ctmDir + GENERIC_FOLDER_SEPS + Fmt2Str("power_T[%1%].ctm", i + 1);
        if(!WriteCTMv1PowerFile(powerFile, *table, err)) return false;
    }

    //density
    std::vector<SPtr<EGridData> > density;
    for(const auto & layer : model.header.layers) {
        auto iter = model.densities.find(layer.name);
        if(iter == model.densities.end() || nullptr == iter->second) {
            if(err) *err = Fmt2Str("Error: failed to get metal density data of layer: %1%!", layer.name);
            return false;
        }
        density.push_back(iter->second);
    }

    FPoint2D rf = model.header.size[0];
    size_t size = model.header.tiles.x * model.header.tiles.y;
    std::string densityFile = ctmDir + GENERIC_FOLDER_SEPS + "metal_density.ctm";
    if(!WriteCTMv1DensityFile(densityFile, size, model.header.resolution, rf, density, err)) return false;

    //make package
    std::string ctmFile = std::string(dirName) + GENERIC_FOLDER_SEPS + std::string(filename) + ".tar.gz";
    if(!GenerateCTMv1Package(ctmDir, ctmFile, false, err)) return false;

    return true;
}

ECAD_INLINE bool GenerateCTMv1ImageProfiles(const EChipThermalModelV1 & model, std::string_view dirName, std::string * err)
{
    using namespace detail;
    if (not CreateDir(dirName)) return false;

    //power
    if (model.powers) {
        auto range = model.powers->GetRange();
        size_t size = std::min(model.header.temperatures.size(), model.powers->GetTable().GetSampleSize());
        for (size_t i = 0; i < size; ++i) {
            auto t = model.header.temperatures.at(i);
            auto table = model.powers->GetTable().GetTable(t);
            if (nullptr == table) continue;
            std::string filename = std::string(dirName) + GENERIC_FOLDER_SEPS + "power_" + std::to_string(t) + "c.png";
            if (not GenerateImageProfile(filename, *table, range.first, range.second)) continue;
        }
    }
    //density
    for (const auto & density : model.densities) {
        if (nullptr == density.second) continue;
        std::string filename = std::string(dirName) + GENERIC_FOLDER_SEPS + "layer_" + density.first + ".png";
        if (not GenerateImageProfile(filename, *(density.second), 0.0, 1.0)) continue;
    }
    return true;
}

namespace detail {
ECAD_INLINE std::string UntarCTMv1File(std::string_view filename, std::string * err)
{
    if (not FileExists(filename)) {
        if(err) *err = Fmt2Str("Error: file %1% not exists!", filename);
        return std::string{};
    }

    auto dir = DirName(filename);
    auto baseName = BaseName(filename);
    
    //unzip ctm file
    auto untarDir = dir.string() + GENERIC_FOLDER_SEPS + baseName.string();
    if (PathExists(untarDir)) RemoveDir(untarDir);
    CreateDir(untarDir);

    std::string cmd = "tar -zxf" + std::string(filename) + " -C " + untarDir;
    int res = std::system(cmd.c_str());
    return res == 0 ? untarDir : std::string{};
}

ECAD_INLINE bool ParseCTMv1HeaderFile(std::string_view filename, ECTMv1Header & header, std::string * err)
{
    std::ifstream fp(filename.data(), std::ios::in);
    if (not fp.good()) {
        if(err) *err = Fmt2Str("Error: failed to open file %1%!", filename);
        return false;
    }

    auto decrypt = [err](const std::string & s, size_t len) {
        auto tmp = s;
        std::for_each(tmp.begin(), tmp.end(), [&](char & c){ c -= 3 * len + 22; });
        double value = std::stod(tmp);
        bool ok = math::GE(value, 0.0);
        if(!ok && err) *err += Fmt2Str("\nWarning: decrypting for %1% failed, 0.0 will be used instead.", s);
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
                if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            size_t lyrNum = std::stol(items[1]);
            if(items.size() != (lyrNum * 2 + 2)) {
                if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
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
                if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
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
                if(err) *err = Fmt2Str("Error: at least 5 temperature points in line %1%!", count);
                return false;
            }
        }
        //TILE
        else if(StartsWith(line, "TILE")) {
            Split(line, items);
            if(items.size() != 3) {
                if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            header.tiles.x = std::stol(items[1]);
            header.tiles.y = std::stol(items[2]);
            if(header.tiles.x > maxTileNum || header.tiles.y > maxTileNum) {
                if(err) *err = Fmt2Str("Error: exceeded maxinum number of tiles: %1%.", maxTileNum);
                return false;
            }
        }
        //RESOLUTION
        else if(StartsWith(line, "RESOLUTION")) {
            Split(line, items);
            if(items.size() != 2) {
                if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            header.resolution = std::stod(items[1]);
        }
        //SCALE_FACTOR
        else if(StartsWith(line, "SCALE_FACTOR")) {
            Split(line, items);
            if(items.size() != 2) {
                if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            header.scale = std::stod(items[1]);
        }
        //ALL_LAYERS
        else if(StartsWith(line, "ALL_LAYERS")) {
            Split(line, items);
            if(items.size() < 2) {
                if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            size_t lyrNum = std::stol(items[1]);
            if(items.size() != (lyrNum + 2)) {
                if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
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
                    if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
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
                    if(err) *err = Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
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

ECAD_INLINE bool ParseCTMv1PowerFile(std::string_view filename, EGridData & powers, std::string * err)
{
    std::ifstream fp(filename.data(), std::ios::in | std::ios::binary);
    if (not fp.good()) {
        if (err) *err = Fmt2Str("Error: failed to open file %1%!", filename);
        return false;
    }

    float f;
    size_t i = 0, size = powers.Size();
    while(fp.read(reinterpret_cast<char*>(&f), sizeof(float)) && i < size) { powers[i++] = f; }
    
    fp.close();
    return true;
}

ECAD_API bool ParseCTMv1DensityFile(std::string_view filename, const size_t size, std::vector<SPtr<EGridData> > & density, std::string * err)
{
    std::ifstream fp(filename.data(), std::ios::in | std::ios::binary);
    if (not fp.good()) {
        if(err) *err = Fmt2Str("Error: failed to open file %1%!", filename);
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

ECAD_INLINE bool WriteCTMv1HeaderFile(std::string_view filename, const ECTMv1Header & header, std::string * err)
{
    std::ofstream out(filename.data());
    if (not out.is_open()) {
        if(err) *err = Fmt2Str("Error: failed to open file: %1%.", filename);
        return false;
    }

    auto encryptFunc = [](std::string s, size_t len) {
        std::for_each(s.begin(), s.end(), [&](char & c) {c += 3 * len + 22; });
        return s;
    };

    out << std::setiosflags(std::ios::fixed) << std::setprecision(6);

    char sp(32);
    out << header.head << sp << (header.encrypted ? "(encrypted)" : "") << GENERIC_DEFAULT_EOL;
    out << "LAYER" << sp << header.layers.size();
    for(const auto & layer : header.layers) {
        out << sp << layer.name << sp;
        if(!header.encrypted) out << layer.thickness;
        else out << encryptFunc(std::to_string(layer.thickness), layer.name.size());
    }
    out << GENERIC_DEFAULT_EOL;

    const auto & size = header.size;
    out << "DIE" << sp << size[0][0] << sp << size[0][1] << sp << size[1][0] << sp << size[1][1] << GENERIC_DEFAULT_EOL;
    out << "TEMP";
    for(const auto & t : header.temperatures) out << sp << t;
    out << GENERIC_DEFAULT_EOL;
    out << "TILE" << sp << header.tiles.x << sp << header.tiles.y << GENERIC_DEFAULT_EOL;
    out << "RESOLUTION" << sp << header.resolution << GENERIC_DEFAULT_EOL;
    out << "DIE_ORIG" << sp << size[0][0] << sp << size[0][1] << sp << size[1][0] << sp << size[1][1] << GENERIC_DEFAULT_EOL;
    out << "SCALE_FACTOR" << sp << header.scale << GENERIC_DEFAULT_EOL;
    out << "ALL_LAYERS" << sp << header.techLayers.size();
    for(const auto & name : header.techLayers) out << sp << name;
    out << GENERIC_DEFAULT_EOL;
    out << GENERIC_DEFAULT_EOL;
    
    out << std::left;
    out << "METAL_LAYERS {" << GENERIC_DEFAULT_EOL;
    out << "#<layer>        <thickness>        <height>" << GENERIC_DEFAULT_EOL;
    for(const auto & mLayer : header.metalLayers) {
        out << sp << sp;
        out << std::setw(21) << mLayer.name;
        if(!header.encrypted) {
            out << std::setw(21) << mLayer.thickness;
            out << std::setw(21) << mLayer.elevation;
        }
        else {
            out << std::setw(21) << encryptFunc(std::to_string(mLayer.thickness), mLayer.name.size());
            out << std::setw(21) << encryptFunc(std::to_string(mLayer.elevation), mLayer.name.size());
        }
        out << GENERIC_DEFAULT_EOL;
    }
    out << "}" << GENERIC_DEFAULT_EOL;

    out << "VIA_LAYERS {" << GENERIC_DEFAULT_EOL;
    out << "#<layer>        <bottom_layer>        <top_layer>" << GENERIC_DEFAULT_EOL;
    for(const auto & vLayer : header.viaLayers) {
        out << sp << sp;
        out << std::setw(21) << vLayer.name;
        out << std::setw(21) << (vLayer.botLayer.empty() ? "None" : vLayer.botLayer);
        out << std::setw(21) << (vLayer.topLayer.empty() ? "None" : vLayer.topLayer);
        out << GENERIC_DEFAULT_EOL;
    }
    out << "}" << GENERIC_DEFAULT_EOL;

    out.close();
    return true;
}

ECAD_INLINE bool WriteCTMv1PowerFile(std::string_view filename, const EGridData & powers, std::string * err)
{
    std::ofstream out(filename.data(), std::ios::out |std::ios::binary);
    if(!out.is_open()) {
        if(err) *err = Fmt2Str("Error: failed to open file: %1%.", filename);
        return false;
    }

    float temp;
    for(size_t i = 0; i < powers.Size(); ++i) {
        temp = powers[i];
        out.write(reinterpret_cast<char*>(&temp), sizeof(float)); 
    }
    out.close();
    return true;
}

ECAD_INLINE bool WriteCTMv1DensityFile(std::string_view filename, const size_t size, EFloat res, const FPoint2D & ref, const std::vector<SPtr<EGridData> > & density, std::string * err)
{
    if (density.empty()) return false;
    for (const auto & layer : density) {
        if (nullptr == layer) {
            if(err) *err = "Error: missing data in metal density!";
            return false;
        }
        if (layer->Size() != size) {
            if(err) *err = "Error: metal density data mismatch with size!";
            return false;
        }
    } 

    std::ofstream out(filename.data(), std::ios::out |std::ios::binary);
    if (not out.is_open()) {
        if (err) *err = Fmt2Str("Error: failed to open file: %1%.", filename);
        return false;
    }

    int id = 0;
    float temp;
    auto nx = density.front()->Width();
    auto ny = density.front()->Height();
    for (size_t x = 0; x < nx; ++x) {
        float x0 = ref[0] + x * res, x1 = x0 + res;
        for (size_t y = 0; y < ny; ++y) {
            id++;
            float y0 = ref[1] + y * res, y1 = y0 + res;
            out.write(reinterpret_cast<char*>(&id), sizeof(int));
            out.write(reinterpret_cast<char*>(&x0), sizeof(float));
            out.write(reinterpret_cast<char*>(&y0), sizeof(float));
            out.write(reinterpret_cast<char*>(&x1), sizeof(float));
            out.write(reinterpret_cast<char*>(&y1), sizeof(float));         

            for(auto layer : density) {
                temp = (*layer)(x, y);
                out.write(reinterpret_cast<char*>(&temp), sizeof(float));         
            }
        }
    }

    out.close();
    return true;
}

ECAD_INLINE bool GenerateCTMv1Package(std::string_view dirName, const std::string & packName, bool removeDir, std::string * err)
{
    std::string cmd = Fmt2Str("tar zcf %1% -C %2% .", packName, dirName);
    int res = std::system(cmd.c_str());
    if (res == 0 && FileExists(packName)) {
        if(removeDir) RemoveDir(dirName);
        return true;
    }

    if (err) *err = Fmt2Str("Error: failed to generate ctm package: %1%.", packName);
    return false;
}

}//namespace detail
}//namespace io
}//namespace model
}//namespace ecad