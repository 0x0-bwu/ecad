#pragma once
#include "interface/IPadstackDefCollection.h"
#include "interface/IDefinitionCollection.h"
#include "interface/IIterator.h"
#include "basic/ECollection.h"
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

    Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type) override;
    Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const override;
    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition) override;
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const override;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const override;

    PadstackDefIter GetPadstackDefIter() const override;
    size_t Size() const override;
    void Clear() override;

protected:
    ///Copy
    virtual Ptr<EPadstackDefCollection> CloneImp() const override { return new EPadstackDefCollection(*this); }
};
}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPadstackDefCollection)
