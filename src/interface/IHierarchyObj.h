#pragma once
#include "basic/ECadCommon.h"
namespace ecad {
    
class ECAD_API IHierarchyObj : public Printable, public Transformable2D
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IHierarchyObj() = default;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IHierarchyObj)