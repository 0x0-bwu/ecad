#ifndef ECAD_IMATERIALDEFCOLLECTION_H
#define ECAD_IMATERIALDEFCOLLECTION_H
#include "ECadCommon.h"
#include "IIterator.h"
#include "Protocol.h"
namespace ecad {
class IMaterialDef;
class ECAD_API IMaterialDefCollection : public Clonable<IMaterialDefCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IMaterialDefCollection() = default;
    virtual MaterialDefIter GetMaterialDefIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IMaterialDefCollection)
#endif//ECAD_IMATERIALDEFCOLLECTION_H