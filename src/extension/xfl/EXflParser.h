#ifndef ECAD_EXT_XFL_EXFLPARSER_H
#define ECAD_EXT_XFL_EXFLPARSER_H
#include "EXflObjects.h"
#include "ECadAlias.h"
#include "generic/tools/StringHelper.hpp"
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/qi_no_case.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/fusion.hpp>
#include <boost/phoenix/stl.hpp>
#include <boost/bind/bind.hpp>
#include <boost/variant.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Material, (bool, isMetal) (std::string, name) (double, conductivity) (double, permittivity) (double, permeability) (double, lossTangent) (int, causality))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Layer, (std::string, name) (double, thickness) (char, type) (std::string, conductingMat) (std::string, dielectricMat))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Point, (double, x) (double, y))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Arc, (int, type) (ecad::ext::xfl::Point, end), (ecad::ext::xfl::Point, mid))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Rectangle, (double, width) (double, height))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Square, (double, width))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Diamond, (double, width))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Circle, (double, diameter))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Annular, (double, outerDia) (double, innerDia))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Oblong, (double, width) (double, left) (double, right))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Bullet, (double, width) (double, left) (double, right))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Finger, (double, width) (double, left) (double, right))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::TemplateShape, (int, id) (ecad::ext::xfl::Shape, shape))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Pad, (int, sigLyr) (int, shapeId) (double, shapeRot) (int, apShapeId) (double, apShapeRot))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Padstack, (int, id) (std::vector<ecad::ext::xfl::Pad>, pads))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Via, (std::string, name) (int, padstackId) (double, padstackRot) (int, shapeId) (double, shapeRot) (double, barrelThickness) (std::string, material))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Node, (std::string, component) (std::string, pinName) (std::string, ioType) (int, npeGrpNum) (int, npeNdUsage) (ecad::ext::xfl::Point, loc) (int, layer))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Net, (std::string, name) (char, type) (int, attrId) (int, analysis) (int, npeType) (int, anlandBch) (std::vector<ecad::ext::xfl::Node>, nodes))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstPath, (int, layer) (double, width) (ecad::ext::xfl::Composite, path))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstVia, (int, sLayer) (int, eLayer) (std::string, name) (ecad::ext::xfl::Point, loc) (double, rot) (char, mirror))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstBondwire, (int, sLayer) (int, eLayer) (int, id) (ecad::ext::xfl::Point, sLoc) (ecad::ext::xfl::Point, eLoc) (std::string, die1) (std::string, die2))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstPolygon, (bool, isVoid) (int, layer) (ecad::ext::xfl::Polygon, polygon))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstRectangle, (bool, isVoid) (int, layer) (ecad::ext::xfl::Rectangle, rectangle) (ecad::ext::xfl::Point, loc))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstSquare, (bool, isVoid) (int, layer) (ecad::ext::xfl::Square, square) (ecad::ext::xfl::Point, loc))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstDiamond, (bool, isVoid) (int, layer) (ecad::ext::xfl::Diamond, diamond) (ecad::ext::xfl::Point, loc))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstCircle, (bool, isVoid) (int, layer) (ecad::ext::xfl::Circle, circle) (ecad::ext::xfl::Point, loc))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstAnnular, (int, layer) (ecad::ext::xfl::Annular, annular) (ecad::ext::xfl::Point, loc))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstComposite, (bool, isVoid) (int, layer) (ecad::ext::xfl::Composite, composite))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::InstShape, (bool, isVoid) (int, layer) (int, shapeId) (ecad::ext::xfl::Point, loc) (double, rot) (char, mirror) (bool, rotThenMirror))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Route, (std::string, net) (std::vector<ecad::ext::xfl::InstObject>, objects))

namespace ecad {
namespace ext {
namespace xfl {

using namespace generic;
namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;
namespace spirit = boost::spirit;
namespace ascii = boost::spirit::ascii;

///@brief print line number and line content if error is met 
template <typename Iterator>
struct ErrorHandler
{
	template <typename, typename, typename>
	struct result { using type = void; };

