#ifndef ECAD_EXT_XFL_EXFLOBJECTS_H
#define ECAD_EXT_XFL_EXFLOBJECTS_H
#include "generic/geometry/Geometries.hpp"
#include "ECadCommon.h"
#include "EShape.h"
#include <unordered_map>
namespace ecad {

namespace ext {
namespace xfl {

using namespace generic::geometry;
using Unit = typename ECoordUnits::Unit;
using Version = std::pair<int, int>;//<major, minor>

struct Material
{
    bool isMetal;
    std::string name;
    double conductivity = .0;
    double permittivity= .0;
    double permeability = .0;
    double lossTangent = .0;
    int causality = 0;//0-none, 1-wideband debye, 2-multipole debye
};

struct Layer
{
    std::string name;
    double thickness;
    char type;//S-Signal layer, D-Dielectric layer, P-Power or ground layer
    std::string conductingMat;
    std::string dielectricMat;
};

struct Point { double x, y; };
using Polygon = std::vector<Point>;
struct Arc { int type; Point end; Point mid; };//type: 0-arc, 1-rarc, 2-arc3
using Composite = std::vector<boost::variant<Point, Arc> >;
struct Rectangle { double width, height; };
struct Square { double width; };
struct Diamond { double width; };
struct Circle { double diameter; };
struct Annular { double outerDia; double innerDia; };
struct Oblong { double width; double left; double right; };
struct Bullet { double width; double left; double right; };
struct Finger { double width; double left; double right; };
using Shape = boost::variant<
                                Polygon,
                                Composite,
                                Rectangle,
                                Square,
                                Diamond, 
                                Circle,
                                Annular, 
                                Oblong, 
                                Bullet, 
                                Finger
                            >;

struct TemplateShape { int id; Shape shape; };

struct BoardShape { int shapeId; Point loc; double rot; char mirror; bool rotThenMirror = true; };
using BoardGeom = boost::variant<Polygon, Composite, BoardShape>;

struct Pad
{   
    int sigLyr;
    int shapeId; 
    double shapeRot;
    int apShapeId = -1;
    double apShapeRot = .0;
};//rot unit: degree, ccw

struct Padstack
{
    int id; 
    std::vector<Pad> pads;
};

struct Via
{
    std::string name; 
    int padstackId; 
    double padstackRot; 
    int shapeId; 
    double shapeRot; 
    double barrelThickness = 0.0; 
    std::string material; 
};

struct Node
{ 
    std::string component; 
    std::string pinName; 
    std::string ioType; 
    int npeGrpNum; 
    int npeNdUsage; 
    Point loc; 
    int layer;
};

struct Net
{ 
    std::string name; 
    char type; 
    int attrId; 
    int analysis; 
    int npeType; 
    int anlandBch; 
    std::vector<Node> nodes; 
};

struct InstPath { int layer; double width; Composite path; };
struct InstVia { int sLayer; int eLayer; std::string name; Point loc; double rot; char mirror = 'N'; };
struct InstBondwire { int sLayer; int eLayer; int id; Point sLoc; Point eLoc; std::string die1; std::string die2; };
struct InstPolygon { bool isVoid; int layer; Polygon polygon; };
struct InstRectangle { bool isVoid; int layer; Rectangle rectangle; Point loc; };
struct InstSquare { bool isVoid; int layer; Square square; Point loc; };
struct InstDiamond { bool isVoid; int layer; Diamond diamond; Point loc; };
struct InstCircle { bool isVoid; int layer; Circle circle; Point loc; };
struct InstAnnular { int layer; Annular annular; Point loc;};
struct InstComposite {bool isVoid; int layer; Composite composite; };
struct InstShape { bool isVoid; int layer; int shapeId; Point loc; double rot; char mirror; bool rotThenMirror = true; };

using InstObject = boost::variant<InstPath,
                                  InstVia,
                                  InstBondwire,
                                  InstPolygon,
                                  InstRectangle, 
                                  InstSquare,
                                  InstDiamond, 
                                  InstCircle, 
                                  InstAnnular, 
                                  InstComposite, 
                                  InstShape
                                  >;

struct Route
{
    std::string net;
    std::vector<InstObject> objects;
};

struct EXflDB
{
    Unit unit;
    double scale;
    Version version;
    BoardGeom boardGeom;
    std::string designType;
    std::vector<Net> nets;
    std::vector<Via> vias;
    std::vector<Layer> layers;
    std::vector<Route> routes;
    std::vector<Material> materials;
    std::vector<Padstack> padstacks;
    std::vector<TemplateShape> templates;

    void Clear()
    {
        ClearLUTs();
        nets.clear();
        vias.clear();
        layers.clear();
        routes.clear();
        materials.clear();
        padstacks.clear();
        templates.clear();
    }

    void BuildLUTs()
    {
        ClearLUTs();
        for(const auto & net : nets)
            m_nets.insert(std::make_pair(net.name, &net));
        
        for(const auto & via : vias)
            m_vias.insert(std::make_pair(via.name, &via));

        for(const auto & temp : templates)
            m_templates.insert(std::make_pair(temp.id, &temp));
    }

private:
    void ClearLUTs()
    {
        m_nets.clear();
        m_vias.clear();
        m_templates.clear();
    }

    std::unordered_map<std::string, CPtr<Net> > m_nets;
    std::unordered_map<std::string, CPtr<Via> > m_vias;
    std::unordered_map<int, CPtr<TemplateShape> > m_templates;
};

}//namespace xfl   
}//namespace ext
}//namespace ecad
#endif//ECAD_EXT_XFL_EXFLOBJECTS_H