#pragma once
#include "ECadCommon.h"
#include "IIterator.h"
#include "Protocol.h"
#include "ECadDef.h"
namespace ecad {
class IMaterialDef;
class ECAD_API IMaterialDefCollection : public Clonable<IMaterialDefCollection>, public Printable
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IMaterialDefCollection() = default;
    virtual Ptr<IMaterialDef> FindMaterialDefById(EMaterialId id) const = 0;
    virtual MaterialDefIter GetMaterialDefIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IMaterialDefCollection)