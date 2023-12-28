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
    EBondwire(std::string name, ENetId net, EPoint2D start, EPoint2D end, EFloat radius);
    virtual ~EBondwire() = default;

    const std::string & GetName() const override;

    void SetNet(ENetId net) override;
    ENetId GetNet() const override;

    void SetRadius(EFloat r) override;
    EFloat GetRadius() const override;

    const EPoint2D & GetStartPt() const override;
    const EPoint2D & GetEndPt() const override;

    void SetStartLayer(ELayerId layerId, bool flipped) override;
    ELayerId GetStartLayer(Ptr<bool> flipped) const override;

    void SetEndLayer(ELayerId layerId, bool flipped) override;
    ELayerId GetEndLayer(Ptr<bool> flipped) const override;

    void SetMaterial(const std::string & material) override;
    const std::string & GetMaterial() const override;

    void SetBondwireType(EBondwireType type) override;
    EBondwireType GetBondwireType() const override;

    void SetHeight(EFloat height) override;
    EFloat GetHeight() const override;

    void SetStartComponent(CPtr<IComponent> comp) override;
    CPtr<IComponent> GetStartComponent() const override;

    void SetEndComponent(CPtr<IComponent> comp) override;
    CPtr<IComponent> GetEndComponent() const override;

    void SetSolderJoints(CPtr<IPadstackDef> s) override;
    CPtr<IPadstackDef> GetSolderJoints() const override;

    void SetCurrent(EFloat current) override;
    EFloat GetCurrent() const override;

    void Transform(const ETransform2D & transform) override;

protected:
    virtual Ptr<EBondwire> CloneImp() const override { return new EBondwire(*this); }
    virtual void PrintImp(std::ostream & os) const override;

protected:
    std::string m_material;
    ELayerId m_endLayer{ELayerId::noLayer};
    std::array<CPtr<IComponent>, 2> m_mountComp{nullptr, nullptr};
    std::array<EPoint2D, 2> m_location;
    std::array<bool, 2> m_flipped{false, false};
    EFloat m_radius{0};
    EFloat m_height{0};
    EFloat m_current{0};
    CPtr<IPadstackDef> m_solderJoints{nullptr};
    EBondwireType m_bondwireType{EBondwireType::Simple};
};

class ECAD_API EText : public EPrimitive, public IText
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EText();
public:
    EText(std::string text, ELayerId layer, ENetId net = noNet);
    explicit EText(std::string text);
    virtual ~EText();

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