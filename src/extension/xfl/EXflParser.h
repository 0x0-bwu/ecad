#ifndef ECAD_EXT_XFL_EXFLPARSER_H
#define ECAD_EXT_XFL_EXFLPARSER_H
#include "EXflObjects.h"
#include "ECadAlias.h"
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
namespace ecad {
namespace ext {
namespace xfl {

struct Point { double x, y; };
using Polygon = std::vector<Point>;
struct Arc { int type; Point end; Point mid; };//type: 0-arc, 1-rarc, 2-arc3
using Composite = std::vector<boost::variant<Point, Arc> >;
struct Pad { int sigLyr; int shapeId; double shapeRot; int apShapeId = -1; double apShapeRot = .0; };//rot unit: degree, ccw
struct Padstack { int id; std::vector<Pad> pads; };
struct Via { std::string name; int padstackId; double padstackRot; int shapeId; double shapeRot; double barrelThickness = 0.0; std::string material; };
struct Node { std::string component; std::string pinName; std::string ioType; int npeGrpNum; int npeNdUsage; Point loc; int layer; };
struct Net { std::string name; char type; int attrId; int analysis; int npeType; int anlandBch; std::vector<Node> nodes; };
}//namespace xfl
}//namespace ext
}//namespace ecad

BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Point, (double, x) (double, y))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Arc, (int, type) (ecad::ext::xfl::Point, end), (ecad::ext::xfl::Point, mid))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Pad, (int, sigLyr) (int, shapeId) (double, shapeRot) (int, apShapeId) (double, apShapeRot))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Padstack, (int, id) (std::vector<ecad::ext::xfl::Pad>, pads))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Via, (std::string, name) (int, padstackId) (double, padstackRot) (int, shapeId) (double, shapeRot) (double, barrelThickness) (std::string, material))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Node, (std::string, component) (std::string, pinName) (std::string, ioType) (int, npeGrpNum) (int, npeNdUsage) (ecad::ext::xfl::Point, loc) (int, layer))
BOOST_FUSION_ADAPT_STRUCT(ecad::ext::xfl::Net, (std::string, name) (char, type) (int, attrId) (int, analysis) (int, npeType) (int, anlandBch) (std::vector<ecad::ext::xfl::Node>, nodes))

