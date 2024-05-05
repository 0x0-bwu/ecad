#pragma once
#include "interface/IConnObj.h"
#include "EObject.h"
namespace ecad {

class ECAD_API EConnObj : public EObject, public IConnObj
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EConnObj();
    explicit EConnObj(std::string name, ENetId net = noNet);
    virtual ~EConnObj();

    void SetNet(ENetId net) override;
    ENetId GetNet() const override;

protected:
    ///Copy
    virtual Ptr<EConnObj> CloneImp() const override { return new EConnObj(*this); }

protected:
    ENetId m_net;
};

ECAD_ALWAYS_INLINE void EConnObj::SetNet(ENetId net)
{
    m_net = net;
}

ECAD_ALWAYS_INLINE ENetId EConnObj::GetNet() const
{
    return m_net;
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EConnObj)