#pragma once
#include "interfaces/IHierarchyObjCollection.h"
#include "interfaces/IIterator.h"
#include "ECollectionCollection.h"
#include "ECadDef.h"
#include <vector>
#include <array>
namespace ecad {

class ICollectionCollection;
class ECAD_API EHierarchyObjCollection : public ECollectionCollection, public IHierarchyObjCollection
{
    friend class EHierarchyObjIterator;
    ECAD_ALWAYS_INLINE static constexpr std::array<ECollectionType, 1> m_collectionTypes = { ECollectionType::CellInst };
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EHierarchyObjCollection();
    virtual ~EHierarchyObjCollection();

    ///Copy
    EHierarchyObjCollection(const EHierarchyObjCollection & other);
    EHierarchyObjCollection & operator= (const EHierarchyObjCollection & other);

    Ptr<ICellInstCollection> GetCellInstCollection() const;

    HierarchyObjIter GetHierarchyObjIter() const;
    size_t Size() const;
protected:
    ///Copy
    virtual Ptr<EHierarchyObjCollection> CloneImp() const override { return new EHierarchyObjCollection(*this); }
};

class ECAD_API EHierarchyObjIterator : public IIterator<IHierarchyObj>
{
public:
    explicit EHierarchyObjIterator(const EHierarchyObjCollection & collection);
    virtual ~EHierarchyObjIterator();

    EHierarchyObjIterator(const EHierarchyObjIterator & other);
    EHierarchyObjIterator & operator= (const EHierarchyObjIterator & other);
    EHierarchyObjIterator(EHierarchyObjIterator && other);
    EHierarchyObjIterator & operator= (EHierarchyObjIterator && other);

    Ptr<IHierarchyObj> Next();
    Ptr<IHierarchyObj> Current();
    HierarchyObjIter Clone() const;

private:
    void Move(EHierarchyObjIterator && other);

private:
    CellInstIter m_cellInstIter;
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EHierarchyObjCollection)

#ifdef ECAD_HEADER_ONLY
#include "EHierarchyObjCollection.cpp"
#endif