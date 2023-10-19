#pragma once
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
    virtual bool isValid() const = 0;
};

class ECAD_API ERectangle : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EBox2D shape;
    ERectangle(EPoint2D ll, EPoint2D ur);
    ERectangle() = default;
    ~ERectangle() = default;

    bool hasHole() const override;
    EBox2D GetBBox() const override;
    EPolygonData GetContour() const override;
    EPolygonWithHolesData GetPolygonWithHoles() const override;
    void Transform(const ETransform2D & trans) override;
    EShapeType GetShapeType() const override;
    bool isValid() const override;

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
    bool hasHole() const override;
    EBox2D GetBBox() const override;
    EPolygonData GetContour() const override;
    EPolygonWithHolesData GetPolygonWithHoles() const override;
    void Transform(const ETransform2D & trans) override;
    EShapeType GetShapeType() const override;
    bool isValid() const override;

    void SetPoints(const std::vector<EPoint2D> & points);
    void SetType(int type);
    void SetWidth(ECoord width);
protected:
    ///Copy
    virtual Ptr<EPath> CloneImp() const override { return new EPath(*this); }
};

class ECAD_API ECircle : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    ECircle() = default;
public:
    ECoord r;
    EPoint2D o;
    size_t div;
    ECircle(EPoint2D o, ECoord r, size_t div);
    ~ECircle() = default;

    bool hasHole() const override;
    EBox2D GetBBox() const override;
    EPolygonData GetContour() const override;
    EPolygonWithHolesData GetPolygonWithHoles() const override;
    void Transform(const ETransform2D & trans) override;
    EShapeType GetShapeType() const override;
    bool isValid() const override;

protected:
    ///Copy
    virtual Ptr<ECircle> CloneImp() const override { return new ECircle(*this); }
};

class ECAD_API EPolygon : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPolygonData shape;
    explicit EPolygon(std::vector<EPoint2D> points);
    EPolygon() = default;
    ~EPolygon() = default;

    bool hasHole() const override;
    EBox2D GetBBox() const override;
    EPolygonData GetContour() const override;
    EPolygonWithHolesData GetPolygonWithHoles() const override;
    void Transform(const ETransform2D & trans) override;
    EShapeType GetShapeType() const override;
    bool isValid() const override;

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
    bool hasHole() const override;
    EBox2D GetBBox() const override;
    EPolygonData GetContour() const override;
    EPolygonWithHolesData GetPolygonWithHoles() const override;
    void Transform(const ETransform2D & trans) override;
    EShapeType GetShapeType() const override;
    bool isValid() const override;

protected:
    ///Copy
    virtual Ptr<EPolygonWithHoles> CloneImp() const override { return new EPolygonWithHoles(*this); }
};

class ECAD_API EShapeFromTemplate : public EShape
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    using Template = ETemplateShape;
    ETransform2D m_transform;
    Template m_template = nullptr;
    EShapeFromTemplate() = default;
public:
    explicit EShapeFromTemplate(Template ts, ETransform2D trans = ETransform2D{});
    ~EShapeFromTemplate() = default;
    bool hasHole() const override;
    EBox2D GetBBox() const override;
    EPolygonData GetContour() const override;
    EPolygonWithHolesData GetPolygonWithHoles() const override;
    void Transform(const ETransform2D & trans) override;
    EShapeType GetShapeType() const override;
    bool isValid() const override;

protected:
    ///Copy
    virtual Ptr<EShapeFromTemplate> CloneImp() const override { return new EShapeFromTemplate(*this); }
};

}//namespace ecad

ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::EShape)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPolygonWithHoles)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ERectangle)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPolygon)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPath)