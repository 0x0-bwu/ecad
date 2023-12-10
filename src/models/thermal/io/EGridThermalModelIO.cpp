#include "models/thermal/io/EGridThermalModelIO.h"

#include "generic/tools/StringHelper.hpp"
#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"
#include "generic/tools/Color.hpp"

namespace ecad {
namespace emodel {
namespace etherm {
namespace io {

using namespace generic::str;
using namespace generic::fmt;
using namespace generic::filesystem;

ECAD_INLINE bool GenerateTxtProfile(const EGridThermalModel & model, const std::string & filename, std::string * err)
{
    auto dir = DirName(filename);
    if(!PathExists(dir)) MakeDir(dir);
    if(!PathExists(dir) || !isDirWritable(dir)) {
        if(err) *err = Fmt2Str("Error: unwritable folder: %1%.", dir);
        return false;
    }
    
    std::ofstream out(filename);
    if(!out.is_open()) {
        if(err) *err = Fmt2Str("Error: failed to open file: %1%.", filename);
        return false;
    }
    ECoordUnits coordUnits(ECoordUnits::Unit::Meter);
    out << std::setiosflags(std::ios::fixed) << std::setprecision(6);

    char sp(32);
    out << "# Version 3.0" << GENERIC_DEFAULT_EOL;

    auto region = model.GetRegion(false) * 1e6;//m to um
    out << "# DIE" << sp << region[0][0] << sp << region[0][1] << sp << region[1][0] << sp << region[1][1] << GENERIC_DEFAULT_EOL;
    out << "# TILE" << sp << model.GridSize().x << sp << model.GridSize().y << GENERIC_DEFAULT_EOL;
    out << "# SCALE_FACTOR" << sp << model.GetScaleH() << GENERIC_DEFAULT_EOL;
    
    std::stringstream ss;
    std::vector<SPtr<ELayerMetalFraction> > mfs;
    for(const auto & layer : model.GetLayers()) {
        auto mf = layer.GetMetalFraction();
        if(nullptr == mf) {
            if(err) *err = Fmt2Str("Error: missing data in layer: %1%", layer.GetName());
            return false;
        }
        mfs.push_back(mf);
        const auto & name = layer.GetName();
        ss << sp << (name.empty() ? "Unknown" : name);
    }
    out << "# LAYER" << sp << model.GetLayers().size() << ss.str() << GENERIC_DEFAULT_EOL;
    out << "# TileID X1(um) Y1(um) X2(um) Y2(um)" << ss.str() << GENERIC_DEFAULT_EOL;

    auto res = model.GetResolution(false);
    res[0] *= 1e6; res[1] *= 1e6;//m to um
    auto sBox = [&region, &res](size_t x, size_t y)
    {
        char sp(32);
        std::stringstream ss;
        auto ll = region[0] + FPoint2D(x * res[0], y * res[1]);
        auto ur = ll + FPoint2D(res[0], res[1]);
        ss << ll[0] << sp << ll[1] << sp << ur[0] << sp << ur[1];
        return ss.str();
    };

    size_t id = 0;
    for(size_t x = 0; x < model.GridSize().x; ++x) {
        for(size_t y = 0; y < model.GridSize().y; ++y) {
            out << ++id << sp << sBox(x, y);
            for(const auto & mf : mfs)
                out << sp << (*mf)(x, y);
            out << GENERIC_DEFAULT_EOL;
        }
    }

    out.close();
    return true;
}

namespace detail {

ECAD_INLINE bool GenerateImageProfile(const std::string & filename, const EGridData & data, double min, double max)
{
    if(min > max)
        std::swap(min, max);
    auto range = max - min;
    auto rgbaFunc = [&min, &range](auto d) {
        int r, g, b, a = 255;
        generic::color::RGBFromScalar((d - min) / range, r, g, b);
        return std::make_tuple(r, g, b, a);
    };

    return data.WriteImgProfile(filename, rgbaFunc);    
}

}//namespace detail
}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad