#ifndef ECAD_EDATABASE_H
#define ECAD_EDATABASE_H
#include "interfaces/IDatabase.h"
#include "EDefinitionCollection.h"
#include <string>
namespace ecad {

class ICell;
class ILayerMap;
class IDefinition;
class IPadstackDef;
class ILayerMapCollection;
class IDefinitionCollection;
class IPadstackDefCollection;
using CellCollection = EUnorderedMapCollection<std::string, UPtr<ICell> >;
class ECAD_API EDatabase : public EDefinitionCollection, public IDatabase
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EDatabase();
public:
    explicit EDatabase(std::string name);
    virtual ~EDatabase();

    ///Copy
    EDatabase(const EDatabase & other);
    EDatabase & operator= (const EDatabase & other);

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    bool Save(const std::string & archive, EArchiveFormat fmt = EArchiveFormat::TXT) const;
    bool Load(const std::string & archive, EArchiveFormat fmt = EArchiveFormat::TXT);
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    void SetCoordUnits(const ECoordUnits & u);
    const ECoordUnits & GetCoordUnits() const;

    std::string GetNextDefName(const std::string & base, EDefinitionType type) const;
    Ptr<IDefinitionCollection> GetDefinitionCollection();

    Ptr<ICellCollection> GetCellCollection() const;
    Ptr<ICell> CreateCircuitCell(const std::string & name);
    Ptr<ICell> FindCellByName(const std::string & name);
    bool GetCircuitCells(std::vector<Ptr<ICell> > & cells) const;
    bool GetTopCells(std::vector<Ptr<ICell> > & cells) const;
    bool Flatten(Ptr<ICell> cell) const;

    ///LayerMap
    Ptr<ILayerMapCollection> GetLayerMapCollection() const;
    Ptr<ILayerMap> CreateLayerMap(const std::string & name);
    bool AddLayerMap(UPtr<ILayerMap> layerMap);
    
    ///PadstackDef
    Ptr<IPadstackDefCollection> GetPadstackDefCollection() const;
    Ptr<IPadstackDef> CreatePadstackDef(const std::string & name);

    ///Iterator
    CellIter GetCellIter() const;
    LayerMapIter GetLayerMapIter() const;
    PadstackDefIter GetPadstackDefIter() const;
    
    const std::string & GetName() const;
    std::string sUuid() const;

    void Clear();

protected:
    ///Copy
    virtual Ptr<EDatabase> CloneImp() const override { return new EDatabase(*this); }
    
protected:
    ECoordUnits m_coordUnits;
};

ECAD_ALWAYS_INLINE const std::string & EDatabase::GetName() const
{
    return EObject::GetName();
}

ECAD_ALWAYS_INLINE std::string EDatabase::sUuid() const
{
    return EObject::sUuid();
}

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EDatabase)

#ifdef ECAD_HEADER_ONLY
#include "EDatabase.cpp"
#endif

#endif//ECAD_EDATABASE_H