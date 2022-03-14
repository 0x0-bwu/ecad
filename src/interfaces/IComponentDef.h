#ifndef ECAD_ICOMPONENTDEF_H
#define ECAD_ICOMPONENTDEF_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
#include <string>
namespace ecad {
class ICell;
class ECAD_API IComponentDef : public Clonable<IComponentDef>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponentDef() = default;
    virtual const std::string & GetName() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponentDef)
#endif//ECAD_ICOMPONENTDEF_H