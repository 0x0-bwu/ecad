#pragma once
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "extension/ECadExtension.h"
#include "TestData.hpp"
#include "EDataMgr.h"
using namespace boost::unit_test;
using namespace ecad;

bool f_serialization_database_varify(SPtr<IDatabase> database)
{
    if(nullptr == database) return false;

    std::vector<Ptr<ICell> > cells;
    database->GetCircuitCells(cells);
    if(cells.size() != 1) return false;
    if(cells.front() != cells.front()->GetLayoutView()->GetCell()) return false;

    bool res = true;
    auto layout = cells.front()->GetLayoutView();
    res = res && (76  == layout->GetNetCollection()->Size());
    res = res && (7   == layout->GetLayerCollection()->Size());
    res = res && (728 == layout->GetConnObjCollection()->Size());
    res = res && (0   == layout->GetCellInstCollection()->Size());
    res = res && (481 == layout->GetPrimitiveCollection()->Size());
    res = res && (247 == layout->GetPadstackInstCollection()->Size());
    return true;
}

void t_boost_serialization()
{
    using namespace generic::filesystem;

    std::string err;
    std::string dmc = ecad_test::GetTestDataPath() + "/dmcdom/import.dmc";
    std::string dom = ecad_test::GetTestDataPath() + "/dmcdom/import.dom";
    auto database = ext::CreateDatabaseFromDomDmc("test_dmcdom", dmc, dom);
    BOOST_CHECK(err.empty());
    BOOST_CHECK(database != nullptr);

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    BOOST_TEST_MESSAGE("boost serialization supported!"); 

    std::string archive_txt = ecad_test::GetTestDataPath() + "/serialization/archive.txt";
    std::string archive_xml = ecad_test::GetTestDataPath() + "/serialization/archive.xml";
    std::string archive_bin = ecad_test::GetTestDataPath() + "/serialization/archive.bin";

    if(FileExists(archive_txt)) RemoveFile(archive_txt);
    if(FileExists(archive_xml)) RemoveFile(archive_xml);
    if(FileExists(archive_bin)) RemoveFile(archive_bin);

    BOOST_CHECK(database->Save(archive_txt, EArchiveFormat::TXT));
    BOOST_CHECK(database->Save(archive_xml, EArchiveFormat::XML));
    BOOST_CHECK(database->Save(archive_bin, EArchiveFormat::BIN));

    BOOST_CHECK(FileExists(archive_txt));
    BOOST_CHECK(FileExists(archive_xml));
    BOOST_CHECK(FileExists(archive_bin));

    BOOST_CHECK(database->Load(archive_txt, EArchiveFormat::TXT));
    BOOST_CHECK(f_serialization_database_varify(database));

    BOOST_CHECK(database->Load(archive_xml, EArchiveFormat::XML));
    BOOST_CHECK(f_serialization_database_varify(database));

    BOOST_CHECK(database->Load(archive_bin, EArchiveFormat::BIN));
    BOOST_CHECK(f_serialization_database_varify(database));

    BOOST_CHECK(RemoveFile(archive_txt));
    BOOST_CHECK(RemoveFile(archive_xml));
    BOOST_CHECK(RemoveFile(archive_bin));

    EDataMgr::Instance().ShutDown();
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
}

test_suite * create_ecad_serialization_test_suite()
{
    test_suite * serialization_suite = BOOST_TEST_SUITE("s_serialization_test");
    //
    serialization_suite->add(BOOST_TEST_CASE(&t_boost_serialization));
    //
    return serialization_suite;
}