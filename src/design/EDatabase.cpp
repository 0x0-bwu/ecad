#include "EDatabase.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EDatabase)

#include "collection/EComponentDefCollection.h"
#include "collection/EMaterialDefCollection.h"
#include "collection/EPadstackDefCollection.h"
#include "collection/ELayerMapCollection.h"
#include "design/EComponentDef.h"
#include "design/EMaterialDef.h"
#include "design/EPadstackDef.h"
#include "design/ELayerMap.h"
#include "design/ECell.h"

#include "generic/tools/FileSystem.hpp"
#include "interface/ICellCollection.h"
#include "utility/EFlattenUtility.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void EDatabase::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)

    if constexpr (Archive::is_loading::value) Clear();
    boost::serialization::void_cast_register<EDatabase, IDatabase>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinitionCollection);
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("coord_units", m_coordUnits);

    if constexpr (Archive::is_loading::value) {
        EDefinitionCollection::SetDatabase(this);
    }
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EDatabase)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EDatabase::EDatabase()
 : EDatabase(std::string{})
{
}

ECAD_INLINE EDatabase::EDatabase(std::string name)
 : EObject(std::move(name))
{
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::ComponentDef);
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::PadstackDef);
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::MaterialDef);
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::LayerMap);
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::Cell);
}

ECAD_INLINE EDatabase::~EDatabase()
{
}

ECAD_INLINE EDatabase::EDatabase(const EDatabase & other)
{
    *this = other;
}

ECAD_INLINE EDatabase & EDatabase::operator= (const EDatabase & other)
{
    EDefinitionCollection::operator=(other);
    EDefinitionCollection::SetDatabase(this);
    EObject::operator=(other);
    
    m_coordUnits = other.m_coordUnits;
    return *this;
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
ECAD_INLINE bool EDatabase::Save(const std::string & archive, EArchiveFormat fmt) const
{
    auto dir = generic::fs::DirName(archive);
    if (not generic::fs::CreateDir(dir)) return false;

    std::ofstream ofs(archive);
    if (not ofs.is_open()) return false;

    unsigned int version = toInt(CURRENT_VERSION);
    if (fmt == EArchiveFormat::TXT) {
        boost::archive::text_oarchive oa(ofs);
        oa & boost::serialization::make_nvp("version", version);
        const_cast<Ptr<EDatabase>>(this)->serialize(oa, version);
    }
    else if (fmt == EArchiveFormat::XML) {
        boost::archive::xml_oarchive oa(ofs);
        oa & boost::serialization::make_nvp("version", version);
        const_cast<Ptr<EDatabase>>(this)->serialize(oa, version);
    }
    else if (fmt == EArchiveFormat::BIN) {
        boost::archive::binary_oarchive oa(ofs);
        oa & boost::serialization::make_nvp("version", version);
        const_cast<Ptr<EDatabase>>(this)->serialize(oa, version);
    }

    return true;
}

ECAD_INLINE bool EDatabase::Load(const std::string & archive, EArchiveFormat fmt)
{
    std::ifstream ifs(archive);
    if(!ifs.is_open()) return false;

    unsigned int version{0};
    if(fmt == EArchiveFormat::TXT){
        boost::archive::text_iarchive ia(ifs);
        ia & boost::serialization::make_nvp("version", version);
        serialize(ia, version);
    }
    else if(fmt == EArchiveFormat::XML){
        boost::archive::xml_iarchive ia(ifs);
        ia & boost::serialization::make_nvp("version", version);
        serialize(ia, version);
    }
    else if(fmt == EArchiveFormat::BIN){
        boost::archive::binary_iarchive ia(ifs);
        ia & boost::serialization::make_nvp("version", version);
        serialize(ia, version);
    }
    m_version = fromInt(version);
    return true;
}
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE void EDatabase::SetCoordUnits(const ECoordUnits & u)
{
    m_coordUnits = u;
}

ECAD_INLINE const ECoordUnits & EDatabase::GetCoordUnits() const
{
    return m_coordUnits;
}

ECAD_INLINE std::string EDatabase::GetNextDefName(const std::string & base, EDefinitionType type) const
{
    return EDefinitionCollection::GetNextDefName(base, type);
}

ECAD_INLINE Ptr<ICellCollection> EDatabase::GetCellCollection() const
{
    return dynamic_cast<Ptr<ICellCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::Cell));
}

ECAD_INLINE Ptr<ICell> EDatabase::CreateCircuitCell(const std::string & name)
{
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::Cell)) return nullptr;

    auto cell = new ECircuitCell(name, this);
    return dynamic_cast<Ptr<ICell> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(cell)));
}

ECAD_INLINE Ptr<ICell> EDatabase::FindCellByName(const std::string & name) const
{
    return dynamic_cast<Ptr<ICell> >(EDefinitionCollection::GetDefinition(name, EDefinitionType::Cell));
}

ECAD_INLINE bool EDatabase::GetCircuitCells(std::vector<Ptr<ICell> > & cells) const
{
    cells.clear();
    auto cellIter = GetCellCollection()->GetCellIter();
    while(auto cell = cellIter->Next()){
        if(ECellType::CircuitCell == cell->GetCellType())
            cells.push_back(cell);
    }
    return cells.size() > 0;
}