	ErrorHandler(Iterator first, Iterator last, std::ostream & os = std::cout)
	  : first(first), last(last), os(os) {}

	template <typename Message, typename What>
	void operator() (const Message & message, const What & what, Iterator errPos) const
	{
		int line;
		Iterator lineStart = GetPos(errPos, line);
		if (errPos != last) {
			os << message << what << " line " << line << ':' << std::endl;
			os << GetLine(lineStart) << std::endl;
			for (; lineStart != errPos; ++lineStart)
				os << ' ';
			os << '^' << std::endl;
		}
		else {
			os << "Unexpected end of file. ";
			os << message << what << " line " << line << std::endl;
		}
	}

	Iterator GetPos(Iterator errPos, int & line) const
	{
		line = 1;
		Iterator i = first;
		Iterator lineStart = first;
		while (i != errPos) {
			bool eol = false;
			if (i != errPos && *i == '\r') {// CR
				eol = true;
				lineStart = ++i;
			}
			if (i != errPos && *i == '\n') {// LF
				eol = true;
				lineStart = ++i;
			}
			if (eol) ++line;
			else ++i;
		}
		return lineStart;
	}

	std::string GetLine(Iterator errPos) const
	{
		Iterator i = errPos;
		// position i to the next EOL
		while (i != last && (*i != '\r' && *i != '\n')) { ++i; }
		return std::string(errPos, i);
	}

	Iterator first;
	Iterator last;
	std::vector<Iterator> iters;
    std::ostream & os;
};

struct EXflReader
{
    EXflDB & db;
    explicit EXflReader(EXflDB & db) : db(db) {}
    
    bool operator() (const std::string & xflFile)
    {
        std::ifstream in;
		in.open(xflFile.c_str(), std::ios::in);
		if(!in.is_open()) return false;

		in.unsetf(std::ios::skipws);// No white space skipping!
		auto str = std::string(std::istreambuf_iterator<char>(in.rdbuf()), std::istreambuf_iterator<char>());
		auto iter = str.cbegin();
		auto end = str.cend();

        using Skipper = SkipperGrammar<std::string::const_iterator>;
        using ErrorHandler = ErrorHandler<std::string::const_iterator>;

        Skipper skipper;
        ErrorHandler errHandler(iter, end);
        EXflGrammar<std::string::const_iterator, Skipper> grammar(db, errHandler);
        
        bool res = qi::phrase_parse(iter, end, grammar, skipper);
        return res && iter == end;
    }

	template <typename Iterator>
	struct SkipperGrammar : qi::grammar<Iterator>
	{
		qi::rule<Iterator> skip;
		SkipperGrammar() : SkipperGrammar::base_type(skip)
		{
			using qi::char_;
			using qi::eol;
			using spirit::lit;

			skip = ascii::space 
				| ("#" >> *(char_ - eol) >> eol )
				;
		}
	};
    
    template <typename Iterator, typename Skipper>
    struct EXflGrammar : qi::grammar<Iterator, Skipper>
    {
        qi::rule<Iterator, Skipper> expression;
        qi::rule<Iterator, Skipper> others;
		qi::rule<Iterator, Skipper> materialSection;
		qi::rule<Iterator, Skipper> materialFreqSection;
		qi::rule<Iterator, Skipper> layerSection;
		qi::rule<Iterator, Skipper> shapeSection;
		qi::rule<Iterator, Skipper> boardGeomSection;
		qi::rule<Iterator, Skipper> padstackSection;
		qi::rule<Iterator, Skipper> viaSection;
		qi::rule<Iterator, Skipper> partSection;
		qi::rule<Iterator, Skipper> componentSection;
		qi::rule<Iterator, Skipper> netAttrSection;
		qi::rule<Iterator, Skipper> netlistSection;
		qi::rule<Iterator, Skipper> routeSection;
		qi::rule<Iterator, std::string(), Skipper> unknownSection;
		qi::rule<Iterator, std::string(), Skipper> startTag;
		qi::rule<Iterator, void(std::string), Skipper> endTag;
        qi::rule<Iterator, std::string(), Skipper> text;
        qi::rule<Iterator, std::string(), Skipper> textNC; //no constraints
		qi::rule<Iterator, std::string(), Skipper> textDQ; //text with double quotes ""
		qi::rule<Iterator, std::string(), Skipper> textEOL; //text to then end of line
		qi::rule<Iterator, Material(), Skipper> material;
		qi::rule<Iterator, Layer(), Skipper> layer;
		qi::rule<Iterator, Point(), Skipper> point;
		qi::rule<Iterator, Polygon(), Skipper> polygon;
		qi::rule<Iterator, Arc(), Skipper> arc;
		qi::rule<Iterator, Composite(), Skipper> composite;
		qi::rule<Iterator, Rectangle(), Skipper> rectangle;
		qi::rule<Iterator, Square(), Skipper> square;
		qi::rule<Iterator, Diamond(), Skipper> diamond;
		qi::rule<Iterator, Circle(), Skipper> circle;
		qi::rule<Iterator, Annular(), Skipper> annular;
		qi::rule<Iterator, Oblong(), Skipper> oblong;
		qi::rule<Iterator, Bullet(), Skipper> bullet;
		qi::rule<Iterator, Finger(), Skipper> finger;
		qi::rule<Iterator, TemplateShape(), Skipper> shape;
		qi::rule<Iterator, Pad(), Skipper> pad;
		qi::rule<Iterator, Padstack(), Skipper> padstack;
		qi::rule<Iterator, Via(), Skipper> via;
		qi::rule<Iterator, Node(), Skipper> node;
		qi::rule<Iterator, Net(), Skipper> net;
		qi::rule<Iterator, InstPath(), Skipper> instPath;
		qi::rule<Iterator, InstVia(), Skipper> instVia;
		qi::rule<Iterator, InstBondwire(), Skipper> instBondwire;
		qi::rule<Iterator, InstPolygon(), Skipper> instPolygon;
		qi::rule<Iterator, InstRectangle(), Skipper> instRectangle;
		qi::rule<Iterator, InstSquare(), Skipper> instSquare;
		qi::rule<Iterator, InstDiamond(), Skipper> instDiamond;
		qi::rule<Iterator, InstCircle(), Skipper> instCircle;
		qi::rule<Iterator, InstAnnular(), Skipper> instAnnular;
		qi::rule<Iterator, InstComposite(), Skipper> instComposite;
		qi::rule<Iterator, InstShape(), Skipper> instShape;
		qi::rule<Iterator, Route(), Skipper> route;

