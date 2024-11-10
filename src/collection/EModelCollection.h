#pragma once
#include "interface/IModelCollection.h"
#include "interface/IIterator.h"
#include "basic/ECollection.h"
namespace ecad {

class IModel;
using EModelIterator = EUnorderedMapCollectionIterator<EModelType, UPtr<IModel> >;
class ECAD_API EModelCollection
 : public EUnorderedMapCollection<EModelType, UPtr<IModel> > 
 , public IModelCollection
{
    using BaseCollection = EUnorderedMapCollection<EModelType, UPtr<IModel> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EModelCollection();
    virtual ~EModelCollection();

    ///Copy
    EModelCollection(const EModelCollection & other);
    EModelCollection & operator= (const EModelCollection & other);

    CPtr<IModel> FindModel(EModelType type) const override;
    bool AddModel(UPtr<IModel> model) override;

    size_t Size() const override;
    void Clear() override;

protected:
    virtual ECollectionType GetType() const override { return ECollectionType::Model; }
    virtual Ptr<EModelCollection> CloneImp() const override { return new EModelCollection(*this); }
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EModelCollection)