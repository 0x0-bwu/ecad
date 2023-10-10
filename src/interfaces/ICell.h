#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
#include <string>
namespace ecad {

class IDatabase;
class ILayoutView;
class ECAD_API ICell : public Clonable<ICell>, public Printable
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ICell() = default;
    virtual const std::string & GetName() const = 0;
    virtual std::string sUuid() const = 0;
    virtual void Print(std::ostream & os) const = 0;

    virtual const ECoordUnits & GetCoordUnits() const = 0;

    virtual void SetDatabase(Ptr<IDatabase> database) = 0;
    virtual bool SetLayoutView(UPtr<ILayoutView> layout) = 0;

    virtual ECellType GetCellType() const = 0;
    virtual Ptr<IDatabase> GetDatabase() const = 0;
    virtual Ptr<ILayoutView> GetLayoutView() const = 0;
    virtual Ptr<ILayoutView> GetFlattenedLayoutView() = 0;
};

}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ICell)