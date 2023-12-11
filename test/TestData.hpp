#pragma once
#include "generic/tools/FileSystem.hpp"
#include <string>
namespace ecad_test{
inline std::string GetTestDataPath()
{
    return generic::fs::DirName(__FILE__).string() + "/data";
}
}//namespace ecad_test
