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
void EDatabase::serialize(Archive & ar, const unsigned int version)
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

EDatabase::EDatabase()
 : EDatabase(std::string{})
{
}

EDatabase::EDatabase(std::string name)
 : EObject(std::move(name))
{
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::ComponentDef);
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::PadstackDef);
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::MaterialDef);
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::LayerMap);
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::Cell);
}

EDatabase::~EDatabase()
{
}

EDatabase::EDatabase(const EDatabase & other)
{
    *this = other;
}

EDatabase & EDatabase::operator= (const EDatabase & other)
{
    EDefinitionCollection::operator=(other);
    EDefinitionCollection::SetDatabase(this);
    EObject::operator=(other);
    
    m_coordUnits = other.m_coordUnits;
    return *this;
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
bool EDatabase::Save(const std::string & archive, EArchiveFormat fmt) const
{
    auto dir = generic::fs::DirName(archive);
    if (not generic::fs::CreateDir(dir)) return false;

    std::ofstream ofs(archive);
    if (not ofs.is_open()) return false;

    unsigned int version = CURRENT_VERSION.toInt();
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

bool EDatabase::Load(const std::string & archive, EArchiveFormat fmt)
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
    m_version = EVersion(version);
    return true;
}
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

void EDatabase::SetCoordUnits(const ECoordUnits & u)
{
    m_coordUnits = u;
}

const ECoordUnits & EDatabase::GetCoordUnits() const
{
    return m_coordUnits;
}

std::string EDatabase::GetNextDefName(const std::string & base, EDefinitionType type) const
{
    return EDefinitionCollection::GetNextDefName(base, type);
}

Ptr<ICellCollection> EDatabase::GetCellCollection() const
{
    return dynamic_cast<Ptr<ICellCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::Cell));
}

Ptr<ICell> EDatabase::CreateCircuitCell(const std::string & name)
{
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::Cell)) return nullptr;

    auto cell = new ECircuitCell(name, this);
    return dynamic_cast<Ptr<ICell> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(cell)));
}

Ptr<ICell> EDatabase::FindCellByName(const std::string & name) const
{
    return dynamic_cast<Ptr<ICell> >(EDefinitionCollection::GetDefinition(name, EDefinitionType::Cell));
}

bool EDatabase::GetCircuitCells(std::vector<Ptr<ICell> > & cells) const
{
    cells.clear();
    auto cellIter = GetCellCollection()->GetCellIter();
    while(auto cell = cellIter->Next()){
        if(ECellType::CircuitCell == cell->GetCellType())
            cells.push_back(cell);
    }
    return cells.size() > 0;
}

bool EDatabase::GetTopCells(std::vector<Ptr<ICell> > & cells) const
{
    return utils::EFlattenUtility::GetTopCells(const_cast<Ptr<EDatabase> >(this), cells);
}

bool EDatabase::Flatten(Ptr<ICell> cell, size_t threads) const
{
    utils::EFlattenUtility utility;
    return utility.Flatten(const_cast<Ptr<EDatabase> >(this), cell, threads);
}

Ptr<IMaterialDefCollection> EDatabase::GetMaterialDefCollection() const
{
    return dynamic_cast<Ptr<IMaterialDefCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::MaterialDef));
}

Ptr<IMaterialDef> EDatabase::CreateMaterialDef(const std::string & name)
{
    if (EDefinitionCollection::GetDefinition(name, EDefinitionType::MaterialDef)) return nullptr;

    EMaterialId id = static_cast<EMaterialId>(GetDefinitionCollection(EDefinitionType::MaterialDef)->Size());
    return dynamic_cast<Ptr<IMaterialDef> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(new EMaterialDef(name, this, id))));
}

Ptr<IMaterialDef> EDatabase::FindMaterialDefByName(const std::string & name) const
{
    auto material = EDefinitionCollection::GetDefinition(name, EDefinitionType::MaterialDef);
    return dynamic_cast<Ptr<IMaterialDef> >(material);
}

Ptr<IMaterialDef> EDatabase::FindMaterialDefById(EMaterialId id) const
{
    auto matLib = dynamic_cast<Ptr<EMaterialDefCollection>>(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::MaterialDef));
    ECAD_ASSERT(nullptr != matLib);
    return matLib->FindMaterialDefById(id);
}

Ptr<IComponentDefCollection> EDatabase::GetComponentDefCollection() const
{
    return dynamic_cast<Ptr<IComponentDefCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::ComponentDef));
}

Ptr<IComponentDef> EDatabase::CreateComponentDef(const std::string & name)
{
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::ComponentDef)) return nullptr;

    auto compDef = new EComponentDef(name, this);
    return dynamic_cast<Ptr<IComponentDef> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(compDef)));
}

Ptr<IComponentDef> EDatabase::FindComponentDefByName(const std::string & name) const
{
    auto compDef = EDefinitionCollection::GetDefinition(name, EDefinitionType::ComponentDef);
    return dynamic_cast<Ptr<IComponentDef> >(compDef);
}

Ptr<ILayerMapCollection> EDatabase::GetLayerMapCollection() const
{
    return dynamic_cast<Ptr<ILayerMapCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::LayerMap));
}

Ptr<ILayerMap> EDatabase::CreateLayerMap(const std::string & name)
{
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::LayerMap)) return nullptr;

    auto layerMap = new ELayerMap(name, this);
    return dynamic_cast<Ptr<ILayerMap> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(layerMap)));
}

Ptr<ILayerMap> EDatabase::FindLayerMapByName(const std::string & name) const
{
    auto layerMap = EDefinitionCollection::GetDefinition(name, EDefinitionType::LayerMap);
    return dynamic_cast<Ptr<ILayerMap> >(layerMap);
}

bool EDatabase::AddLayerMap(UPtr<ILayerMap> layerMap)
{
    auto name = layerMap->GetName();
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::LayerMap)) return false;
    auto def = dynamic_cast<Ptr<IDefinition> >(layerMap.get());
    auto res = EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(def));
    layerMap.release();
    return res != nullptr;
}

Ptr<IPadstackDefCollection> EDatabase::GetPadstackDefCollection() const
{
    return dynamic_cast<Ptr<IPadstackDefCollection> >(EDefinitionCollection::GetDefinitionCollection(EDefinitionType::PadstackDef));
}

Ptr<IPadstackDef> EDatabase::CreatePadstackDef(const std::string & name)
{
    if(EDefinitionCollection::GetDefinition(name, EDefinitionType::PadstackDef)) return nullptr;

    auto padstackDef = new EPadstackDef(name, this);
    return dynamic_cast<Ptr<IPadstackDef> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(padstackDef)));
}

Ptr<IPadstackDef> EDatabase::FindPadstackDefByName(const std::string & name) const
{
    auto psDef = EDefinitionCollection::GetDefinition(name, EDefinitionType::PadstackDef);
    return dynamic_cast<Ptr<IPadstackDef> >(psDef);
}

CellIter EDatabase::GetCellIter() const
{
    return GetCellCollection()->GetCellIter();
}

LayerMapIter EDatabase::GetLayerMapIter() const
{
    return GetLayerMapCollection()->GetLayerMapIter();
}

MaterialDefIter EDatabase::GetMaterialDefIter() const
{
    return GetMaterialDefCollection()->GetMaterialDefIter();
}

PadstackDefIter EDatabase::GetPadstackDefIter() const
{
    return GetPadstackDefCollection()->GetPadstackDefIter();
}

void EDatabase::Clear()
{
    EDefinitionCollection::Clear();
}

}//namespace ecad