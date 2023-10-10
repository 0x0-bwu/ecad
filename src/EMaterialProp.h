#pragma once
#include "interfaces/IMaterialProp.h"
#include <vector>
#include <map>
namespace ecad {

enum class EMatDependency { None, Temperature/*degC*/, Frequency/*Hz*/ };

class ECAD_API EMaterialProp : public IMaterialProp
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EMaterialProp() = default;
    virtual ~EMaterialProp() = default;
    virtual Ptr<IMaterialPropValue> GetPropValue() { return nullptr; }
    virtual Ptr<IMaterialPropTable> GetPropTable() { return nullptr; }
};

class ECAD_API EMaterialPropValue : public EMaterialProp, public IMaterialPropValue
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EMaterialPropValue();
    explicit EMaterialPropValue(EValue value);
    ~EMaterialPropValue() = default;

    bool isPropValue() const { return true;  } 
    bool isPropTable() const { return false; }
    Ptr<IMaterialPropValue> GetPropValue();

    void SetSimpleProperty(const EValue & value);
    void SetAnsiotropicProerty(const std::array<EValue, 3> & values);
    void SetTensorProperty(const std::array<EValue, 9> & values);

    bool GetSimpleProperty(EValue & value) const;
    bool GetAnsiotropicProperty(size_t row, EValue & value) const;
    bool GetTensorProperty(size_t row, size_t col, EValue & value) const;

    //1x1-simple, 3x1-anisotropic, 3x3-tensor
    void GetDimensions(size_t & row, size_t & col) const;;

protected:
    ///Copy
    virtual Ptr<EMaterialPropValue> CloneImp() const override { return new EMaterialPropValue(*this); }

private:
    std::vector<EValue> m_values;
};

class ECAD_API EMaterialPropTable : public EMaterialProp, public IMaterialPropTable
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EMaterialPropTable();
public:
    EMaterialPropTable(EMatDependency dependency);
    ~EMaterialPropTable() = default;
    
    ///Copy
    EMaterialPropTable(const EMaterialPropTable & other);
    EMaterialPropTable & operator= (const EMaterialPropTable & other);

    bool isPropValue() const { return false; } 
    bool isPropTable() const { return true;  }
    Ptr<IMaterialPropTable> GetPropTable() override;

protected:
    ///Copy
    virtual Ptr<EMaterialPropTable> CloneImp() const override { return new EMaterialPropTable(*this); }

private:
    EMatDependency m_dependency;
    std::map<EValue, UPtr<IMaterialProp> > m_values;
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialProp)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialPropValue)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialPropTable)

#ifdef ECAD_HEADER_ONLY
#include "EMaterialProp.cpp"
#endif