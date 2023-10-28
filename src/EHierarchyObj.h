#pragma once
#include "interfaces/IHierarchyObj.h"
#include "ETransform.h"
#include "EObject.h"
#include "ECadDef.h"
namespace ecad {

class ECAD_API EHierarchyObj : public EObject, public IHierarchyObj
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EHierarchyObj();
    explicit EHierarchyObj(std::string name);
    virtual ~EHierarchyObj();

    ///Copy
    EHierarchyObj(const EHierarchyObj & other);
    EHierarchyObj & operator= (const EHierarchyObj & other);

protected:
    ///Transform
    virtual ETransform2D & GetTransformImp() override { return m_transform; }
    virtual const ETransform2D & GetTransformImp() const override { return m_transform; }
    virtual void PrintImp(std::ostream & os) const override;

protected:
    ETransform2D m_transform;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EHierarchyObj)