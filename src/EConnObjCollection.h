#pragma once
#include "interfaces/IConnObjCollection.h"
#include "interfaces/IIterator.h"
#include "ECollectionCollection.h"
#include "ECadDef.h"
#include <vector>
#include <array>
namespace ecad {

class ICollectionCollection;
class ECAD_API EConnObjCollection : public ECollectionCollection, public IConnObjCollection
{
    friend class EConnObjIterator;
    ECAD_ALWAYS_INLINE static constexpr std::array<ECollectionType, 2> m_collectionTypes = { ECollectionType::Primitive,
                                                                                             ECollectionType::PadstackInst };
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EConnObjCollection();
    virtual ~EConnObjCollection();

    ///Copy
    EConnObjCollection(const EConnObjCollection & other);
    EConnObjCollection & operator= (const EConnObjCollection & other);

    Ptr<IPrimitiveCollection> GetPrimitiveCollection() const;
    Ptr<IPadstackInstCollection> GetPadstackInstCollection() const;

    ConnObjIter GetConnObjIter() const;
    size_t Size() const;
    
protected:
    ///Copy
    virtual Ptr<EConnObjCollection> CloneImp() const override { return new EConnObjCollection(*this); }
};

class ECAD_API EConnObjIterator : public IIterator<IConnObj>
{
public:
    explicit EConnObjIterator(const EConnObjCollection & collection);
    virtual ~EConnObjIterator();

    EConnObjIterator(const EConnObjIterator & other);
    EConnObjIterator & operator= (const EConnObjIterator & other);
    EConnObjIterator(EConnObjIterator && other);
    EConnObjIterator & operator= (EConnObjIterator && other);

    Ptr<IConnObj> Next();
    Ptr<IConnObj> Current();
    ConnObjIter Clone() const;

private:
    void Move(EConnObjIterator && other);

private:
    PrimitiveIter m_primitiveIter;
    PadstackInstIter m_padstackInstIter;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EConnObjCollection)