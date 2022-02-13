#ifndef ECAD_EXT_XFL_EXFLPARSER_H
#define ECAD_EXT_XFL_EXFLPARSER_H
#include "EXflObjects.h"
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/qi_no_case.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace ecad {
namespace ext {
namespace xfl {

namespace qi = boost::spirit::qi;
namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;
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
		qi::rule<Iterator, Skipper> material;
		qi::rule<Iterator, Skipper> materialFreq;
		qi::rule<Iterator, Skipper> layer;
		qi::rule<Iterator, Skipper> shape;
        qi::rule<Iterator, std::string(), Skipper> text;
        qi::rule<Iterator, std::string(), Skipper> textNC; //no constraints
		qi::rule<Iterator, std::string(), Skipper> textDQ; //text with double quotes ""
		qi::rule<Iterator, std::string(), Skipper> textSQ; //text with single quotes ''
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
            using qi::lexeme;
            using qi::no_case;
            using spirit::repeat;
            using spirit::as_string;
            using spirit::lit;

            expression = qi::eps > 
				*(
					others
					| material
					| materialFreq
					| layer
					| shape
				) 
            ;
            others = 
                  (lexeme[no_case[".version"]] >> int_ >> int_) [boost::phoenix::bind(&EXflGrammar::VersionHandle, this, _1, _2)]
                | (lexeme[no_case[".unit"]] >> text) [boost::phoenix::bind(&EXflGrammar::UnitHandle, this, _1)]
				| (lexeme[no_case[".design_type"]] >> text) [boost::phoenix::bind(&EXflGrammar::DesignTypeHandle, this, _1)]
				| (lexeme[no_case[".scale"]] >> double_) [boost::phoenix::bind(&EXflGrammar::ScaleHandle, this, _1)]
				;
            material = lexeme[no_case[".material"]] >>
				*(
					  (lexeme[no_case["C"]] >> textDQ >> double_) [boost::phoenix::bind(&EXflGrammar::ConductingMatHandle, this, _1, _2)]
					| (lexeme[no_case["D"]] >> textDQ >> double_ >> double_ >> double_ >> double_ >> uint_) [boost::phoenix::bind(&EXflGrammar::DielectricMatHandle, this, _1, _2, _3, _4, _5, _6)]
				) >> lexeme[no_case[".end material"]]
				;
			
			materialFreq = lexeme[no_case[".material_frequency"]] >>
				lexeme[no_case[".end material_frequency"]]
			;

			layer = lexeme[no_case[".layer"]] >>
				*(
					(textDQ >> double_ >> char_("SDP") >> textDQ >> textDQ) [boost::phoenix::bind(&EXflGrammar::LayerHandle, this, _1, _2, _3, _4, _5)]
				) >> lexeme[no_case[".end layer"]]
			;

			shape = lexeme[no_case[".shape"]] >>
				*(
					(int_ >> lexeme[no_case["circle"]] >> double_) [boost::phoenix::bind(&EXflGrammar::ShapeCircleHandle, this, _1, _2)]
				) >> lexeme[no_case[".end shape"]]
			;
			
			text = lexeme[(char_("a-zA-Z_") >> *char_("a-zA-Z_0-9-"))];
			textNC = lexeme[+char_("a-zA-Z_0-9.-")];
			textDQ = lexeme['"' >> + (char_ - '"') >> '"'];
			textSQ = lexeme['\'' >> + (char_ - '\'') >> '\''];
            expression.name("XFL Expression");
            others.name("XFL Block Others");
			text.name("XFL Text");
			textNC.name("XFL Text No Constraints");

            qi::on_error<qi::fail> (
                expression,
                phoenix::ref(std::cout)
                << phoenix::val("Error! Expecting ")
                << _4 << " here: '"
                << phoenix::construct<std::string>(_3, _2)
                << phoenix::val("'\n")
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

		void ShapeCircleHandle(int id, double radius)
		{
			std::cout << "Shape ID: " << id << ", R: " << radius << std::endl;
		}
    };

};

}//namespace xfl   
}//namespace ext
}//namespace ecad
#endif//ECAD_EXT_XFL_EXFLPARSER_H