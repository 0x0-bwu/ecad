#pragma once
#include "ECadCommon.h"
#include "ECadSerialization.h"


namespace ecad {

class INet;
class ICell;
class ILayer;
class IConnObj;
class ICellInst;
class ILayerMap;
class IPrimitive;
class IMaterialDef;
class IPadstackDef;
class IComponentDef;
class IHierarchyObj;
class IPadstackInst;
class IComponentDefPin;
template <typename T>
class ECAD_API IIterator
{
public:
    virtual ~IIterator() = default;
    
    virtual Ptr<T> Next() = 0;
    virtual Ptr<T> Current() = 0;
    virtual UPtr<IIterator<T> > Clone() const = 0;
};

using NetIter = UPtr<IIterator<INet> >;
using CellIter = UPtr<IIterator<ICell> >;
using LayerIter = UPtr<IIterator<ILayer> >;
using ConnObjIter = UPtr<IIterator<IConnObj> >;
using CellInstIter = UPtr<IIterator<ICellInst> >;
using LayerMapIter = UPtr<IIterator<ILayerMap> >;
using PrimitiveIter = UPtr<IIterator<IPrimitive> >;
using MaterialDefIter = UPtr<IIterator<IMaterialDef> >;
using PadstackDefIter = UPtr<IIterator<IPadstackDef> >;
using ComponentDefIter = UPtr<IIterator<IComponentDef> >;
using HierarchyObjIter = UPtr<IIterator<IHierarchyObj> >;
using PadstackInstIter = UPtr<IIterator<IPadstackInst> >;
using ComponentDefPinIter = UPtr<IIterator<IComponentDefPin> >;
}//namespace ecad