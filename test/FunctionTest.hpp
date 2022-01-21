#ifndef ECAD_TEST_FUNCTIONTEST_HPP
#define ECAD_TEST_FUNCTIONTEST_HPP
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "Interface.h"
#include "EDataMgr.h"

using namespace boost::unit_test;
using namespace ecad;

const std::string dbName = "test_db";
const std::string cellName = "test_cell";
const std::string netName = "test_net";

void t_class_data_mgr()
{
    auto & mgr = EDataMgr::Instance();
    auto database = mgr.CreateDatabase(dbName);
    BOOST_CHECK(database != nullptr);
    database = mgr.CreateDatabase(dbName);
    BOOST_CHECK(database == nullptr);
    BOOST_CHECK(mgr.RemoveDatabase(dbName));

    database = mgr.CreateDatabase(dbName);
    auto cell = mgr.CreateCircuitCell(database, cellName);
    BOOST_CHECK(cell != nullptr);

    auto find = mgr.FindCellByName(database, cellName);
    BOOST_CHECK(cell == find);

    auto layout = cell->GetLayoutView();
    BOOST_CHECK(layout != nullptr);

    auto net = mgr.CreateNet(layout, netName);
    BOOST_CHECK(net != nullptr);

    mgr.ShutDown();
}

void t_class_cell()
{
    auto & mgr = EDataMgr::Instance();
    auto database = mgr.CreateDatabase(dbName);
    auto cell = mgr.CreateCircuitCell(database, cellName);
    auto layout = cell->GetLayoutView();
    auto flattenedLayout = cell->GetFlattenedLayoutView();
    BOOST_CHECK(layout->GetCell() == flattenedLayout->GetCell());
    mgr.ShutDown();
}

void t_class_layer_collection()
{
    auto & mgr = EDataMgr::Instance();
    auto database = mgr.CreateDatabase(dbName);
    auto cell = mgr.CreateCircuitCell(database, cellName);
    auto layout = cell->GetLayoutView();
    auto lc = layout->GetLayerCollection();
    BOOST_CHECK(lc != nullptr);

    std::vector<UPtr<ILayer> > layers;
    layers.push_back(mgr.CreateStackupLayer("layer1", ELayerType::ConductingLayer, 0, 0));
    layers.push_back(mgr.CreateStackupLayer("layer2", ELayerType::ConductingLayer, 0, 0));
    layers.push_back(mgr.CreateStackupLayer("layer3", ELayerType::ConductingLayer, 0, 0));

    lc->AppendLayers(std::move(layers));
    BOOST_CHECK(lc->Size() == 3);

    auto iter1 = lc->GetLayerIter();
    size_t id = 0;
    while(auto layer = iter1->Next()){
        BOOST_CHECK(layer->GetLayerId() == static_cast<ELayerId>(id++));
    }
    lc->AddDefaultDielectricLayers();
    BOOST_CHECK(lc->Size() == 5);

    mgr.ShutDown();
}

void t_class_primitive()
{
    auto & mgr = EDataMgr::Instance();
    auto database = mgr.CreateDatabase(dbName);
    auto cell = mgr.CreateCircuitCell(database, cellName);
    auto layout = cell->GetLayoutView();

    auto shape = mgr.CreateShapePolygon({{0, 0}, {10, 0}, {10, 10}, {0, 10}});
    auto prim = mgr.CreateGeometry2D(layout, ELayerId(-1), ENetId(-1), std::move(shape));

    //geometry2d
    auto geom = prim->GetGeometry2DFromPrimitive();
    BOOST_CHECK(geom != nullptr);
    
    mgr.ShutDown();   
}

test_suite * create_ecad_function_test_suite()
{
    test_suite * function_suite = BOOST_TEST_SUITE("s_function_test");
    //
    function_suite->add(BOOST_TEST_CASE(&t_class_data_mgr));
    function_suite->add(BOOST_TEST_CASE(&t_class_cell));
    function_suite->add(BOOST_TEST_CASE(&t_class_layer_collection));
    function_suite->add(BOOST_TEST_CASE(&t_class_primitive));
    //
    return function_suite;
}
#endif//ECAD_TEST_FUNCTIONTEST_HPP
