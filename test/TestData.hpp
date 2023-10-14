#pragma once
#include "generic/tools/FileSystem.hpp"
#include <string>
namespace ecad_test{
inline std::string GetTestDataPath()
{
    return generic::filesystem::DirName(__FILE__) + "/data";
}
}//namespace ecad_test
