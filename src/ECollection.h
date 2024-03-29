#pragma once
#include "interfaces/ICollection.h"
#include "ECadDef.h"
#include <string>
namespace ecad {

class ECAD_API ECollection : public ICollection
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECollection();
    virtual ~ECollection();

    ///Copy
    ECollection(const ECollection & other);
    ECollection & operator= (const ECollection & other);

    virtual size_t Size() const override;

protected:
    ///Copy
    virtual Ptr<ECollection> CloneImp() const override { return new ECollection(*this); }
    virtual void PrintImp(std::ostream & os) const override;

protected:
    ECollectionType m_type;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECollection)