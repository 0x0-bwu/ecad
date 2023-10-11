#pragma once
#include "interfaces/IPadstackInst.h"
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

    ///Copy
    EPadstackInst(const EPadstackInst & other);
    EPadstackInst & operator= (const EPadstackInst & other);

    void SetNet(ENetId net);
    ENetId GetNet() const;

    void SetLayerRange(ELayerId top, ELayerId bot);
    void GetLayerRange(ELayerId & top, ELayerId & bot) const;

    /**
     * @brief layer id map b/w design stackup layers and padstackup layers
     * 
     * @param layerMap design layer id -> pad layer id
     */
    void SetLayerMap(CPtr<ILayerMap> layerMap);
    CPtr<ILayerMap> GetLayerMap() const;

    CPtr<IPadstackDef> GetPadstackDef() const;

    bool isLayoutPin() const;
    void SetIsLayoutPin(bool isPin);

    /**
     * @brief get shape at stackup layer
     */
    UPtr<EShape> GetLayerShape(ELayerId lyr) const;

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
