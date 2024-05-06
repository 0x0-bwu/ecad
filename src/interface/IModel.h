#pragma once
#include "basic/ECadSettings.h"
namespace ecad {
class ECAD_API IModel : public Clonable<IModel>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IModel() = default;
    virtual EModelType GetModelType() const = 0;
    virtual bool Match(const ECadSettings & settings) const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IModel)