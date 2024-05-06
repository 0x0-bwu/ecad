#pragma once
#include "interface/IHierarchyObjCollection.h"
#include "interface/IIterator.h"
#include "ECollectionCollection.h"
#include <vector>
#include <array>
namespace ecad {

class ECAD_API EHierarchyObjCollection : public ECollectionCollection, public IHierarchyObjCollection
{
    friend class EHierarchyObjIterator;
    ECAD_ALWAYS_INLINE static constexpr std::array<ECollectionType, 2> m_collectionTypes = { ECollectionType::CellInst,
                                                                                             ECollectionType::Component };
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EHierarchyObjCollection();
    virtual ~EHierarchyObjCollection();

    ///Copy
    EHierarchyObjCollection(const EHierarchyObjCollection & other);
    EHierarchyObjCollection & operator= (const EHierarchyObjCollection & other);

    Ptr<ICellInstCollection> GetCellInstCollection() const override;
    Ptr<IComponentCollection> GetComponentCollection() const override;

    HierarchyObjIter GetHierarchyObjIter() const override;
    size_t Size() const override;
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
    ComponentIter m_componentIter;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EHierarchyObjCollection)