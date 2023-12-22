#pragma once
#include "ECadCommon.h"
#include "Protocol.h"
#include <string>
namespace ecad {
class IMaterialPropValue;
class IMaterialPropTable;
class ECAD_API IMaterialProp : public Clonable<IMaterialProp>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IMaterialProp() = default;
    virtual bool isPropValue() const = 0;
    virtual bool isPropTable() const = 0;
    virtual Ptr<IMaterialPropValue> GetPropValue() = 0;
    virtual Ptr<IMaterialPropTable> GetPropTable() = 0;
    virtual bool GetSimpleProperty(EValue index, EValue & value) const = 0;
    virtual bool GetAnsiotropicProperty(EValue index, size_t row, EValue & value) const = 0;
    virtual bool GetTensorProperty(EValue index, size_t row, size_t col, EValue & value) const = 0;
};

class ECAD_API IMaterialPropValue
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IMaterialPropValue() = default;

    virtual void SetSimpleProperty(const EValue & value) = 0;
    virtual void SetAnsiotropicProerty(const std::array<EValue, 3> & values) = 0;
    virtual void SetTensorProperty(const std::array<EValue, 9> & values) = 0;

    virtual bool GetSimpleProperty(EValue & value) const = 0;
    virtual bool GetAnsiotropicProperty(size_t row, EValue & value) const = 0;
    virtual bool GetTensorProperty(size_t row, size_t col, EValue & value) const = 0;

    //1x1-simple, 3x1-anisotropic, 3x3-tensor
    virtual void GetDimensions(size_t & row, size_t & col) const = 0;
};

class ECAD_API IMaterialPropTable
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IMaterialPropTable() = default;
};

}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IMaterialProp)