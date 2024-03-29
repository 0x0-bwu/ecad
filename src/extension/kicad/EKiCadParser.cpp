#include "EKiCadParser.h"
#include "EKiCadObjects.h"
namespace ecad::ext::kicad {

ECAD_INLINE EKiCadParser::EKiCadParser()
{
}

ECAD_INLINE bool EKiCadParser::operator() (std::string_view filename, Tree & tree)
{
    std::ifstream in(filename.data());
    if (not in.good()) {
        //todo, report error
        return false;
    }

    tree = ReadTree(in);
    std::cout << tree.m_branches.size() << std::endl;//wbtest
    return true;
}

Tree EKiCadParser::ReadTree(std::istream & in)
{
    char c;
    ReadUntil(in, '(');
    ReadWhitespace(in);
    auto t = Tree{ReadNodeName(in), {}};
    for(;;) {
        ReadWhitespace(in);
        auto b = in.peek();
        if (b == EOF) break;
        if (b == ')') {
            in.get(c);
            break;
        }
        if (b == '(') {
            t.m_branches.emplace_back(ReadTree(in));
            continue;
        }
        if (b == '"') {
            in.get(c);
            t.m_branches.emplace_back(ReadQuotedString(in));
            in.get(c);
            continue;
        }
        t.m_branches.emplace_back(ReadString(in));
    }
    return t;
}

Tree EKiCadParser::ReadString(std::istream & in)
{
    std::string s;
    for (;;) {
        auto b = in.peek();
        if (b == '\t' || b == '\n' || b == '\r' || b == ')' || b == ' ')
            break;
        char c;
        in.get(c);
        s.push_back(c);
    }
    return Tree{s, {}};    
}

Tree EKiCadParser::ReadQuotedString(std::istream & in)
{
    char c;
    std::string s;
    s.push_back('"');
    auto a = in.peek();
    for (;;) {
        auto b = in.peek();
        if (b == '"' && a != '\\')
            break;
        in.get(c);
        s.push_back(c);
        a = b;
    }
    s.push_back('"');
    return Tree{s, {}};
}

std::string EKiCadParser::ReadNodeName(std::istream & in)
{
    char c;
    std::string s;
    for(;;) {
        auto b = in.peek();
        if (b == '\t' || b == '\n' || b == '\r' || b == ' ' || b == ')') break;
        in.get(c);
        s.push_back(c);
    }
    return s;
}

bool EKiCadParser::ReadUntil(std::istream & in, char c)
{
    char input;
    while(in.get(input)) {
        if (input == c)
            return false;
    }
    return true;
}

void EKiCadParser::ReadWhitespace(std::istream & in)
{
    char c;
    for (;;) {
        auto b = in.peek();
        if (b != '\t' && b != '\n' && b != '\r' && b != ' ')
            break;
        in.get(c);
    }
}


} // namespace ecad::ext::kicad