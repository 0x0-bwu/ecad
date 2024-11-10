#pragma once
#include "interface/IDefinitionCollection.h"
#include "interface/ICellCollection.h"
#include "interface/IIterator.h"
#include "basic/ECollection.h"
namespace ecad {

class ICell;
using ECellIterator = EUnorderedMapCollectionIterator<std::string, UPtr<ICell> >;
class ECAD_API ECellCollection
 : public EUnorderedMapCollection<std::string, UPtr<ICell> > 
 , public ICellCollection
 , public IDefinitionCollection
{
    using BaseCollection = EUnorderedMapCollection<std::string, UPtr<ICell> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECellCollection();
    virtual ~ECellCollection();

    ///Copy
    ECellCollection(const ECellCollection & other);
    ECellCollection & operator= (const ECellCollection & other);

    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition) override;
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const override;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const override;

    void SetDatabase(CPtr<IDatabase> database) override;
    CellIter GetCellIter() const override;
    size_t Size() const override;
    void Clear() override;

protected:
    virtual ECollectionType GetType() const override { return ECollectionType::Cell; }
    virtual Ptr<ECellCollection> CloneImp() const override { return new ECellCollection(*this); }
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECellCollection)