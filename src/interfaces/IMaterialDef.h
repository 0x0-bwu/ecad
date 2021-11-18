#ifndef ECAD_IMATERIALDEF_H
#define ECAD_IMATERIALDEF_H
#include "ECadCommon.h"
#include "Protocol.h"
#include <string>
namespace ecad {
class ECAD_API IMaterialDef : public Clonable<IMaterialDef>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IMaterialDef() = default;
    virtual const std::string & GetName() const = 0;
    virtual const EUuid & Uuid() const = 0;

};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IMaterialDef)
#endif//ECAD_IMATERIALDEF_H