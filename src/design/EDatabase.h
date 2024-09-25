#pragma once
#include "interface/IDatabase.h"
#include "collection/EDefinitionCollection.h"
#include "EObject.h"
#include <string>
namespace ecad {

class ECAD_API EDatabase : public EDefinitionCollection, public EObject, public IDatabase
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
    bool Save(const std::string & archive, EArchiveFormat fmt = EArchiveFormat::BIN) const override;
    bool Load(const std::string & archive, EArchiveFormat fmt = EArchiveFormat::BIN) override;
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

    void SetCoordUnits(const ECoordUnits & u) override;
    const ECoordUnits & GetCoordUnits() const override;

    std::string GetNextDefName(const std::string & base, EDefinitionType type) const override;
    
    ///Cell
    Ptr<ICellCollection> GetCellCollection() const override;
    Ptr<ICell> CreateCircuitCell(const std::string & name) override;
    Ptr<ICell> FindCellByName(const std::string & name) const override;
    bool GetCircuitCells(std::vector<Ptr<ICell> > & cells) const override;
    bool GetTopCells(std::vector<Ptr<ICell> > & cells) const override;
    bool Flatten(Ptr<ICell> cell, size_t threads) const override;
    
    ///Material
    Ptr<IMaterialDefCollection> GetMaterialDefCollection() const override;
    Ptr<IMaterialDef> CreateMaterialDef(const std::string & name) override;
    Ptr<IMaterialDef> FindMaterialDefByName(const std::string & name) const override;
    Ptr<IMaterialDef> FindMaterialDefById(EMaterialId id) const override;

    ///ComponentDef
    Ptr<IComponentDefCollection> GetComponentDefCollection() const override;
    Ptr<IComponentDef> CreateComponentDef(const std::string & name) override;
    Ptr<IComponentDef> FindComponentDefByName(const std::string & name) const override;

    ///LayerMap
    Ptr<ILayerMapCollection> GetLayerMapCollection() const override;
    Ptr<ILayerMap> CreateLayerMap(const std::string & name) override;
    Ptr<ILayerMap> FindLayerMapByName(const std::string & name) const override;
    bool AddLayerMap(UPtr<ILayerMap> layerMap) override;
    
    ///PadstackDef
    Ptr<IPadstackDefCollection> GetPadstackDefCollection() const override;
    Ptr<IPadstackDef> CreatePadstackDef(const std::string & name) override;
    Ptr<IPadstackDef> FindPadstackDefByName(const std::string & name) const override;

    ///Iterator
    CellIter GetCellIter() const override;
    LayerMapIter GetLayerMapIter() const override;
    MaterialDefIter GetMaterialDefIter() const override;
    PadstackDefIter GetPadstackDefIter() const override;
    
    const std::string & GetName() const override;
    std::string sUuid() const override;

    virtual void Clear() override;

protected:
    ///Copy
    virtual Ptr<EDatabase> CloneImp() const override { return new EDatabase(*this); }
    
protected:
    ECoordUnits m_coordUnits;
    EVersion m_version{CURRENT_VERSION};
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