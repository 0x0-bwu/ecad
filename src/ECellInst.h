#pragma once
#include "interfaces/ICellInst.h"
#include "EHierarchyObj.h"
namespace ecad {

class ICell;
class ILayoutView;
class ECAD_API ECellInst : public EHierarchyObj, public ICellInst
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    ECellInst();
public:
    ECellInst(std::string name, CPtr<ILayoutView> refLayout, CPtr<ILayoutView> defLayout);
    virtual ~ECellInst();

    ///Copy
    ECellInst(const ECellInst & other);
    ECellInst & operator= (const ECellInst & other);

    void SetRefLayoutView(CPtr<ILayoutView> refLayout);
    CPtr<ILayoutView> GetRefLayoutView() const;

    void SetDefLayoutView(CPtr<ILayoutView> defLayout);
    CPtr<ILayoutView> GetDefLayoutView() const;

    CPtr<ILayoutView> GetFlattenedLayoutView() const;

protected:
    ///Copy
    virtual Ptr<ECellInst> CloneImp() const override { return new ECellInst(*this); }
    ///Transform
    virtual ETransform2D & GetTransformImp() override { return EHierarchyObj::GetTransformImp(); }
    virtual const ETransform2D & GetTransformImp() const override { return EHierarchyObj::GetTransformImp(); }
private:
    CPtr<ILayoutView> m_refLayout;
    CPtr<ILayoutView> m_defLayout;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECellInst)