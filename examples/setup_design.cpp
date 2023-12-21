#include <boost/stacktrace.hpp>
#include <string_view>
#include <filesystem>
#include <cassert>
#include <csignal>

#include "simulation/EThermalNetworkExtraction.h"
#include "../test/TestData.hpp"
#include "EDataMgr.h"

void SignalHandler(int signum)
{
    ::signal(signum, SIG_DFL);
    std::cout << boost::stacktrace::stacktrace();
    ::raise(SIGABRT);
}

void test0()
{
    using namespace ecad;
    auto & eDataMgr = EDataMgr::Instance();

    //database
    auto database = eDataMgr.CreateDatabase("Simple");
    auto matCu = database->CreateMaterialDef("Cu");

    //coord units
    ECoordUnits coordUnits(ECoordUnits::Unit::Micrometer);
    database->SetCoordUnits(coordUnits);
    
    //top cell
    auto topCell = eDataMgr.CreateCircuitCell(database, "TopCell");
    auto topLayout = topCell->GetLayoutView();
    auto topBouds = std::make_unique<EPolygon>(std::vector<EPoint2D>{{-5000000, -5000000}, {5000000, -5000000}, {5000000, 5000000}, {-5000000, 5000000}});
    topLayout->SetBoundary(std::move(topBouds));

    [[maybe_unused]] auto iLyrTopCu = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCu", ELayerType::ConductingLayer, 0, 400, matCu->GetName(), matCu->GetName()));

    //component
    auto compDef = eDataMgr.CreateComponentDef(database, "CPMF-1200-S080B Z-FET");
    assert(compDef);
    compDef->SetBondingBox(EBox2D{-2000000, -2000000, 2000000, 2000000});
    compDef->SetMaterial(matCu->GetName());
    compDef->SetHeight(365);

    [[maybe_unused]] auto comp1 = eDataMgr.CreateComponent(topLayout, "M1", compDef, iLyrTopCu, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(0) , coordUnits.toCoord(0))), 400, false);
    assert(comp1);
    comp1->SetLossPower(33.8);

    //flatten
    database->Flatten(topCell);
    auto layout = topCell->GetFlattenedLayoutView();

    ELayoutViewRendererSettings rendererSettings;
    rendererSettings.format = ELayoutViewRendererSettings::Format::PNG;
    rendererSettings.dirName = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->Renderer(rendererSettings);
    
    ELayoutPolygonMergeSettings mergeSettings;
    mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->MergeLayerPolygons(mergeSettings);

    EThermalNetworkExtractionSettings extSettings;
    extSettings.outDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    extSettings.dumpHotmaps = true;
    extSettings.dumpSpiceFile = true;
    extSettings.dumpDensityFile = true;
    extSettings.dumpTemperatureFile = true;

    size_t xGrid = 3;
    auto bbox = layout->GetBoundary()->GetBBox();
    extSettings.grid = {xGrid, static_cast<size_t>(xGrid * EValue(bbox.Width()) / bbox.Length())};
    extSettings.mergeGeomBeforeMetalMapping = false;

    esim::EThermalNetworkExtraction ne;
    ne.SetExtractionSettings(extSettings);
    auto model1 = ne.GenerateGridThermalModel(layout);
    auto model2 = ne.GeneratePrismaThermalModel(layout, generic::math::Rad(30), 1e6, 1e10, 100);

    EDataMgr::Instance().ShutDown();
}

void test1()
{
    using namespace ecad;
    auto & eDataMgr = EDataMgr::Instance();

    //database
    auto database = eDataMgr.CreateDatabase("RobGrant");

    auto matAl = database->CreateMaterialDef("Al");
    auto matCu = database->CreateMaterialDef("Cu");
    auto matAir = database->CreateMaterialDef("Air");
    matAir->SetMaterialType(EMaterialType::Fluid);
    auto matSiC = database->CreateMaterialDef("SiC");  
    auto matSi3N4 = database->CreateMaterialDef("Si3N4");  
    //coord units
    ECoordUnits coordUnits(ECoordUnits::Unit::Micrometer);
    database->SetCoordUnits(coordUnits);
    
    //top cell
    auto topCell = eDataMgr.CreateCircuitCell(database, "TopCell");
    auto topLayout = topCell->GetLayoutView();
    auto topBouds = std::make_unique<EPolygon>(std::vector<EPoint2D>{{-5000000, -5000000}, {86000000, -5000000}, {86000000, 31000000}, {-5000000, 31000000}});
    topLayout->SetBoundary(std::move(topBouds));

    eDataMgr.CreateNet(topLayout, "Gate");
    eDataMgr.CreateNet(topLayout, "Drain");
    eDataMgr.CreateNet(topLayout, "Source");

    //substrate
    [[maybe_unused]] auto iLyrTopCu = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCu", ELayerType::ConductingLayer, 0, 400, matCu->GetName(), matAir->GetName()));
    [[maybe_unused]] auto iLyrSubstrate = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("Substrate", ELayerType::DielectricLayer, -400, 635, matSi3N4->GetName(), matSi3N4->GetName()));
    [[maybe_unused]] auto iLyrCuPlate = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("CuPlate", ELayerType::ConductingLayer, -1035, 300, matCu->GetName(), matCu->GetName()));
    assert(iLyrTopCu != ELayerId::noLayer);
    assert(iLyrSubstrate != ELayerId::noLayer);
    assert(iLyrCuPlate != ELayerId::noLayer);

    //sic die
    auto sicCell = eDataMgr.CreateCircuitCell(database, "SicDie");
    assert(sicCell);
    auto sicLayout = sicCell->GetLayoutView();
    assert(sicLayout);

    //boundary
    auto sicBonds = std::make_unique<EPolygon>(std::vector<EPoint2D>{{0, 0}, {23000000, 0}, {23000000, 26000000}, {0, 26000000}});
    sicLayout->SetBoundary(std::move(sicBonds));

    auto iLyrWire = sicLayout->AppendLayer(eDataMgr.CreateStackupLayer("Wire", ELayerType::ConductingLayer, 0, 400, matCu->GetName(), matAir->GetName()));
    assert(iLyrWire != ELayerId::noLayer);

    //component
    auto compDef = eDataMgr.CreateComponentDef(database, "CPMF-1200-S080B Z-FET");
    assert(compDef);
    compDef->SetBondingBox(EBox2D{-2000000, -2000000, 2000000, 2000000});
    compDef->SetMaterial(matSiC->GetName());
    compDef->SetHeight(365);

    bool flipped{false};
    FCoord compHeight = 400;
    [[maybe_unused]] auto comp1 = eDataMgr.CreateComponent(sicLayout, "M1", compDef, iLyrWire, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(3450) , coordUnits.toCoord(15000))), compHeight, flipped);
    [[maybe_unused]] auto comp2 = eDataMgr.CreateComponent(sicLayout, "M2", compDef, iLyrWire, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(19000) , coordUnits.toCoord(15000))), compHeight, flipped);
    assert(comp1);
    assert(comp2);
    comp1->SetLossPower(33.8);
    comp2->SetLossPower(31.9);
 
    //net
    auto gateNet = eDataMgr.CreateNet(sicLayout, "Gate");
    auto drainNet = eDataMgr.CreateNet(sicLayout, "Drain");
    auto sourceNet = eDataMgr.CreateNet(sicLayout, "Source");

    //wire
    FCoord bwRadius = 2000;//um
    std::vector<EPoint2D> ps1 {{0, 0}, {14200000, 0}, {14200000, 3500000}, {5750000, 3500000}, {5750000, 9150000}, {0, 9150000}};
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, sourceNet->GetNetId(), eDataMgr.CreateShapePolygon(std::move(ps1)));

    std::vector<EPoint2D> ps2 {{0, 10650000}, {7300000, 10650000}, {7300000, 5000000}, {14300000, 5000000}, {14300000, 19000000}, {1450000, 19000000}, {1450000, 26000000}, {0, 26000000}};
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, drainNet->GetNetId(), eDataMgr.CreateShapePolygon(std::move(ps2)));

    std::vector<EPoint2D> ps3 {{15750000, 0}, {23000000, 0}, {23000000, 18850000}, {18000000, 18850000}, {18000000, 26000000}, {14500000, 26000000}, {14500000, 20500000}, {15750000, 20500000}};
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, drainNet->GetNetId(), eDataMgr.CreateShapePolygon(std::move(ps3)));

    auto rec1 = eDataMgr.CreateShapeRectangle(EPoint2D(2500000, 20500000), EPoint2D(4000000, 26000000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec1));

    auto rec2 = eDataMgr.CreateShapeRectangle(EPoint2D(5000000, 20500000), EPoint2D(6500000, 26000000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec2));

    auto rec3 = eDataMgr.CreateShapeRectangle(EPoint2D(7500000, 20500000), EPoint2D(13500000, 23000000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec3));

    auto rec4 = eDataMgr.CreateShapeRectangle(EPoint2D(7500000, 24000000), EPoint2D(10000000, 26000000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec4));

    auto rec5 = eDataMgr.CreateShapeRectangle(EPoint2D(11000000, 24000000), EPoint2D(13500000, 26000000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec5));

    auto rec6 = eDataMgr.CreateShapeRectangle(EPoint2D(19000000, 20500000), EPoint2D(20500000, 26000000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec6));

    auto rec7 = eDataMgr.CreateShapeRectangle(EPoint2D(21500000, 20500000), EPoint2D(23000000, 26000000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec7));

    auto sourceBW1 = eDataMgr.CreateBondwire(sicLayout, "SourceBW1", sourceNet->GetNetId(), {3000000, 7500000}, {4450000, 16000000}, bwRadius);
    sourceBW1->SetStartLayer(iLyrWire);
    sourceBW1->SetEndComponent(comp1);
    
    auto sourceBW2 = eDataMgr.CreateBondwire(sicLayout, "SourceBW2", sourceNet->GetNetId(), {3000000, 5000000}, {4450000, 15000000}, bwRadius);
    sourceBW2->SetStartLayer(iLyrWire);
    sourceBW2->SetEndComponent(comp1);
    
    auto sourceBW3 = eDataMgr.CreateBondwire(sicLayout, "SourceBW3", sourceNet->GetNetId(), {3000000, 2500000}, {4450000, 14000000}, bwRadius);
    sourceBW3->SetStartLayer(iLyrWire);
    sourceBW3->SetEndComponent(comp1);

    auto sourceBW4 = eDataMgr.CreateBondwire(sicLayout, "SourceBW4", sourceNet->GetNetId(), {4450000, 16000000}, {18000000, 16000000}, bwRadius);
    sourceBW4->SetStartComponent(comp1);
    sourceBW4->SetEndComponent(comp2);

    auto sourceBW5 = eDataMgr.CreateBondwire(sicLayout, "SourceBW5", sourceNet->GetNetId(), {4450000, 15000000}, {18000000, 15000000}, bwRadius);
    sourceBW5->SetStartComponent(comp1);
    sourceBW5->SetEndComponent(comp2);

    auto sourceBW6 = eDataMgr.CreateBondwire(sicLayout, "SourceBW6", sourceNet->GetNetId(), {4450000, 14000000}, {18000000, 14000000}, bwRadius);
    sourceBW6->SetStartComponent(comp1);
    sourceBW6->SetEndComponent(comp2);

    auto drainBW1 = eDataMgr.CreateBondwire(sicLayout, "DrainBW1", drainNet->GetNetId(), {10800000, 12200000}, {19350000, 7500000}, bwRadius);
    drainBW1->SetStartLayer(iLyrWire);
    drainBW1->SetEndLayer(iLyrWire);

    auto drainBW2 = eDataMgr.CreateBondwire(sicLayout, "DrainBW2", drainNet->GetNetId(), {10800000, 10200000}, {19350000, 5500000}, bwRadius);
    drainBW2->SetStartLayer(iLyrWire);
    drainBW2->SetEndLayer(iLyrWire);

    auto drainBW3 = eDataMgr.CreateBondwire(sicLayout, "DrainBW3", drainNet->GetNetId(), {10800000,  8200000}, {19350000, 3500000}, bwRadius);
    drainBW3->SetStartLayer(iLyrWire);
    drainBW3->SetEndLayer(iLyrWire);

    auto drainBW4 = eDataMgr.CreateBondwire(sicLayout, "DrainBW4", drainNet->GetNetId(), {10800000,  6200000}, {19350000, 1500000}, bwRadius);
    drainBW4->SetStartLayer(iLyrWire);
    drainBW4->SetEndLayer(iLyrWire);

    auto gateBW1 = eDataMgr.CreateBondwire(sicLayout, "GateBW1", gateNet->GetNetId(), {3250000, 24000000}, {2450000, 14000000}, bwRadius);
    gateBW1->SetStartLayer(iLyrWire);
    gateBW1->SetEndComponent(comp1);

    auto gateBW2 = eDataMgr.CreateBondwire(sicLayout, "GateBW2", gateNet->GetNetId(), {5750000, 24000000}, {2450000, 16000000}, bwRadius);
    gateBW2->SetStartLayer(iLyrWire);
    gateBW2->SetEndComponent(comp1);

    auto gateBW3 = eDataMgr.CreateBondwire(sicLayout, "GateBW3", gateNet->GetNetId(), {19750000, 24000000}, {20000000, 16000000}, bwRadius);
    gateBW3->SetStartLayer(iLyrWire);
    gateBW3->SetEndComponent(comp2);

    auto gateBW4 = eDataMgr.CreateBondwire(sicLayout, "GateBW4", gateNet->GetNetId(), {22250000, 24000000}, {20000000, 14000000}, bwRadius);
    gateBW4->SetStartLayer(iLyrWire);
    gateBW4->SetEndComponent(comp2);
    
    auto primIter = sicLayout->GetPrimitiveIter();
    while (auto * prim = primIter->Next()) {
        if (auto * bw = prim->GetBondwireFromPrimitive(); bw) {
            bw->SetMaterial(matAl->GetName());
            bw->SetHeight(500);
        }
    }

    //layer map
    auto layerMap = eDataMgr.CreateLayerMap(database, "Layermap");
    layerMap->SetMapping(iLyrWire, iLyrTopCu);

    //instance
    auto inst1 = eDataMgr.CreateCellInst(topLayout, "Inst1", sicLayout, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(0) , 0)));
    inst1->SetLayerMap(layerMap);

    auto inst2 = eDataMgr.CreateCellInst(topLayout, "Inst2", sicLayout, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(29, ECoordUnits::Unit::Millimeter), 0)));
    inst2->SetLayerMap(layerMap);

    auto inst3 = eDataMgr.CreateCellInst(topLayout, "Inst3", sicLayout, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(58, ECoordUnits::Unit::Millimeter), 0)));
    inst3->SetLayerMap(layerMap);

    //flatten
    database->Flatten(topCell);
    auto layout = topCell->GetFlattenedLayoutView();

    ELayoutViewRendererSettings rendererSettings;
    rendererSettings.format = ELayoutViewRendererSettings::Format::PNG;
    rendererSettings.dirName = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->Renderer(rendererSettings);
    
    ELayoutPolygonMergeSettings mergeSettings;
    mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->MergeLayerPolygons(mergeSettings);

    EThermalNetworkExtractionSettings extSettings;
    extSettings.outDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    extSettings.dumpHotmaps = true;
    extSettings.dumpSpiceFile = true;
    extSettings.dumpDensityFile = true;
    extSettings.dumpTemperatureFile = true;

    size_t xGrid = 150;
    auto bbox = layout->GetBoundary()->GetBBox();
    extSettings.grid = {xGrid, static_cast<size_t>(xGrid * EValue(bbox.Width()) / bbox.Length())};
    extSettings.mergeGeomBeforeMetalMapping = false;

    esim::EThermalNetworkExtraction ne;
    ne.SetExtractionSettings(extSettings);
    // auto model = ne.GenerateGridThermalModel(layout);
    auto model = ne.GeneratePrismaThermalModel(layout, generic::math::Rad(30), 0, 100, 5000);

    EDataMgr::Instance().ShutDown();
}
int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &SignalHandler);
    ::signal(SIGABRT, &SignalHandler);

    test0();
    test1();
    return EXIT_SUCCESS;
}