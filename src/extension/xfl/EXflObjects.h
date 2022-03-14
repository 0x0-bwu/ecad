#ifndef ECAD_EXT_XFL_EXFLOBJECTS_H
#define ECAD_EXT_XFL_EXFLOBJECTS_H
#include "generic/geometry/Utility.hpp"
#include "ECadCommon.h"
#include "EShape.h"
#include <boost/variant.hpp>
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
struct Arc { int type; Point end; Point mid; };//type: 0-arc(cw), 1-rarc(ccw), 2-arc3
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
struct InstVia { int sLayer; int eLayer; std::string name; Point loc; double rot; char mirror = 'N'; };//rot unit: degree
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

class EShapeGetter : public boost::static_visitor<UPtr<EShape> >
{
public:
    EShapeGetter(double scale, size_t circleDiv)
     : m_scale(scale), m_circleDiv(circleDiv) {}

    UPtr<EShape> operator() (const Polygon & polygon) const
    {
        auto shape = new EPolygon;
        auto & data = shape->shape;
        for(const auto & point : polygon){
            data << toEPoint2D(point);
        }
        return UPtr<EShape>(shape);
    }

    UPtr<EShape> operator() (const Composite & composite) const
    {
        auto shape = new EPolygon;
        auto & data = shape->shape;

        auto iter = composite.cbegin();
        auto * pt = boost::get<Point>(&(*iter));
        GENERIC_ASSERT(pt != nullptr);
        data << toEPoint2D(*pt);

        iter++;
        Point lastPt = *pt;
        for(; iter != composite.cend(); ++iter) {
            if(pt = boost::get<Point>(&(*iter))) {
                data << toEPoint2D(*pt);
                lastPt = *pt;
            }
            else if(auto * arc = boost::get<Arc>(&(*iter))) {
                auto points = toEPoint2Ds(lastPt, *arc);
                auto iter_p = points.begin(); iter_p++;//skip first one
                for(; iter_p != points.end(); ++iter_p)
                    data << *iter_p;
                lastPt = arc->end;
            }
            else {
                GENERIC_ASSERT(false)
            }
        }
        return UPtr<EShape>(shape);        
    }

    UPtr<EShape> operator() (const Rectangle & rectangle) const
    {
        auto shape = new ERectangle;
        auto & box = shape->shape;
        ECoord halfW  = 0.5 * rectangle.width  * m_scale;
        ECoord halfH = 0.5 * rectangle.height * m_scale; 
        box[0][0] = -0.5 * halfW;
        box[1][0] =  0.5 * halfW;
        box[0][1] = -0.5 * halfH;
        box[1][1] =  0.5 * halfH;

        return UPtr<EShape>(shape);
    }

    UPtr<EShape> operator() (const Square & square) const
    {
        auto shape = new ERectangle;
        auto & box = shape->shape;
        ECoord halfW = 0.5 * square.width;
        box[0][0] = -halfW;
        box[1][0] =  halfW;
        box[0][1] = -halfW;
        box[1][1] =  halfW;

        return UPtr<EShape>(shape);
    }

    UPtr<EShape> operator() (const Diamond & diamond) const
    {
        auto shape = new EPolygon;
        auto & data = shape->shape;
        ECoord halfW = 0.5 * diamond.width;
        data << EPoint2D(halfW, 0) << EPoint2D(0, halfW) << EPoint2D(-halfW, 0) << EPoint2D(0, -halfW);

        return UPtr<EShape>(shape);
    }

    UPtr<EShape> operator() (const Circle & circle) const
    {
        using namespace generic;
        auto shape = new ECircle(EPoint2D(0, 0), 0.5 * circle.diameter, m_circleDiv);
        return UPtr<EShape>(shape);    
    }

    UPtr<EShape> operator() (const Annular & annular) const
    {
        using namespace generic;
        auto shape = new EPolygonWithHoles;
        auto & data = shape->shape;
        ECoord innerR = 0.5 * annular.innerDia * m_scale;
        ECoord outerR = 0.5 * annular.outerDia * m_scale;
        data.outline = geometry::InscribedPolygon(geometry::Circle<ECoord>(EPoint2D(0, 0), outerR), m_circleDiv);
        data.holes.emplace_back(geometry::CircumscribedPolygon(geometry::Circle<ECoord>(EPoint2D(0, 0), innerR), m_circleDiv));

        return UPtr<EShape>(shape);
    }

    UPtr<EShape> operator() (const Oblong & oblong) const
    {
        auto shape = new ERectangle;
        auto & box = shape->shape;
        ECoord halfW = 0.5 * oblong.width * m_scale;
        box[0][0] = -oblong.left  * m_scale;
        box[0][1] = -halfW;
        box[1][0] =  oblong.right * m_scale;
        box[1][1] =  halfW; 

        return UPtr<EShape>(shape);
    }

    UPtr<EShape> operator() (const Bullet & bullet) const
    {
        using namespace generic;
        auto shape = new EPolygon;
        auto & data = shape->shape;
        ECoord halfW = 0.5 * bullet.width * m_scale;
        data << EPoint2D(-bullet.left * m_scale,  halfW);
        data << EPoint2D(-bullet.left * m_scale, -halfW);
        data << EPoint2D(bullet.right * m_scale - halfW, -halfW);

        geometry::Arc<ECoord> arc(EPoint2D(bullet.right * m_scale - halfW, 0), data.Back(), math::pi);
        auto arcPts = geometry::toPolyline(arc, m_circleDiv);
        auto iter = arcPts.begin(); iter++;//skip first one
        data.Insert(data.End(), iter, arcPts.end());

        return UPtr<EShape>(shape);
    }

    UPtr<EShape> operator() (const Finger & finger) const
    {
        using namespace generic;
        auto shape = new EPolygon;
        auto & data = shape->shape;
        ECoord halfW = 0.5 * finger.width * m_scale;
        data << EPoint2D( finger.right * m_scale - halfW, -halfW);

        geometry::Arc<ECoord> arc1(EPoint2D(finger.right * m_scale - halfW, 0), data.Back(), math::pi);
        auto arc1Pts = geometry::toPolyline(arc1, m_circleDiv);
        auto iter1 = arc1Pts.begin(); iter1++;//skip first one
        data.Insert(data.End(), iter1, arc1Pts.end());

        data << EPoint2D(-finger.left  * m_scale + halfW,  halfW);
        geometry::Arc<ECoord> arc2(EPoint2D(-finger.left  * m_scale + halfW, 0), data.Back(), math::pi);
        auto arc2Pts = geometry::toPolyline(arc2, m_circleDiv);
        auto iter2 = arc2Pts.begin(); iter2++;//skip first one
        data.Insert(data.End(), iter2, arc2Pts.end());
        
        return UPtr<EShape>(shape);
    }

private:
    EPoint2D toEPoint2D(const Point & pt) const
    {
        return EPoint2D(pt.x * m_scale, pt.y * m_scale);
    }

    std::vector<EPoint2D> toEPoint2Ds(const Point & start, const Arc & arc) const
    {
        using namespace generic;
        auto toFPoint2D = [](const Point & p) { return FPoint2D(p.x, p.y); };
        std::vector<geometry::Point2D<double> > fps;
        if(0 == arc.type) {//arc
            auto radian = geometry::Angle(toFPoint2D(arc.end), toFPoint2D(arc.mid), toFPoint2D(start));
            geometry::Arc<double> a(toFPoint2D(arc.mid), toFPoint2D(start), -radian);
            fps = geometry::toPolyline(a, m_circleDiv);

        }
        else if(1 == arc.type) {//rarc
            auto radian = geometry::Angle(toFPoint2D(start), toFPoint2D(arc.mid), toFPoint2D(arc.end));
            geometry::Arc<double> a(toFPoint2D(arc.mid), toFPoint2D(start), radian);
            fps = geometry::toPolyline(a, m_circleDiv);
        }
        else {
            geometry::Arc3<double> a(toFPoint2D(start), toFPoint2D(arc.mid), toFPoint2D(arc.end));
            auto fps = geometry::toPolyline(a, m_circleDiv); 
        }
        std::vector<EPoint2D> points;
        for(const auto & fpt : fps)
            points.emplace_back((fpt * m_scale).template Cast<ECoord>());
        return points;
    }

private:
    double m_scale = 1.0;
    size_t m_circleDiv = 12;
};

struct EXflDB
{
    Unit unit;
    double scale;
    Version version;
    bool hasBoardGeom;
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
        hasBoardGeom = false;
    }

    void BuildLUTs(const EShapeGetter & eShapeGetter)
    {
        ClearLUTs();
        for(const auto & net : nets)
            m_nets.insert(std::make_pair(net.name, &net));
        
        for(const auto & via : vias)
            m_vias.insert(std::make_pair(via.name, &via));

        for(const auto & padstack : padstacks)
            m_padstacks.insert(std::make_pair(padstack.id, &padstack));
    
        for(const auto & t : templates){
            auto  s = boost::apply_visitor(eShapeGetter, t.shape);
            auto ts = ETemplateShape(std::move(s));
            m_templates.insert(std::make_pair(t.id, ts));
        }
    }

    CPtr<Net> GetNet(const std::string & name)
    {
        auto iter = m_nets.find(name);
        return iter == m_nets.end() ? nullptr : iter->second;
    }

    CPtr<Padstack> GetPadstack(int id)
    {
        auto iter = m_padstacks.find(id);
        return iter == m_padstacks.end() ? nullptr : iter->second;
    }

    ETemplateShape GetTemplateShape(int id)
    {
        auto iter = m_templates.find(id);
        return iter == m_templates.end() ? nullptr : iter->second;
    }

private:
    void ClearLUTs()
    {
        m_nets.clear();
        m_vias.clear();
        m_padstacks.clear();
        m_templates.clear();
    }

    std::unordered_map<std::string, CPtr<Net> > m_nets;
    std::unordered_map<std::string, CPtr<Via> > m_vias;
    std::unordered_map<int, CPtr<Padstack> > m_padstacks;
    std::unordered_map<int, ETemplateShape > m_templates;
};

}//namespace xfl   
}//namespace ext
}//namespace ecad
#endif//ECAD_EXT_XFL_EXFLOBJECTS_H