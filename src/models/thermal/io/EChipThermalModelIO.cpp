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

    auto headerFile = dir + GENERIC_FOLDER_SEPS + "CTM_header.txt";
    if(!ParseCTMv1HeaderFile(headerFile, model->header, err)) return nullptr;
    return nullptr;//wbtest
}

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
    std::ifstream fp(filename.c_str());
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
        if(StartsWith(line, "LAYER")) {
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
                layer.name = items[i * 2 + 2];
                layer.thickness = header.encrypted ? decrypt(items[i * 2 + 3], layer.name.size()) : std::stod(items[i * 2 + 3]);
                header.layers.emplace_back(std::move(layer));
            }
        }

        //DIE and DIE_ORIG
        if(StartsWith(line, "DIE")) {
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
        if(StartsWith(line, "TEMP")) {
            Split(line, items);
            for(size_t i = 1; i < items.size(); ++i)
                header.temperatures.push_back(std::stod(items[i]));
            if(header.temperatures.size() < 5) {
                if(err) *err = Format2String("Error: at least 5 temperature points in line %1%!", count);
                return false;
            }
        }

        //TILE
        const size_t maxTileNum = 1e5;
        if(StartsWith(line, "TILE")) {
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
        if(StartsWith(line, "RESOLUTION")) {
            Split(line, items);
            if(items.size() != 2) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            header.resolution = std::stod(items[1]);
        }

        //SCALE_FACTOR
        if(StartsWith(line, "SCALE_FACTOR")) {
            Split(line, items);
            if(items.size() != 2) {
                if(err) *err = Format2String("Error: fail to parse file %1% at line %2%.", filename, count);
                return false;
            }
            header.scale = std::stod(items[1]);
        }

        //ALL_LAYERS
        if(StartsWith(line, "ALL_LAYERS")) {
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
          
    }

    return false;
}

}//namespace detail
}//namespace ctm
}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad