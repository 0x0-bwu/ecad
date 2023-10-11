#pragma once
#include "ECadCommon.h"
#include "Protocol.h"
#include <string>
namespace ecad {

class IPadstackDefData;
class ECAD_API IPadstackDef : public Clonable<IPadstackDef>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IPadstackDef() = default;

    virtual const std::string & GetName() const = 0;
    virtual void SetPadstackDefData(UPtr<IPadstackDefData> data) = 0;
    virtual Ptr<IPadstackDefData> GetPadstackDefData() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IPadstackDef)