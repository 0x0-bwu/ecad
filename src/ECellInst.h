#pragma once
#include "interfaces/ICellInst.h"
#include "EHierarchyObj.h"
namespace ecad {

class ICell;
class ILayerMap;
class ILayoutView;
class ECAD_API ECellInst : public EHierarchyObj, public ICellInst
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    ECellInst();
public:
    ECellInst(std::string name, CPtr<ILayoutView> refLayout, CPtr<ILayoutView> defLayout);
    virtual ~ECellInst();

    void SetName(std::string name) override;
    const std::string & GetName() const override;
    
    void SetRefLayoutView(CPtr<ILayoutView> refLayout) override;
    CPtr<ILayoutView> GetRefLayoutView() const override;

    void SetDefLayoutView(CPtr<ILayoutView> defLayout) override;
    CPtr<ILayoutView> GetDefLayoutView() const override;

    CPtr<ILayoutView> GetFlattenedLayoutView() const override;

    void SetLayerMap(CPtr<ILayerMap> layerMap) override;
    CPtr<ILayerMap> GetLayerMap() const override;

protected:
    ///Copy
    virtual Ptr<ECellInst> CloneImp() const override { return new ECellInst(*this); }
    ///Transform
    virtual ETransform2D & GetTransformImp() override { return EHierarchyObj::GetTransformImp(); }
    virtual const ETransform2D & GetTransformImp() const override { return EHierarchyObj::GetTransformImp(); }
private:
    CPtr<ILayoutView> m_defLayout{nullptr};
    CPtr<ILayerMap> m_layerMap{nullptr};
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECellInst)