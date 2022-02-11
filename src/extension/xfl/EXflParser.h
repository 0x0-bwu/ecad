#ifndef ECAD_EXT_XFL_EXFLPARSER_H
#define ECAD_EXT_XFL_EXFLPARSER_H
#include "EXflObjects.h"
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/qi_no_case.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <unordered_map>
#include <unordered_set>
namespace ecad {
namespace ext {
namespace xfl {

namespace qi = boost::spirit::qi;
namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;
template <typename Iterator, typename Skipper, typename DatabaseType>
struct EXflGrammar : qi::grammar<Iterator, SKipper>
{
    DatabaseType & db;
    EXflGrammar(DatabaseType & _db){}

};

}//namespace xfl   
}//namespace ext
}//namespace ecad
#endif//ECAD_EXT_XFL_EXFLPARSER_H