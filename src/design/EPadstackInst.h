#pragma once
#include "interface/IPadstackInst.h"
#include "EConnObj.h"
namespace ecad {

class EShape;
class ILayerMap;
class IPadstackDef;
class ECAD_API EPadstackInst : public EConnObj, public IPadstackInst
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EPadstackInst();
public:
    EPadstackInst(std::string name, CPtr<IPadstackDef> def, ENetId net);
    virtual ~EPadstackInst();

    void SetNet(ENetId net) override;
    ENetId GetNet() const override;

    void SetLayerRange(ELayerId top, ELayerId bot) override;
    void GetLayerRange(ELayerId & top, ELayerId & bot) const override;

    /**
     * @brief layer id map b/w design stackup layers and padstackup layers
     * 
     * @param layerMap design layer id -> pad layer id
     */
    void SetLayerMap(CPtr<ILayerMap> layerMap) override;
    CPtr<ILayerMap> GetLayerMap() const override;

    CPtr<IPadstackDef> GetPadstackDef() const override;

    bool isLayoutPin() const override;
    void SetIsLayoutPin(bool isPin) override;

    /**
     * @brief get shape at stackup layer
     */
    UPtr<EShape> GetLayerShape(ELayerId lyr) const override;

protected:
    ///Copy
    virtual Ptr<EPadstackInst> CloneImp() const override { return new EPadstackInst(*this); }
    ///Transform
    virtual ETransform2D & GetTransformImp() override { return m_transform; }
    virtual const ETransform2D & GetTransformImp() const override { return m_transform; }
protected:
    CPtr<IPadstackDef> m_def;
    CPtr<ILayerMap> m_layerMap;
    ELayerId m_topLyr, m_botLyr;
    ETransform2D m_transform;
    bool m_isLayoutPin = false;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPadstackInst)
