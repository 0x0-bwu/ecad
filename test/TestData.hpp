#ifndef ECAD_TEST_TESTDATA_HPP
#define ECAD_TEST_TESTDATA_HPP
#include "generic/tools/FileSystem.hpp"
#include <string>
namespace ecad_test{
inline std::string GetTestDataPath()
{
    return generic::filesystem::CurrentPath() + "/test/data";
}
}//namespace ecad_test
#endif//ECAD_TEST_TESTDATA_HPP
