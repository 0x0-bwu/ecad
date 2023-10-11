#pragma once
#include "interfaces/INet.h"
#include "EObject.h"
namespace ecad {

class ECAD_API ENet : public EObject, public INet
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ENet();
    ENet(std::string name);
    virtual ~ENet();

    ///Copy
    ENet(const ENet & other);
    ENet & operator= (const ENet & other);

    void SetNetId(ENetId id);
    ENetId GetNetId() const;

    void SetName(std::string name);
    const std::string & GetName() const;
    const EUuid & Uuid() const;
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