ECAD_INLINE bool EDatabase::GetTopCells(std::vector<Ptr<ICell> > & cells) const
{
    return utils::EFlattenUtility::GetTopCells(const_cast<Ptr<EDatabase> >(this), cells);
}

ECAD_INLINE bool EDatabase::Flatten(Ptr<ICell> cell, size_t threads) const
{
    utils::EFlattenUtility utility;
    return utility.Flatten(const_cast<Ptr<EDatabase> >(this), cell, threads);
}

ECAD_INLINE Ptr<IMaterialDefCollection> EDatabase::GetMaterialDefCollection() const
{
    return dynamic_cast<Ptr<IMaterialDefCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::MaterialDef));
}

ECAD_INLINE Ptr<IMaterialDef> EDatabase::CreateMaterialDef(const std::string & name)
{
    if (EDefinitionCollection::GetDefinition(name, EDefinitionType::MaterialDef)) return nullptr;

    EMaterialId id = static_cast<EMaterialId>(GetDefinitionCollection(EDefinitionType::MaterialDef)->Size());
    return dynamic_cast<Ptr<IMaterialDef> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(new EMaterialDef(name, this, id))));
}

ECAD_INLINE Ptr<IMaterialDef> EDatabase::FindMaterialDefByName(const std::string & name) const
{
    auto material = EDefinitionCollection::GetDefinition(name, EDefinitionType::MaterialDef);
    return dynamic_cast<Ptr<IMaterialDef> >(material);
}

ECAD_INLINE Ptr<IMaterialDef> EDatabase::FindMaterialDefById(EMaterialId id) const
{
    auto matLib = dynamic_cast<Ptr<EMaterialDefCollection>>(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::MaterialDef));
    ECAD_ASSERT(nullptr != matLib);
    return matLib->FindMaterialDefById(id);
}

ECAD_INLINE Ptr<IComponentDefCollection> EDatabase::GetComponentDefCollection() const
{
    return dynamic_cast<Ptr<IComponentDefCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::ComponentDef));
}

ECAD_INLINE Ptr<IComponentDef> EDatabase::CreateComponentDef(const std::string & name)
{
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::ComponentDef)) return nullptr;

    auto compDef = new EComponentDef(name, this);
    return dynamic_cast<Ptr<IComponentDef> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(compDef)));
}

ECAD_INLINE Ptr<IComponentDef> EDatabase::FindComponentDefByName(const std::string & name) const
{
    auto compDef = EDefinitionCollection::GetDefinition(name, EDefinitionType::ComponentDef);
    return dynamic_cast<Ptr<IComponentDef> >(compDef);
}

ECAD_INLINE Ptr<ILayerMapCollection> EDatabase::GetLayerMapCollection() const
{
    return dynamic_cast<Ptr<ILayerMapCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::LayerMap));
}

ECAD_INLINE Ptr<ILayerMap> EDatabase::CreateLayerMap(const std::string & name)
{
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::LayerMap)) return nullptr;

    auto layerMap = new ELayerMap(name, this);
    return dynamic_cast<Ptr<ILayerMap> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(layerMap)));
}

ECAD_INLINE Ptr<ILayerMap> EDatabase::FindLayerMapByName(const std::string & name) const
{
    auto layerMap = EDefinitionCollection::GetDefinition(name, EDefinitionType::LayerMap);
    return dynamic_cast<Ptr<ILayerMap> >(layerMap);
}

ECAD_INLINE bool EDatabase::AddLayerMap(UPtr<ILayerMap> layerMap)
{
    auto name = layerMap->GetName();
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::LayerMap)) return false;
    auto def = dynamic_cast<Ptr<IDefinition> >(layerMap.get());
    auto res = EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(def));
    layerMap.release();
    return res != nullptr;
}

ECAD_INLINE Ptr<IPadstackDefCollection> EDatabase::GetPadstackDefCollection() const
{
    return dynamic_cast<Ptr<IPadstackDefCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::PadstackDef));
}

ECAD_INLINE Ptr<IPadstackDef> EDatabase::CreatePadstackDef(const std::string & name)
{
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::PadstackDef)) return nullptr;

    auto padstackDef = new EPadstackDef(name, this);
    return dynamic_cast<Ptr<IPadstackDef> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(padstackDef)));
}

ECAD_INLINE Ptr<IPadstackDef> EDatabase::FindPadstackDefByName(const std::string & name) const
{
    auto psDef = EDefinitionCollection::GetDefinition(name, EDefinitionType::PadstackDef);
    return dynamic_cast<Ptr<IPadstackDef> >(psDef);
}

ECAD_INLINE CellIter EDatabase::GetCellIter() const
{
    return GetCellCollection()->GetCellIter();
}

ECAD_INLINE LayerMapIter EDatabase::GetLayerMapIter() const
{
    return GetLayerMapCollection()->GetLayerMapIter();
}

ECAD_INLINE MaterialDefIter EDatabase::GetMaterialDefIter() const
{
    return GetMaterialDefCollection()->GetMaterialDefIter();
}

ECAD_INLINE PadstackDefIter EDatabase::GetPadstackDefIter() const
{
    return GetPadstackDefCollection()->GetPadstackDefIter();
}

ECAD_INLINE void EDatabase::Clear()
{
    EDefinitionCollection::Clear();
}

}//namespace ecad