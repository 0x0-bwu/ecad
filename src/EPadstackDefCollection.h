#pragma once
#include "interfaces/IPadstackDefCollection.h"
#include "interfaces/IDefinitionCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
namespace ecad {

class IPadstackDef;
using EPadstackDefIterator = EUnorderedMapCollectionIterator<std::string, UPtr<IPadstackDef> >;
class ECAD_API EPadstackDefCollection
 : public EUnorderedMapCollection<std::string, UPtr<IPadstackDef> >
 , public IPadstackDefCollection
 , public IDefinitionCollection
{
    using BaseCollection = EUnorderedMapCollection<std::string, UPtr<IPadstackDef> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPadstackDefCollection();
    virtual ~EPadstackDefCollection();

    ///Copy
    EPadstackDefCollection(const EPadstackDefCollection & other);
    EPadstackDefCollection & operator= (const EPadstackDefCollection & other);

    Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type);
    Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const;
    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition);
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const;

    PadstackDefIter GetPadstackDefIter() const;
    size_t Size() const;
    void Clear();

protected:
    ///Copy
    virtual Ptr<EPadstackDefCollection> CloneImp() const override { return new EPadstackDefCollection(*this); }
};
}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPadstackDefCollection)