        EXflDB & db;
        EXflGrammar(EXflDB & db, ErrorHandler<Iterator> & errorHandler)
        : EXflGrammar::base_type(expression), db(db)
        {
			using qi::eoi;
			using qi::eol;
            using qi::int_;
            using qi::uint_;
            using qi::double_;
            using qi::char_;
            using qi::_1; using qi::_2; using qi::_3; using qi::_4; using qi::_5; using qi::_6;
            using qi::_a; using qi::_r1;
			using qi::_val;
            using qi::lexeme;
            using qi::no_case;
			using phx::at_c;
            using phx::push_back;
            using spirit::repeat;
            using spirit::as_string;
            using spirit::lit;
			using spirit::inf;

            expression = //qi::eps > 
				*(
					(
						others
						| materialSection
						| materialFreqSection
						| layerSection
						| shapeSection
						| boardGeomSection
						| padstackSection
						| viaSection
						| partSection
						| componentSection
						| netAttrSection
						| netlistSection
						| routeSection
					) |
					unknownSection[phx::bind(&EXflGrammar::UnknownSectionHandle, this, _1)] |
					textEOL[phx::bind(&EXflGrammar::UnrecognizedHandle, this, _1)]
				)
            ;

            others = 
                  (lexeme[no_case[".version"]] >> int_ >> int_) [phx::bind(&EXflGrammar::VersionHandle, this, _1, _2)]
                | (lexeme[no_case[".unit"]] >> text) [phx::bind(&EXflGrammar::UnitHandle, this, _1)]
				| (lexeme[no_case[".design_type"]] >> text) [phx::bind(&EXflGrammar::DesignTypeHandle, this, _1)]
				| (lexeme[no_case[".scale"]] >> double_) [phx::bind(&EXflGrammar::ScaleHandle, this, _1)]
			;

            materialSection = lexeme[no_case[".material"]] >>
				*(
					material[phx::bind(&EXflGrammar::MaterialHandle, this, _1)]
				) >>
				lexeme[no_case[".end material"]]
			;
			
			materialFreqSection = lexeme[no_case[".material_frequency"]] >>
				*(char_ - lexeme[no_case[".end material_frequency"]]) >>//todo
				lexeme[no_case[".end material_frequency"]]
			;

			layerSection = lexeme[no_case[".layer"]] >>
				*(
					layer[phx::bind(&EXflGrammar::LayerHandle, this, _1)]
				) >>
				lexeme[no_case[".end layer"]]
			;

			shapeSection = lexeme[no_case[".shape"]] >>
				*(
					shape[phx::bind(&EXflGrammar::ShapeHandle, this, _1)]
				) >>
				lexeme[no_case[".end shape"]]
			;
			
			boardGeomSection = lexeme[no_case[".board_geom"]] >>
				(
					  (lexeme[no_case["polygon"]] >> polygon) [phx::bind(&EXflGrammar::BoardPolygonHandle, this, _1)]
					| (lexeme[no_case["composite"]] >> composite) [phx::bind(&EXflGrammar::BoardCompositeHandle, this, _1)]
					| (lexeme[no_case["shape"]] >> int_ >> point >> double_ >> char_("XYN")) [phx::bind(&EXflGrammar::BoardShapeWithRotMirrorHandle, this, _1, _2, _3, _4)]
					| (lexeme[no_case["shape"]] >> int_ >> point >> char_("XYN") >> double_) [phx::bind(&EXflGrammar::BoardShapeWithMirrorRotHandle, this, _1, _2, _3, _4)]
				)
				>>
				lexeme[no_case[".end board_geom"]]
			;

			padstackSection = lexeme[no_case[".padstack"]] >>
				*padstack [phx::bind(&EXflGrammar::PadstackHandle, this, _1)] >>
				lexeme[no_case[".end padstack"]]
			;

			viaSection = lexeme[no_case[".via"]] >>
				*via [phx::bind(&EXflGrammar::ViaHandle, this, _1)] >>
				lexeme[no_case[".end via"]]
			;

			partSection = lexeme[no_case[".part"]] >>
				*(char_ - lexeme[no_case[".end part"]]) >>//todo
				lexeme[no_case[".end part"]]
			;

			componentSection = lexeme[no_case[".component"]] >>
				*(char_ - lexeme[no_case[".end component"]]) >>//todo
				lexeme[no_case[".end component"]]
			;

			netAttrSection = lexeme[no_case[".netattr"]] >>
				*(char_ - lexeme[no_case[".end netattr"]]) >>//todo
				lexeme[no_case[".end netattr"]]
			;

			netlistSection = lexeme[no_case[".netlist"]] >>
				*net [phx::bind(&EXflGrammar::NetHandle, this, _1)] >>
				lexeme[no_case[".end netlist"]]
			;

			routeSection = lexeme[no_case[".route"]] >>
				*route [phx::bind(&EXflGrammar::RouteHandle, this, _1)] >>
				lexeme[no_case[".end route"]]
			;

			unknownSection =
				startTag[_val = _1] >>
				*(char_ - lexeme[no_case[".end"]]) >>
				endTag(_val)
			;
 
			startTag = '.' >> !lit("end") >> textNC[_val = _1];
			endTag = lexeme[no_case[".end"]] >> lit(_r1);

			text = lexeme[(char_("a-zA-Z_") >> *char_("a-zA-Z_0-9-"))];
			textNC = lexeme[+char_("a-zA-Z_0-9.-")];
			textDQ = lexeme['"' >> + (char_ - '"') >> '"'];
			textEOL = lexeme[+(char_ - eol) >> eol];

			material = (lexeme[no_case["C"]][at_c<0>(_val) = true] >> textDQ[at_c<1>(_val) = _1] >> double_[at_c<2>(_val) = _1])
					| (lexeme[no_case["D"]][at_c<0>(_val) = false] >> textDQ[at_c<1>(_val) = _1] >> double_[at_c<2>(_val) = _1] >>
					double_[at_c<3>(_val) = _1] >> double_[at_c<4>(_val) = _1] >> double_[at_c<5>(_val) = _1] >> int_[at_c<6>(_val) = _1])
			;

			layer %= textDQ >> double_ >> char_("SDP") >> textDQ >> textDQ;

			point %= double_ >> double_;
			polygon = "{" >> *point [push_back(_val, _1)] >> "}";
			arc = (lexeme[no_case["ARC"]][at_c<0>(_val) = 0]
				| lexeme[no_case["RARC"]][at_c<0>(_val) = 1]
				| lexeme[no_case["ARC3"]][at_c<0>(_val) = 2] ) >>
				(point)[at_c<1>(_val) = _1] >>
				(point)[at_c<2>(_val) = _1]
			;

			composite = "{" >> (point)[push_back(_val, _1)] >>
				*(
					(point) [push_back(_val, _1)]
					| (arc) [push_back(_val, _1)]
				) >>
				"}"
			;

			rectangle 	%= double_ >> double_;
			square 		%= double_;
			diamond 	%= double_;
			circle 		%= double_;
			annular 	%= double_ >> double_;
			oblong 		%= double_ >> double_ >> double_;
			bullet 		%= double_ >> double_ >> double_;
			finger 		%= double_ >> double_ >> double_;

			shape = int_[at_c<0>(_val) = _1] >>
				(
					  (lexeme[no_case["polygon"]] >> polygon	[at_c<1>(_val) = _1])
					| (lexeme[no_case["rectangle"]] >> rectangle[at_c<1>(_val) = _1])
					| (lexeme[no_case["square"]] >> square		[at_c<1>(_val) = _1])
					| (lexeme[no_case["diamond"]] >> square		[at_c<1>(_val) = _1])
					| (lexeme[no_case["circle"]] >> circle		[at_c<1>(_val) = _1])
					| (lexeme[no_case["annular"]] >> annular	[at_c<1>(_val) = _1])
					| (lexeme[no_case["oblong"]] >> oblong		[at_c<1>(_val) = _1])
					| (lexeme[no_case["bullet"]] >> bullet		[at_c<1>(_val) = _1])
					| (lexeme[no_case["finger"]] >> finger		[at_c<1>(_val) = _1])
					| (lexeme[no_case["composite"]] >> composite[at_c<1>(_val) = _1])
				)
				;

			pad %= int_ >> int_ >> double_ >> -(int_ >> double_);
			padstack = int_[at_c<0>(_val) = _1] >> "{" >> *pad [push_back(at_c<1>(_val), _1)] >> "}";

			via %= textNC >> int_ >> double_ >> int_ >> double_ >> -(double_ >> textNC);

			node %= textNC >> textNC >> text >> int_ >> int_ >> "{" >> point >> int_ >> "}";
			net = textDQ[at_c<0>(_val) = _1] >>
				char_("SPG")[at_c<1>(_val) = _1] >>
				int_[at_c<2>(_val) = _1] >>
				int_[at_c<3>(_val) = _1] >>
				int_[at_c<4>(_val) = _1] >>
				int_[at_c<5>(_val) = _1] >>
				"{" >> *node [push_back(at_c<6>(_val), _1)] >> "}"
			;

			instPath %= lexeme[no_case["path"]] >> int_ >> double_ >> composite;
			instVia = lexeme[no_case["via"]] >> int_[at_c<0>(_val) = _1] >> int_[at_c<1>(_val) = _1] >>
				textNC[at_c<2>(_val) = _1] >> point[at_c<3>(_val) = _1] >> double_[at_c<4>(_val) = _1] >>
				-char_("NY")[at_c<5>(_val) = _1]
			;
			instBondwire %= lexeme[no_case["bondwire"]] >> int_ >> int_ >> int_ >> point >> point >> -(textNC >> textNC);
			instPolygon = (lexeme[no_case["polygon"]][at_c<0>(_val) = false] |
				lexeme[no_case["void_polygon"]][at_c<0>(_val) = true]) >> 
				int_[at_c<1>(_val) = _1] >> polygon[at_c<2>(_val) = _1]
			;
			instRectangle = (lexeme[no_case["rectangle"]][at_c<0>(_val) = false] |
				lexeme[no_case["void_rectangle"]][at_c<0>(_val) = true]) >>
				int_[at_c<1>(_val) = _1] >> rectangle[at_c<2>(_val) = _1] >> point[at_c<3>(_val) = _1]
			;
			instSquare = (lexeme[no_case["square"]][at_c<0>(_val) = false] | lexeme[no_case["void_square"]][at_c<0>(_val) = true]) >>
				int_[at_c<1>(_val) = _1] >> square[at_c<2>(_val) = _1] >> point[at_c<3>(_val) = _1]
			;
			
			instDiamond = (lexeme[no_case["diamond"]][at_c<0>(_val) = false] | lexeme[no_case["void_diamond"]][at_c<0>(_val) = true]) >>
				int_[at_c<1>(_val) = _1] >> diamond[at_c<2>(_val) = _1] >> point[at_c<3>(_val) = _1]
			;
			
			instCircle = (lexeme[no_case["circle"]][at_c<0>(_val) = false] |
				lexeme[no_case["void_circle"]][at_c<0>(_val) = true]) >>
				int_[at_c<1>(_val) = _1] >> circle[at_c<2>(_val) = _1] >>
				point[at_c<3>(_val) = _1]
			;
			instAnnular = lexeme[no_case["annular"]] >> int_[at_c<0>(_val) = _1] >> 
				annular[at_c<1>(_val) = _1] >> point[at_c<2>(_val) = _1]
			;
			instComposite = (lexeme[no_case["composite"]][at_c<0>(_val) = false] |
				lexeme[no_case["void_composite"]][at_c<0>(_val) = true]) >>
				int_[at_c<1>(_val) = _1] >>
				composite[at_c<2>(_val) = _1]
			;
			instShape = (lexeme[no_case["shape"]][at_c<0>(_val) = false] |
				lexeme[no_case["void_shape"]][at_c<0>(_val) = true]) >>
				int_[at_c<1>(_val) = _1] >> int_[at_c<2>(_val) = _1] >>
				point[at_c<3>(_val) = _1] >>
				(
					(double_[at_c<4>(_val) = _1] >> char_("NY")[at_c<5>(_val) = _1, at_c<6>(_val) = true]) |
					(char_("NY")[at_c<5>(_val) = _1] >> double_[at_c<4>(_val) = _1, at_c<6>(_val) = false])
				)
			;

			route = textDQ[at_c<0>(_val) = _1] >> "{" >>
				*(
					instPath		[push_back(at_c<1>(_val), _1)] |
					instVia			[push_back(at_c<1>(_val), _1)] |
					instBondwire	[push_back(at_c<1>(_val), _1)] |
					instPolygon		[push_back(at_c<1>(_val), _1)] |
					instRectangle	[push_back(at_c<1>(_val), _1)] |
					instSquare		[push_back(at_c<1>(_val), _1)] |
					instCircle		[push_back(at_c<1>(_val), _1)] |
					instAnnular		[push_back(at_c<1>(_val), _1)] |
					instComposite	[push_back(at_c<1>(_val), _1)] |
					instShape		[push_back(at_c<1>(_val), _1)]
				) >> "}"
			;

            expression.name("XFL Expression");
            others.name("XFL Block Others");
			text.name("XFL Text");
			textNC.name("XFL Text No Constraints");

            qi::on_error<qi::fail> (
                expression,
                phx::ref(std::cout)
                << phx::val("Error! Expecting ")
                << _4 << " here: '"
                << phx::construct<std::string>(_3, _2)
                << phx::val("'\n")
            );
        }

        void VersionHandle(int major, int minor)
        {
            //std::cout << "Version: " << major << "." << minor << std::endl;
			db.version = std::make_pair(major, minor); 
        }

		void UnitHandle(const std::string & unit)
		{
			//std::cout << "Unit: " << unit << std::endl;
			if(str::Equals<str::CaseInsensitive>(unit, "inch"))
				db.unit = Unit::Inch;
			else db.unit = Unit::Millimeter;
		}

		void DesignTypeHandle(const std::string & designType)
		{
			//std::cout << "Design Type: " << designType << std::endl;
			db.designType = designType;
		}

		void ScaleHandle(double scale)
		{
			//std::cout << "Scale: " << scale << std::endl;
			db.scale = scale;
		}

		void MaterialHandle(Material material)
		{
			//std::cout << "Material: " << material.name << ", Conductivity: " << material.conductivity << std::endl;
			db.materials.emplace_back(std::move(material));
		}
		
		void LayerHandle(Layer layer)
		{
			//std::cout << "Layer Name: " << layer.name << ", Thickness: " << layer.thickness << ", Type: " << layer.type << std::endl;
			db.layers.emplace_back(std::move(layer));
		}

		void ShapeHandle(TemplateShape shape)
		{
			//std::cout << "Shape ID: " << shape.id << std::endl;
			db.templates.emplace_back(std::move(shape));
		}

		void BoardPolygonHandle(Polygon polygon)
		{
			//std::cout << "Board Geom Polygon Size: " << polygon.size() << std::endl;
			db.boardGeom = std::move(polygon);
		}

		void BoardCompositeHandle(Composite composite)
		{
			//std::cout << "Board Geom Composite Size: " << composite.size() << std::endl;
			db.boardGeom = std::move(composite);
		}

		void BoardShapeWithRotMirrorHandle(int id, const Point & loc, double rot, char mirror)//rot-unit: degree, mirror: X-axisX, Y-axisY, N-no
		{
			//std::cout << "Board Shape ID: " << id << ", Loc: " << loc.x << ", " << loc.y << ", Rot: " << rot << ", Mirror: " << mirror << std::endl;
			db.boardGeom = BoardShape{id, loc, rot, mirror, true};
		}

		void BoardShapeWithMirrorRotHandle(int id, const Point & loc, char mirror, double rot)//rot-unit: degree, mirror: X-axisX, Y-axisY, N-no
		{
			//std::cout << "Board Shape ID: " << id << ", Loc: " << loc.x << ", " << loc.y << ", Mirror: " << mirror << ", Rot: " << rot << std::endl;
			db.boardGeom = BoardShape{id, loc, rot, mirror, false};
		}

