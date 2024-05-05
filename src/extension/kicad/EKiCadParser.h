#pragma once
#include "basic/ECadCommon.h"
#include <string_view>
#include <istream>

namespace ecad::ext::kicad {

struct Tree
{
    std::string value;
    std::vector<Tree> branches;
};

class ECAD_API EKiCadParser
{
public:
    EKiCadParser();
    virtual ~EKiCadParser() = default;

    bool operator() (std::string_view filename, Tree & tree);

protected:
    Tree ReadTree(std::istream & in);
    Tree ReadString(std::istream & in);
    Tree ReadQuotedString(std::istream & in);
    std::string ReadNodeName(std::istream & in);
    bool ReadUntil(std::istream & in, char c);
    void ReadWhitespace(std::istream & in);
};

}//namespace ecad::ext::kicad