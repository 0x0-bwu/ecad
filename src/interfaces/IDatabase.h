#ifndef ECAD_IDATABASE_H
#define ECAD_IDATABASE_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include "IIterator.h"
#include "Protocol.h"
#include <string>
namespace ecad {

class ICell;
class ILayerMap;
class IDefinition;
class IMaterialDef;
class IPadstackDef;
class IComponentDef;
class ILayerMapCollection;
class IDefinitionCollection;
class IMaterialDefCollection;
class IPadstackDefCollection;
class IComponentDefCollection;
class ECAD_API IDatabase : public Clonable<IDatabase>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IDatabase() = default;
    virtual const std::string & GetName() const = 0;
    virtual std::string sUuid() const = 0;

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    virtual bool Save(const std::string & archive, EArchiveFormat fmt = EArchiveFormat::TXT) const = 0;
    virtual bool Load(const std::string & archive, EArchiveFormat fmt = EArchiveFormat::TXT) = 0;
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    virtual void SetCoordUnits(const ECoordUnits & u) = 0;
    virtual const ECoordUnits & GetCoordUnits() const = 0;

    virtual std::string GetNextDefName(const std::string & base, EDefinitionType type) const = 0;
    virtual Ptr<IDefinitionCollection> GetDefinitionCollection() = 0;

    virtual Ptr<ICell> CreateCircuitCell(const std::string & name) = 0;
    virtual Ptr<ICell> FindCellByName(const std::string & name) = 0;
    virtual bool GetCircuitCells(std::vector<Ptr<ICell> > & cells) const = 0;
    virtual bool GetTopCells(std::vector<Ptr<ICell> > & cells) const = 0;
    virtual bool Flatten(Ptr<ICell> cell) const = 0;

    virtual Ptr<IMaterialDef> CreateMaterialDef(const std::string & name) = 0;
    virtual Ptr<IMaterialDef> FindMaterialDefByName(const std::string & name) const = 0;

    virtual Ptr<IComponentDef> CreateComponentDef(const std::string & name) = 0;
    virtual Ptr<IComponentDef> FindComponentDefByName(const std::string & name) = 0;

    virtual Ptr<ILayerMapCollection> GetLayerMapCollection() const = 0;
    virtual Ptr<ILayerMap> CreateLayerMap(const std::string & name) = 0;
    virtual Ptr<ILayerMap> FindLayerMapByName(const std::string & name) const = 0;
    virtual bool AddLayerMap(UPtr<ILayerMap> layerMap) = 0;
    
    virtual Ptr<IPadstackDef> CreatePadstackDef(const std::string & name) = 0;
    virtual Ptr<IPadstackDef> FindPadstackDefByName(const std::string & name) const = 0;

    ///Iterator
    virtual CellIter GetCellIter() const = 0;
    virtual LayerMapIter GetLayerMapIter() const = 0;
    virtual PadstackDefIter GetPadstackDefIter() const = 0;
};

}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IDatabase)
#endif//ECAD_IDATABASE_H
