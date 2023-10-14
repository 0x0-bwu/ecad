#pragma once
#include "interfaces/ICollection.h"
#include "EObject.h"
#include "ECadDef.h"
#include <string>
namespace ecad {

class ECAD_API ECollection : public EObject, public ICollection
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECollection();
    explicit ECollection(std::string name);
    virtual ~ECollection();

    ///Copy
    ECollection(const ECollection & other);
    ECollection & operator= (const ECollection & other);

    virtual size_t Size() const override;

protected:
    ///Copy
    virtual Ptr<ECollection> CloneImp() const override { return new ECollection(*this); }
    
protected:
    ECollectionType m_type;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECollection)