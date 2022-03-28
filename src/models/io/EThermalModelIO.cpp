#ifndef ECAD_HEADER_ONLY
#include "models/io/EThermalModelIO.h"
#endif

#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"
namespace ecad {
namespace emodel {
namespace io {

ECAD_INLINE UPtr<EGridThermalModel> makeGridThermalModelFromCtmV1File(const std::string & filename, std::string * err)
{
    auto dir = detail::UntarCtmV1File(filename, err);
    return nullptr;//wbtest
}

namespace detail {
ECAD_INLINE std::string UntarCtmV1File(const std::string & filename, std::string * err)
{
    using namespace generic::format;
    using namespace generic::filesystem;
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

}//namespace detail

}//namespace io
}//namespace emodel
}//namespace ecad