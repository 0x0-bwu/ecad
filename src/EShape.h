#ifndef ECAD_ESHAPE_H
#define ECAD_ESHAPE_H
#include "generic/geometry/Geometries.hpp"
#include "generic/geometry/GeometryIO.hpp"
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
namespace ecad {

using namespace generic::geometry;
using EPolygonData = Polygon2D<ECoord>;
using EPolylineData = Polyline2D<ECoord>;
using EPolygonWithHolesData = PolygonWithHoles2D<ECoord>;
using EPolygonHolesData = typename EPolygonWithHolesData::hole_container;

class ETransform;
class ECAD_API EShape : public Clonable<EShape>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~EShape() = default;
    virtual bool hasHole() const = 0;
    virtual EBox2D GetBBox() const = 0;
    virtual EPolygonData GetContour() const = 0;
    virtual EPolygonWithHolesData GetPolygonWithHoles() const = 0;
    virtual void Transform(const ETransform2D & trans) = 0;
    virtual EShapeType GetShapeType() const = 0;
};

class ECAD_API ERectangle : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EBox2D shape;
    ERectangle() = default;
    ~ERectangle() = default;

    bool hasHole() const;
    EBox2D GetBBox() const;
    EPolygonData GetContour() const;
    EPolygonWithHolesData GetPolygonWithHoles() const;
    void Transform(const ETransform2D & trans);
    EShapeType GetShapeType() const;
protected:
    ///Copy
    virtual Ptr<ERectangle> CloneImp() const override { return new ERectangle(*this); }
};

class ECAD_API EPath : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION

    int m_type = 0;
    ECoord m_width;
public:
    EPolylineData shape;
    EPath() = default;
    ~EPath() = default;
    bool hasHole() const;
    EBox2D GetBBox() const;
    EPolygonData GetContour() const;
    EPolygonWithHolesData GetPolygonWithHoles() const;
    void Transform(const ETransform2D & trans);
    EShapeType GetShapeType() const;
    void SetPoints(const std::vector<EPoint2D> & points);
    void SetType(int type);
    void SetWidth(ECoord width);
protected:
    ///Copy
    virtual Ptr<EPath> CloneImp() const override { return new EPath(*this); }
};

class ECAD_API EPolygon : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPolygonData shape;
    EPolygon() = default;
    ~EPolygon() = default;

    bool hasHole() const;
    EBox2D GetBBox() const;
    EPolygonData GetContour() const;
    EPolygonWithHolesData GetPolygonWithHoles() const;
    void Transform(const ETransform2D & trans);
    EShapeType GetShapeType() const;
    void SetPoints(const std::vector<EPoint2D> & points);
    EPolygon ConvexHull(const EPolygon & other);
protected:
    ///Copy
    virtual Ptr<EPolygon> CloneImp() const override { return new EPolygon(*this); }
};

class ECAD_API EPolygonWithHoles : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPolygonWithHolesData shape;
    EPolygonWithHoles() = default;
    ~EPolygonWithHoles() = default;
    bool hasHole() const;
    EBox2D GetBBox() const;
    EPolygonData GetContour() const;
    EPolygonWithHolesData GetPolygonWithHoles() const;
    void Transform(const ETransform2D & trans);
    EShapeType GetShapeType() const;
    void SetVoid(bool isVoid);
protected:
    ///Copy
    virtual Ptr<EPolygonWithHoles> CloneImp() const override { return new EPolygonWithHoles(*this); }
};

}//namespace ecad

ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::EShape)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPolygonWithHoles)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ERectangle)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPolygon)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPath)

#ifdef ECAD_HEADER_ONLY
#include "EShape.cpp"
#endif

#endif//ECAD_ESHAPE_H