#ifndef ECAD_IHIERARCHYOBJCOLLECTION_H
#define ECAD_IHIERARCHYOBJCOLLECTION_H
#include "ECadCommon.h"
#include "IIterator.h"
#include "Protocol.h"
namespace ecad {

class ICellInstCollection;
class ECAD_API IHierarchyObjCollection : public Clonable<IHierarchyObjCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IHierarchyObjCollection() = default;

    virtual Ptr<ICellInstCollection> GetCellInstCollection() const = 0;
    virtual HierarchyObjIter GetHierarchyObjIter() const = 0;
    virtual size_t Size() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IHierarchyObjCollection)
#endif//ECAD_IHIERARCHYOBJCOLLECTION_H