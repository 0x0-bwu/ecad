#pragma once
#include "interface/INet.h"
#include "EObject.h"
namespace ecad {

class ECAD_API ENet : public EObject, public INet
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ENet();
    ENet(std::string name);
    virtual ~ENet();

    void SetNetId(ENetId id) override;
    ENetId GetNetId() const override;

    void SetName(std::string name) override;
    const std::string & GetName() const override;
    const EUuid & Uuid() const override;
protected:
    ///Copy
    Ptr<ENet> CloneImp() const override;
protected:
    ENetId m_netId;
};

ECAD_ALWAYS_INLINE void ENet::SetName(std::string name)
{
    return EObject::SetName(std::move(name));
}

ECAD_ALWAYS_INLINE const std::string & ENet::GetName() const
{
    return EObject::GetName();
}

ECAD_ALWAYS_INLINE const EUuid & ENet::Uuid() const
{
    return EObject::Uuid();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ENet)