#pragma once
#include "interfaces/IDatabase.h"
#include "EDefinitionCollection.h"
#include <string>
namespace ecad {

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
    bool Save(const std::string & archive, EArchiveFormat fmt = EArchiveFormat::BIN) const;
    bool Load(const std::string & archive, EArchiveFormat fmt = EArchiveFormat::BIN);
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    void SetCoordUnits(const ECoordUnits & u);
    const ECoordUnits & GetCoordUnits() const;

    std::string GetNextDefName(const std::string & base, EDefinitionType type) const;
    Ptr<IDefinitionCollection> GetDefinitionCollection();

    ///Cell
    Ptr<ICellCollection> GetCellCollection() const;
    Ptr<ICell> CreateCircuitCell(const std::string & name);
    Ptr<ICell> FindCellByName(const std::string & name);
    bool GetCircuitCells(std::vector<Ptr<ICell> > & cells) const;
    bool GetTopCells(std::vector<Ptr<ICell> > & cells) const;
    bool Flatten(Ptr<ICell> cell) const;
    
    ///Material
    Ptr<IMaterialDefCollection> GetMaterialCollection() const;
    Ptr<IMaterialDef> CreateMaterialDef(const std::string & name);
    Ptr<IMaterialDef> FindMaterialDefByName(const std::string & name) const;

    ///ComponentDef
    Ptr<IComponentDefCollection> GetComponentDefCollection() const;
    Ptr<IComponentDef> CreateComponentDef(const std::string & name);
    Ptr<IComponentDef> FindComponentDefByName(const std::string & name);

    ///LayerMap
    Ptr<ILayerMapCollection> GetLayerMapCollection() const;
    Ptr<ILayerMap> CreateLayerMap(const std::string & name);
    Ptr<ILayerMap> FindLayerMapByName(const std::string & name) const;
    bool AddLayerMap(UPtr<ILayerMap> layerMap);
    
    ///PadstackDef
    Ptr<IPadstackDefCollection> GetPadstackDefCollection() const;
    Ptr<IPadstackDef> CreatePadstackDef(const std::string & name);
    Ptr<IPadstackDef> FindPadstackDefByName(const std::string & name) const;

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