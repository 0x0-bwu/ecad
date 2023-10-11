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

    Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type);
    Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const;
    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition);
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const;

    MaterialDefIter GetMaterialDefIter() const;
    size_t Size() const;
    void Clear();

protected:
    ///Copy
    virtual Ptr<EMaterialDefCollection> CloneImp() const override { return new EMaterialDefCollection(*this); }
};
}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialDefCollection)