#pragma once
#include "interfaces/IDefinitionCollection.h"
#include "ECollectionCollection.h"

namespace ecad {

class ECAD_API EDefinitionCollection : public ECollectionCollection, public IDefinitionCollection
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EDefinitionCollection();
    explicit EDefinitionCollection(std::string name);
    virtual ~EDefinitionCollection();

    ///Copy
    EDefinitionCollection(const EDefinitionCollection & other);
    EDefinitionCollection & operator= (const EDefinitionCollection & other);

    virtual Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type) override;
    virtual Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const override;
    virtual Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition) override;
    virtual Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const override;
    virtual std::string GetNextDefName(const std::string & name, EDefinitionType type) const override;
    virtual size_t Size() const override;
protected:
    ///Copy
    virtual Ptr<EDefinitionCollection> CloneImp() const override { return new EDefinitionCollection(*this); }
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EDefinitionCollection)
