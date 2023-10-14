#pragma once
#include "interfaces/IMaterialDefCollection.h"
#include "interfaces/IDefinitionCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
namespace ecad {

class IMaterialDef;
using EMaterialDefIterator = EUnorderedMapCollectionIterator<std::string, UPtr<IMaterialDef> >;
class ECAD_API EMaterialDefCollection
 : public EUnorderedMapCollection<std::string, UPtr<IMaterialDef> >
 , public IMaterialDefCollection
 , public IDefinitionCollection
{
    using BaseCollection = EUnorderedMapCollection<std::string, UPtr<IMaterialDef> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EMaterialDefCollection();
    virtual ~EMaterialDefCollection();

    ///Copy
    EMaterialDefCollection(const EMaterialDefCollection & other);
    EMaterialDefCollection & operator= (const EMaterialDefCollection & other);

    Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type) override;
    Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const override;
    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition) override;
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const override;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const override;

    MaterialDefIter GetMaterialDefIter() const override;
    size_t Size() const override;
    void Clear() override;

protected:
    ///Copy
    virtual Ptr<EMaterialDefCollection> CloneImp() const override { return new EMaterialDefCollection(*this); }
};
}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialDefCollection)