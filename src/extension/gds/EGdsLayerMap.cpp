#ifndef ECAD_HEADER_ONLY
#include "EGdsLayerMap.h"
#endif

#include "generic/tools/Parser.hpp"

namespace ecad {

namespace ext {
namespace gds {

namespace psr = generic::parser;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

ECAD_INLINE EGdsLayerMapParser::EGdsLayerMapParser(EGdsLayerMap & layerMap)
 : m_layerMap(layerMap)
{
}

ECAD_INLINE EGdsHelicLayerMapParser::EGdsHelicLayerMapParser(EGdsLayerMap & layerMap)
 : EGdsLayerMapParser(layerMap)
{
}

ECAD_INLINE bool EGdsHelicLayerMapParser::operator() (const std::string & filename)
{
    std::ifstream fp(filename.c_str());
    if(!fp.good()) return false;

    m_layerMap.Clear();
    bool res = (*this)(fp);
    fp.close();
    return res;
}

ECAD_INLINE bool EGdsHelicLayerMapParser::operator() (std::istream & fp)
{

}

ECAD_INLINE bool EGdsHelicLayerMapParser::ParseOneLine(const std::string & line, EGdsLayer & layer)
{
    // auto begin = line.begin(), end = line.end();
    // bool ok = qi::phrase_parse(begin, end,
    //                             (
    //                                 qi::double_[phx::ref(point[0]) = qi::_1] >>
    //                                 qi::double_[phx::ref(point[1]) = qi::_1]
    //                             ),
    //                             ascii::space);
    // if(!ok || begin != end) return false;
    return true;
}

}//namespace gds   
}//namespace ext
}//namespace ecad
