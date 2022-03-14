#ifndef ECAD_ECOMPONENTDEFPIN_HPP
#define ECAD_ECOMPONENTDEFPIN_HPP
#include "interfaces/IComponentDefPin.h"
#include "EObject.h"
namespace ecad {

class ECAD_API EComponentDefPin : public EObject, public IComponentDefPin
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EComponentDefPin();
public:
    EComponentDefPin(const std::string & name);
    virtual ~EComponentDefPin();

    ///Copy
    EComponentDefPin(const EComponentDefPin & other);
    EComponentDefPin & operator= (const EComponentDefPin & other);

    const std::string & GetName() const;

protected:
    ///Copy
    virtual Ptr<EComponentDefPin> CloneImp() const override { return new EComponentDefPin(*this); }

protected:
};

ECAD_ALWAYS_INLINE const std::string & EComponentDefPin::GetName() const
{
    return EObject::GetName();
}

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponentDefPin)

#ifdef ECAD_HEADER_ONLY
#include "EComponentDefPin.cpp"
#endif

#endif//ECAD_ECOMPONENTDEFPIN_HPP