		void PadstackHandle(Padstack padstack)
		{
			// std::cout << "Padstack ID: " << padstack.id << ", Pads: " << padstack.pads.size() << std::endl;
			db.padstacks.emplace_back(std::move(padstack));
		}

		void ViaHandle(Via via)
		{
			// std::cout << "Via Name: " << via.name << ", Padstack ID: " << via.padstackId << ", Material: " << via.material << std::endl;
			db.vias.emplace_back(std::move(via));
		}

		void NetHandle(Net net)
		{
			// std::cout << "Net Name: " << net.name << ", Type: " << net.type << ", Nodes: " << net.nodes.size() << std::endl;
			db.nets.emplace_back(std::move(net));
		}

		void RouteHandle(Route route)
		{
			// std::cout << "Route Name: " << route.net << ", Object Size: " << route.objects.size() << std::endl;
			db.routes.emplace_back(std::move(route));
		}

		void UnknownSectionHandle(const std::string & unknown)
		{
			ECAD_UNUSED(unknown)
			//std::cout << "Unknow Section: " << unknown << std::endl;
		}

		void UnrecognizedHandle(const std::string & unrecognized)
		{
			ECAD_UNUSED(unrecognized)
			// std::cout << "Unrecogized: " << unrecognized << std::endl;
		}
    };
};

}//namespace xfl   
}//namespace ext
}//namespace ecad
#endif//ECAD_EXT_XFL_EXFLPARSER_H