namespace ecad {
namespace ext {
namespace xfl {

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

template <typename DatabaseType>
struct EXflReader
{
    DatabaseType db;
    explicit EXflReader(DatabaseType & db) : db(db) {}
    
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
		std::cout << "res: " << (res && iter == end) << std::endl;//wbtest
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
        qi::rule<Iterator, std::string(), Skipper> text;
        qi::rule<Iterator, std::string(), Skipper> textNC; //no constraints
		qi::rule<Iterator, std::string(), Skipper> textDQ; //text with double quotes ""
		qi::rule<Iterator, std::string(), Skipper> textSQ; //text with single quotes ''
		qi::rule<Iterator, Point(), Skipper> point;
		qi::rule<Iterator, Polygon(), Skipper> polygon;
		qi::rule<Iterator, Arc(), Skipper> arc;
		qi::rule<Iterator, Composite(), Skipper> composite;
		qi::rule<Iterator, Pad(), Skipper> pad;
		qi::rule<Iterator, Padstack(), Skipper> padstack;
		qi::rule<Iterator, Via(), Skipper> via;
		qi::rule<Iterator, Node(), Skipper> node;
		qi::rule<Iterator, Net(), Skipper> net;
        DatabaseType & db;
        EXflGrammar(DatabaseType & db, ErrorHandler<Iterator> & errorHandler)
        : EXflGrammar::base_type(expression), db(db)
        {
			using qi::eoi;
			using qi::eol;
            using qi::int_;
            using qi::uint_;
            using qi::double_;
            using qi::char_;
            using qi::_1; using qi::_2; using qi::_3; using qi::_4; using qi::_5; using qi::_6;
            using qi::_a;
			using qi::_val;
            using qi::lexeme;
            using qi::no_case;
			using phx::at_c;
            using phx::push_back;
            using spirit::repeat;
            using spirit::as_string;
            using spirit::lit;
			using spirit::inf;

            expression = qi::eps > 
				*(
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
					  (lexeme[no_case["C"]] >> textDQ >> double_) [phx::bind(&EXflGrammar::ConductingMatHandle, this, _1, _2)]
					| (lexeme[no_case["D"]] >> textDQ >> double_ >> double_ >> double_ >> double_ >> uint_) [phx::bind(&EXflGrammar::DielectricMatHandle, this, _1, _2, _3, _4, _5, _6)]
				) >>
				lexeme[no_case[".end material"]]
				;
			
			materialFreqSection = lexeme[no_case[".material_frequency"]] >>
				+(char_ - lexeme[no_case[".end material_frequency"]]) >>//todo
				lexeme[no_case[".end material_frequency"]]
			;

			layerSection = lexeme[no_case[".layer"]] >>
				*(
					(textDQ >> double_ >> char_("SDP") >> textDQ >> textDQ) [phx::bind(&EXflGrammar::LayerHandle, this, _1, _2, _3, _4, _5)]
				) >>
				lexeme[no_case[".end layer"]]
			;

			shapeSection = lexeme[no_case[".shape"]] >>
				*(
					  (int_ >> lexeme[no_case["polygon"]] >> polygon) [phx::bind(&EXflGrammar::ShapePolygonHandle, this, _1, _2)]
					| (int_ >> lexeme[no_case["rectangle"]] >> double_ >> double_) [phx::bind(&EXflGrammar::ShapeRectangleHandle, this, _1, _2, _3)]
					| (int_ >> lexeme[no_case["square"]] >> double_) [phx::bind(&EXflGrammar::ShapeSquareHandle, this, _1, _2)]
					| (int_ >> lexeme[no_case["circle"]] >> double_) [phx::bind(&EXflGrammar::ShapeCircleHandle, this, _1, _2)]
					| (int_ >> lexeme[no_case["annular"]] >> double_ >> double_) [phx::bind(&EXflGrammar::ShapeAnnularHandle, this, _1, _2, _3)]
					| (int_ >> lexeme[no_case["oblong"]] >> double_ >> double_ >> double_) [phx::bind(&EXflGrammar::ShapeOblongHandle, this, _1, _2, _3, _4)]
					| (int_ >> lexeme[no_case["bullet"]] >> double_ >> double_ >> double_) [phx::bind(&EXflGrammar::ShapeBulletHandle, this, _1, _2, _3, _4)]
					| (int_ >> lexeme[no_case["finger"]] >> double_ >> double_ >> double_) [phx::bind(&EXflGrammar::ShapeFingerHandle, this, _1, _2, _3, _4)]
					| (int_ >> lexeme[no_case["composite"]] >> composite) [phx::bind(&EXflGrammar::ShapeCompositeHandle, this, _1, _2)]
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
				+(char_ - lexeme[no_case[".end part"]]) >>//todo
				lexeme[no_case[".end part"]]
			;

			componentSection = lexeme[no_case[".component"]] >>
				+(char_ - lexeme[no_case[".end component"]]) >>//todo
				lexeme[no_case[".end component"]]
			;

			netAttrSection = lexeme[no_case[".netattr"]] >>
				+(char_ - lexeme[no_case[".end netattr"]]) >>//todo
				lexeme[no_case[".end netattr"]]
			;

			netlistSection = lexeme[no_case[".netlist"]] >>
				*net [phx::bind(&EXflGrammar::NetHandle, this, _1)] >>
				lexeme[no_case[".end netlist"]]
			;

			routeSection = lexeme[no_case[".route"]] >>
				//todo
				lexeme[no_case[".end route"]]
			;

			text = lexeme[(char_("a-zA-Z_") >> *char_("a-zA-Z_0-9-"))];
			textNC = lexeme[+char_("a-zA-Z_0-9.-")];
			textDQ = lexeme['"' >> + (char_ - '"') >> '"'];
			textSQ = lexeme['\'' >> + (char_ - '\'') >> '\''];

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
            std::cout << "Version: " << major << "." << minor << std::endl;
        }

		void UnitHandle(const std::string & unit)
		{
			std::cout << "Unit: " << unit << std::endl;
		}

		void DesignTypeHandle(const std::string & designType)
		{
			std::cout << "Design Type: " << designType << std::endl;
		}

		void ScaleHandle(double scale)
		{
			std::cout << "Scale: " << scale << std::endl;
		}

		void ConductingMatHandle(const std::string & name, double conductivity)
		{
			std::cout << "Material: " << name << ", Conductivity: " << conductivity << std::endl;
		}

		void DielectricMatHandle(const std::string & name, double conductivity, double permittivity, double permeability, double lossTangent, uint causality)
		{
			std::cout << "Material: " << name << ", Conductivity: " << conductivity << ", Permittivity: " << permittivity << ", Permeability: " << permeability << ", Loss Tangent: " << lossTangent << ", Causality: " << causality << std::endl;
		}
		
		void MatetialFreqHandle(const std::string & text)
		{
			std::cout << "Material Freq: " << text << std::endl;
		}

		void LayerHandle(const std::string & name, double thickness, char type, const std::string & conductingMat, const std::string & dielectricMat)
		{
			std::cout << "Layer Name: " << name << ", Thickness: " << thickness << ", Type: " << type << ", Conducting Material: " << conductingMat << ", Dielectric Material: " << dielectricMat << std::endl;
		}

		void ShapePolygonHandle(int id, const Polygon & polygon)
		{
			std::cout << "Shape ID: " << id << ", Size: " << polygon.size() << std::endl;
		}

		void ShapeRectangleHandle(int id, double width, double height)
		{
			std::cout << "Shape ID: " << id << ", Width: " << width << ", Height: " << height << std::endl;
		}

		void ShapeSquareHandle(int id, double width)
		{
			std::cout << "Shape ID: " << id << ", Width: " << width << std::endl;
		}
		
		void ShapeCircleHandle(int id, double radius)
		{
			std::cout << "Shape ID: " << id << ", R: " << radius << std::endl;
		}

		void ShapeAnnularHandle(int id, double outerDia, double innerDia)
		{
			std::cout << "Shape ID: " << id << ", Outer D: " << outerDia << ", Inner D: " << innerDia << std::endl;
		}

		void ShapeOblongHandle(int id, double width, double left, double right)
		{
			std::cout << "Shape ID: " << id << ", Width: " << width << ", Left: " << left << ", Right: " << right << std::endl;
		}

		void ShapeBulletHandle(int id, double width, double left, double right)
		{
			std::cout << "Shape ID: " << id << ", Width: " << width << ", Left: " << left << ", Right: " << right << std::endl;
		}

		void ShapeFingerHandle(int id, double width, double left, double right)
		{
			std::cout << "Shape ID: " << id << ", Width: " << width << ", Left: " << left << ", Right: " << right << std::endl;
		}

		void ShapeCompositeHandle(int id, const Composite & composite)
		{
			std::cout << "Shape ID: " << id << ", Size: " << composite.size() << std::endl; 
		}

		void BoardPolygonHandle(const Polygon & polygon)
		{
			std::cout << "Board Geom Polygon Size: " << polygon.size() << std::endl;
		}

		void BoardCompositeHandle(const Composite & composite)
		{
			std::cout << "Board Geom Composite Size: " << composite.size() << std::endl;
		}

		void BoardShapeWithRotMirrorHandle(int id, const Point & loc, double rot, char mirror)//rot-unit: degree, mirror: X-axisX, Y-axisY, N-no
		{
			std::cout << "Board Shape ID: " << id << ", Loc: " << loc.x << ", " << loc.y << ", Rot: " << rot << ", Mirror: " << mirror << std::endl;
		}

		void BoardShapeWithMirrorRotHandle(int id, const Point & loc, char mirror, double rot)//rot-unit: degree, mirror: X-axisX, Y-axisY, N-no
		{
			std::cout << "Board Shape ID: " << id << ", Loc: " << loc.x << ", " << loc.y << ", Mirror: " << mirror << ", Rot: " << rot << std::endl;
		}

		void PadstackHandle(const Padstack & padstack)
		{
			std::cout << "Padstack ID: " << padstack.id << ", Pads: " << padstack.pads.size() << std::endl;
		}

		void ViaHandle(const Via & via)
		{
			std::cout << "Via Name: " << via.name << ", Padstack ID: " << via.padstackId << ", Material: " << via.material << std::endl;
		}

		void NetHandle(const Net & net)
		{
			std::cout << "Net Name: " << net.name << ", Type: " << net.type << ", Nodes: " << net.nodes.size() << std::endl;
		}
    };

};

}//namespace xfl   
}//namespace ext
}//namespace ecad
#endif//ECAD_EXT_XFL_EXFLPARSER_H