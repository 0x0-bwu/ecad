#pragma once
#include "interfaces/IPrimitive.h"
#include "ETransform.h"
#include "EConnObj.h"
#include "ECadDef.h"
#include "EShape.h"
namespace ecad {

class ECAD_API EPrimitive : public EConnObj, public IPrimitive
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPrimitive();
    EPrimitive(std::string name, ELayerId layer, ENetId net);
    explicit EPrimitive(ENetId net);
    virtual ~EPrimitive();

    ///Copy
    EPrimitive(const EPrimitive & other);
    EPrimitive & operator= (const EPrimitive & other);

    virtual Ptr<IText> GetTextFromPrimitive() override;
    virtual Ptr<IConnObj> GetConnObjFromPrimitive() override;
    virtual Ptr<IBondwire> GetBondwireFromPrimitive() override;
    virtual Ptr<IGeometry2D> GetGeometry2DFromPrimitive() override;

    virtual void SetNet(ENetId net) override;
    virtual ENetId GetNet() const override;

    void SetLayer(ELayerId layer) override;
    ELayerId GetLayer() const override;

    EPrimitiveType GetPrimitiveType() const override;

protected:
    virtual void PrintImp(std::ostream & os) const override;
    
protected:
    ELayerId m_layer;
    EPrimitiveType m_type;
};

class ECAD_API EGeometry2D : public EPrimitive, public IGeometry2D
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EGeometry2D();
public:
    EGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape);
    EGeometry2D(ELayerId layer, ENetId net);
    virtual ~EGeometry2D();

    ///Copy
    EGeometry2D(const EGeometry2D & other);
    EGeometry2D & operator= (const EGeometry2D & other);

    void SetShape(UPtr<EShape> shape);
    Ptr<EShape> GetShape() const override;

    void Transform(const ETransform2D & transform) override;

protected:
    virtual Ptr<EGeometry2D> CloneImp() const override { return new EGeometry2D(*this); }
    virtual void PrintImp(std::ostream & os) const override;

protected:
    UPtr<EShape> m_shape;
};

class ECAD_API EBondwire : public EPrimitive, public IBondwire
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EBondwire();
public:
    EBondwire(std::string name, ENetId net, ELayerId startLyr, ELayerId endLyr, EPoint2D startLoc, EPoint2D endLoc);
    virtual ~EBondwire() = default;

    ///Copy
    EBondwire(const EBondwire & other);
    EBondwire & operator= (const EBondwire & other);

    void SetRadius(FCoord r) override;
    FCoord GetRadius() const override;

    void Transform(const ETransform2D & transform) override;

protected:
    virtual Ptr<EBondwire> CloneImp() const override { return new EBondwire(*this); }
    virtual void PrintImp(std::ostream & os) const override;

protected:
    ELayerId m_endLayer;
    EPoint2D m_startLoc;
    EPoint2D m_endLoc;
    FCoord m_radius{0};
};

class ECAD_API EText : public EPrimitive, public IText
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EText();
public:
    EText(std::string text, ELayerId layer, ENetId net = noNet);
    explicit EText(std::string text);
    virtual ~EText();

    ///Copy
    EText(const EText & other);
    EText & operator= (const EText & other);

    const std::string & GetText() const override;
    EPoint2D GetPosition() const override;
protected:
    ///Copy
    virtual Ptr<EText> CloneImp() const override { return new EText(*this); }
    ///Transform
    virtual ETransform2D & GetTransformImp() override { return m_transform; }
    virtual const ETransform2D & GetTransformImp() const override { return m_transform; }
    
protected:
    std::string m_text;
    ETransform2D m_transform;
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPrimitive)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EGeometry2D)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EText)