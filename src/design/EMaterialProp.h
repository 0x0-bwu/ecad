#pragma once
#include "interface/IMaterialProp.h"
#include <vector>
#include <map>

///@note for temperature dependent material property, the index unit use Kelvin

namespace ecad {

enum class EMatDependency { None, Temperature/*degC*/, Frequency/*Hz*/ };

class ECAD_API EMaterialProp : public IMaterialProp
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    virtual ~EMaterialProp() = default;
    virtual bool isPropValue() const override { return false;  } 
    virtual bool isPropTable() const override { return false; }
    virtual bool isPropPolynomial() const override { return false; }
    virtual Ptr<IMaterialPropValue> GetPropValue() override { ECAD_ASSERT(false) return nullptr; }
    virtual Ptr<IMaterialPropTable> GetPropTable() override { ECAD_ASSERT(false) return nullptr; }
    virtual Ptr<IMaterialPropPolynomial> GetPropPolynomial() override { ECAD_ASSERT(false) return nullptr; }
};

class ECAD_API EMaterialPropValue : public EMaterialProp, public IMaterialPropValue
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EMaterialPropValue();
public:
    explicit EMaterialPropValue(const std::array<EFloat, 9> & values);
    explicit EMaterialPropValue(const std::array<EFloat, 3> & values);
    explicit EMaterialPropValue(EFloat value);
    ~EMaterialPropValue() = default;

    bool isPropValue() const override { return true;  } 
    Ptr<IMaterialPropValue> GetPropValue() override;

    void SetSimpleProperty(const EFloat & value) override;
    void SetAnsiotropicProerty(const std::array<EFloat, 3> & values) override;
    void SetTensorProperty(const std::array<EFloat, 9> & values) override;

    bool GetSimpleProperty(EFloat & value) const override;
    bool GetAnsiotropicProperty(size_t row, EFloat & value) const override;
    bool GetTensorProperty(size_t row, size_t col, EFloat & value) const override;

    bool GetSimpleProperty(EFloat index, EFloat & value) const override;
    bool GetAnsiotropicProperty(EFloat index, size_t row, EFloat & value) const override;
    bool GetTensorProperty(EFloat index, size_t row, size_t col, EFloat & value) const override;

    //1x1-simple, 3x1-anisotropic, 3x3-tensor
    void GetDimensions(size_t & row, size_t & col) const override;

protected:
    ///Copy
    virtual Ptr<EMaterialPropValue> CloneImp() const override { return new EMaterialPropValue(*this); }

private:
    std::vector<EFloat> m_values;
};

class ECAD_API EMaterialPropTable : public EMaterialProp, public IMaterialPropTable //todo refine
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EMaterialPropTable();
public:
    EMaterialPropTable(EMatDependency dependency);
    ~EMaterialPropTable() = default;
    
    ///Copy
    EMaterialPropTable(const EMaterialPropTable & other);
    EMaterialPropTable & operator= (const EMaterialPropTable & other);

    bool isPropTable() const override { return true;  }
    Ptr<IMaterialPropTable> GetPropTable() override;

    bool GetSimpleProperty(EFloat index, EFloat & value) const override { ECAD_ASSERT(false/*todo*/) return false; }
    bool GetAnsiotropicProperty(EFloat index, size_t row, EFloat & value) const override { ECAD_ASSERT(false/*todo*/) return false; }
    bool GetTensorProperty(EFloat index, size_t row, size_t col, EFloat & value) const override { ECAD_ASSERT(false/*todo*/) return false; }

protected:
    ///Copy
    virtual Ptr<EMaterialPropTable> CloneImp() const override { return new EMaterialPropTable(*this); }

private:
    EMatDependency m_dependency;
    std::map<EFloat, UPtr<IMaterialProp> > m_values;
};

class ECAD_API EMaterialPropPolynomial : public EMaterialProp, public IMaterialPropPolynomial
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EMaterialPropPolynomial();
public:
    explicit EMaterialPropPolynomial(std::vector<std::vector<EFloat>> coefficients);
    ~EMaterialPropPolynomial() = default;

    bool isPropPolynomial() const override { return true;  } 
    Ptr<IMaterialPropPolynomial> GetPropPolynomial() override;

    bool GetSimpleProperty(EFloat index, EFloat & value) const override;
    bool GetAnsiotropicProperty(EFloat index, size_t row, EFloat & value) const override;
    bool GetTensorProperty(EFloat index, size_t row, size_t col, EFloat & value) const override;

    //1x1-simple, 3x1-anisotropic, 3x3-tensor
    void GetDimensions(size_t & row, size_t & col) const override;
    
protected:
    ///Copy
    virtual Ptr<EMaterialPropPolynomial> CloneImp() const override { return new EMaterialPropPolynomial(*this); }

    static EFloat Calculate(const std::vector<EFloat> & coefficients, EFloat index);
private:
    std::vector<std::vector<EFloat> > m_coefficients;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialProp)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialPropValue)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialPropTable)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialPropPolynomial)