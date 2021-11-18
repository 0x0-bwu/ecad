#ifndef ECAD_HEADER_ONLY
#include "EDatabase.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EDatabase)
#endif

#include "utility/EFlattenUtility.h"
#include "EPadstackDefCollection.h"
#include "ELayerMapCollection.h"
#include "ECellCollection.h"
#include "EPadstackDef.h"
#include "ELayerMap.h"
#include "ECell.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
template <typename Archive>
ECAD_INLINE void EDatabase::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EDatabase, IDatabase>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinitionCollection);
    ar & boost::serialization::make_nvp("coord_units", m_coordUnits);
}

template <typename Archive>
ECAD_INLINE void EDatabase::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)

    Clear();
    boost::serialization::void_cast_register<EDatabase, IDatabase>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinitionCollection);
    ar & boost::serialization::make_nvp("coord_units", m_coordUnits);

    //need set cell's ref since EDatabase may not come from serialization
    auto cellIter = GetCellIter();
    while(auto * cell = cellIter->Next()){
        cell->SetDatabase(this);
    }
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EDatabase)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EDatabase::EDatabase()
 : EDatabase(std::string{})
{
}

ECAD_INLINE EDatabase::EDatabase(std::string name)
 : EDefinitionCollection(std::move(name))
{
    EDefinitionCollection::AddDefinitionCollection(EDefinitionType::PadstackDef);
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
    m_coordUnits = other.m_coordUnits;

    auto cellIter = GetCellIter();
    while(auto cell = cellIter->Next()){
        cell->SetDatabase(this);
    }
    return *this;
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
ECAD_INLINE bool EDatabase::Save(const std::string & archive, EArchiveFormat fmt) const
{
    std::ofstream ofs(archive);
    if(!ofs.is_open()) return false;
    
    unsigned int version = toInt(currentVer);
    if(fmt == EArchiveFormat::TXT){
        boost::archive::text_oarchive oa(ofs);
        oa & boost::serialization::make_nvp("version", version);
        save(oa, version);
    }
    else if(fmt == EArchiveFormat::XML){
        boost::archive::xml_oarchive oa(ofs);
        oa & boost::serialization::make_nvp("version", version);
        save(oa, version);
    }
    else if(fmt == EArchiveFormat::BIN){
        boost::archive::binary_oarchive oa(ofs);
        oa & boost::serialization::make_nvp("version", version);
        save(oa, version);
    }

    return true;
}

ECAD_INLINE bool EDatabase::Load(const std::string & archive, EArchiveFormat fmt)
{
    std::ifstream ifs(archive);
    if(!ifs.is_open()) return false;

    unsigned int version;
    if(fmt == EArchiveFormat::TXT){
        boost::archive::text_iarchive ia(ifs);
        ia & boost::serialization::make_nvp("version", version);
        load(ia, version);
    }
    else if(fmt == EArchiveFormat::XML){
        boost::archive::xml_iarchive ia(ifs);
        ia & boost::serialization::make_nvp("version", version);
        load(ia, version);
    }
    else if(fmt == EArchiveFormat::BIN){
        boost::archive::binary_iarchive ia(ifs);
        ia & boost::serialization::make_nvp("version", version);
        load(ia, version);
    }

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

ECAD_INLINE Ptr<IDefinitionCollection> EDatabase::GetDefinitionCollection()
{
    return dynamic_cast<Ptr<IDefinitionCollection> >(this);
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

ECAD_INLINE Ptr<ICell> EDatabase::FindCellByName(const std::string & name)
{
    return dynamic_cast<Ptr<ICell> >(EDefinitionCollection::GetDefinition(name, EDefinitionType::Cell));
}

ECAD_INLINE bool EDatabase::GetCircuitCells(std::vector<Ptr<ICell> > & cells) const
{
    cells.clear();
    auto cellIter = GetCellCollection()->GetCellIter();
    while(auto * cell = cellIter->Next()){
        if(ECellType::CircuitCell == cell->GetCellType())
            cells.push_back(cell);
    }
    return cells.size() > 0;
}

ECAD_INLINE bool EDatabase::GetTopCells(std::vector<Ptr<ICell> > & cells) const
{
    return euti::EFlattenUtility::GetTopCells(const_cast<Ptr<EDatabase> >(this), cells);
}

ECAD_INLINE bool EDatabase::Flatten(Ptr<ICell> cell) const
{
    euti::EFlattenUtility utility;
    return utility.Flatten(const_cast<Ptr<EDatabase> >(this), cell, DefaultThreads());
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

    auto padstackDef = new EPadstackDef(name);
    return dynamic_cast<Ptr<IPadstackDef> >(EDefinitionCollection::AddDefinition(name, UPtr<IDefinition>(padstackDef)));
}

ECAD_INLINE CellIter EDatabase::GetCellIter() const
{
    return GetCellCollection()->GetCellIter();
}

ECAD_INLINE LayerMapIter EDatabase::GetLayerMapIter() const
{
    return GetLayerMapCollection()->GetLayerMapIter();
}

ECAD_INLINE PadstackDefIter EDatabase::GetPadstackDefIter() const
{
    return GetPadstackDefCollection()->GetPadstackDefIter();
}

ECAD_INLINE void EDatabase::Clear()
{
    GetPadstackDefCollection()->Clear();
    GetLayerMapCollection()->Clear();
    GetCellCollection()->Clear();
}

}//namesapce ecad