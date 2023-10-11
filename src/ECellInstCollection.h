#pragma once
#include "interfaces/ICellInstCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
namespace ecad {

class ICellInst;
using ECellInstIterator = EVectorCollectionIterator<UPtr<ICellInst> >;
class ECAD_API ECellInstCollection : public EVectorCollection<UPtr<ICellInst> >, public ICellInstCollection
{
    using BaseCollection = EVectorCollection<UPtr<ICellInst> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECellInstCollection();
    virtual ~ECellInstCollection();

    ///Copy
    ECellInstCollection(const ECellInstCollection & other);
    ECellInstCollection & operator= (const ECellInstCollection & other);

    Ptr<ICellInst> AddCellInst(UPtr<ICellInst> cellInst);

    Ptr<ICellInst> CreateCellInst(Ptr<ILayoutView> reflayout, const std::string & name,
                                  Ptr<ILayoutView> defLayout, const ETransform2D & transform);

    CellInstIter GetCellInstIter() const;
    size_t Size() const;
    void Clear();
    
protected:
    ///Copy
    virtual Ptr<ECellInstCollection> CloneImp() const override { return new ECellInstCollection(*this); }
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECellInstCollection)