#ifndef ECAD_ESHAPE_H
#define ECAD_ESHAPE_H
#include "generic/geometry/Geometries.hpp"
#include "ECadCommon.h"
#include "Protocol.h"
namespace ecad {

using namespace generic::geometry;

class ETransform;
class ECAD_API EShape : public Clonable<EShape>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~EShape() = default;
    virtual bool hasHole() const = 0;
    virtual const EBox2D & GetBBox() const = 0;
    virtual void SetBBox(EBox2D bbox) = 0;
    virtual Polygon2D<ECoord> GetContour() const = 0;
    virtual PolygonWithHoles2D<ECoord> GetPolygonWithHoles() const = 0;
    virtual void Transform(const ETransform2D & trans) = 0;
};

class ECAD_API ERectangle : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EBox2D shape;
    ERectangle() = default;
    ~ERectangle() = default;

    bool hasHole() const;
    const EBox2D & GetBBox() const;
    void SetBBox(EBox2D bbox);
    Polygon2D<ECoord> GetContour() const;
    PolygonWithHoles2D<ECoord> GetPolygonWithHoles() const;
    void Transform(const ETransform2D & trans);
protected:
    ///Copy
    virtual Ptr<ERectangle> CloneImp() const override { return new ERectangle(*this); }
};

class ECAD_API EPath : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION

    int m_type;
    ECoord m_width;
    mutable EBox2D m_bbox;
public:
    Polyline2D<ECoord> shape;
    EPath() = default;
    ~EPath() = default;
    bool hasHole() const;
    const EBox2D & GetBBox() const;
    void SetBBox(EBox2D bbox);   
    Polygon2D<ECoord> GetContour() const;
    PolygonWithHoles2D<ECoord> GetPolygonWithHoles() const;
    void Transform(const ETransform2D & trans);
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
    mutable EBox2D m_bbox;
public:
    Polygon2D<ECoord> shape;
    EPolygon() = default;
    ~EPolygon() = default;

    bool hasHole() const;
    const EBox2D & GetBBox() const;
    void SetBBox(EBox2D bbox);
    Polygon2D<ECoord> GetContour() const;
    PolygonWithHoles2D<ECoord> GetPolygonWithHoles() const;
    void Transform(const ETransform2D & trans);
    void SetPoints(const std::vector<EPoint2D> & points);
    EPolygon ConvexHull(const EPolygon & other);
protected:
    ///Copy
    virtual Ptr<EPolygon> CloneImp() const override { return new EPolygon(*this); }
};

class ECAD_API EPolygonWithHoles : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    mutable EBox2D m_bbox;
public:
    PolygonWithHoles2D<ECoord> shape;
    EPolygonWithHoles() = default;
    ~EPolygonWithHoles() = default;
    bool hasHole() const;
    const EBox2D & GetBBox() const;
    void SetBBox(EBox2D bbox);
    Polygon2D<ECoord> GetContour() const;
    PolygonWithHoles2D<ECoord> GetPolygonWithHoles() const;
    void Transform(const ETransform2D & trans);
protected:
    ///Copy
    virtual Ptr<EPolygonWithHoles> CloneImp() const override { return new EPolygonWithHoles(*this); }
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPolygonWithHoles)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ERectangle)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPolygon)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPath)
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::EShape)

#ifdef ECAD_HEADER_ONLY
#include "EShape.cpp"
#endif

#endif//ECAD_ESHAPE_H