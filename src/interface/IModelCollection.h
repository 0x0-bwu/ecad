#pragma once
#include "basic/ECadCommon.h"
#include "IIterator.h"
namespace ecad {
class IModel;
class ECAD_API IModelCollection : public Clonable<IModelCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IModelCollection() = default;
    virtual CPtr<IModel> FindModel(EModelType type) const = 0;
    virtual bool AddModel(UPtr<IModel> model) = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IModelCollection)