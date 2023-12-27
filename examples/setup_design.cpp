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
    matCu->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(398));
    matCu->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(380));
    matCu->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(8850));

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
    compDef->SetSolderFillingMaterial(matCu->GetName());

    [[maybe_unused]] auto comp1 = eDataMgr.CreateComponent(topLayout, "M1", compDef, iLyrTopCu, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(0) , coordUnits.toCoord(0))), false);
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
    extSettings.grid = {xGrid, static_cast<size_t>(xGrid * EFloat(bbox.Width()) / bbox.Length())};
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
    matAl->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(238));
    matAl->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(880));
    matAl->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(2700));
    matAl->SetProperty(EMaterialPropId::Resistivity, eDataMgr.CreateSimpleMaterialProp(2.82e-8));

    auto matCu = database->CreateMaterialDef("Cu");
    matCu->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(398));
    matCu->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(380));
    matCu->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(8850));

    auto matAir = database->CreateMaterialDef("Air");
    matAir->SetMaterialType(EMaterialType::Fluid);
    matAir->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(0.026));
    matAir->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(1.003));
    matAir->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(1.225));

    auto matSiC = database->CreateMaterialDef("SiC");  
    matSiC->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(370));
    matSiC->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(750));
    matSiC->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(3210));

    auto matSi3N4 = database->CreateMaterialDef("Si3N4");
    matSi3N4->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(70));
    matSi3N4->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(691));
    matSi3N4->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(2400));

    auto matSolder = database->CreateMaterialDef("Sn-3.5Ag");
    matSolder->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(33));
    matSolder->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(200));
    matSolder->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(7360));
    matSolder->SetProperty(EMaterialPropId::Resistivity, eDataMgr.CreateSimpleMaterialProp(11.4e-8));

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
    compDef->SetSolderBallBumpHeight(100);
    compDef->SetSolderFillingMaterial(matSolder->GetName());
    compDef->SetBondingBox(EBox2D{-2000000, -2000000, 2000000, 2000000});
    compDef->SetMaterial(matSiC->GetName());
    compDef->SetHeight(365);

    bool flipped{false};
    [[maybe_unused]] auto comp1 = eDataMgr.CreateComponent(sicLayout, "M1", compDef, iLyrWire, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(3450) , coordUnits.toCoord(15000))), flipped);
    [[maybe_unused]] auto comp2 = eDataMgr.CreateComponent(sicLayout, "M2", compDef, iLyrWire, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(19000) , coordUnits.toCoord(15000))), flipped);
    assert(comp1);
    assert(comp2);
    comp1->SetLossPower(33.8);
    comp2->SetLossPower(31.9);
 
    //net
    auto gateNet = eDataMgr.CreateNet(sicLayout, "Gate");
    auto drainNet = eDataMgr.CreateNet(sicLayout, "Drain");
    auto sourceNet = eDataMgr.CreateNet(sicLayout, "Source");

    //wire
    FCoord bwRadius = 250;//um
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

    //bondwire
    auto sourceBW1 = eDataMgr.CreateBondwire(sicLayout, "SourceBW1", sourceNet->GetNetId(), {4450000, 16000000}, {3000000, 7500000}, bwRadius);
    sourceBW1->SetBondwireType(EBondwireType::JEDEC4);
    sourceBW1->SetStartComponent(comp1);
    sourceBW1->SetEndLayer(iLyrWire, false);
    sourceBW1->SetCurrent(20);
    
    auto sourceBW2 = eDataMgr.CreateBondwire(sicLayout, "SourceBW2", sourceNet->GetNetId(), {4450000, 15000000}, {3000000, 5000000}, bwRadius);
    sourceBW2->SetBondwireType(EBondwireType::JEDEC4);
    sourceBW2->SetStartComponent(comp1);
    sourceBW2->SetEndLayer(iLyrWire, false);
    sourceBW2->SetCurrent(20);
    
    auto sourceBW3 = eDataMgr.CreateBondwire(sicLayout, "SourceBW3", sourceNet->GetNetId(), {4450000, 14000000}, {3000000, 2500000}, bwRadius);
    sourceBW3->SetBondwireType(EBondwireType::JEDEC4);
    sourceBW3->SetStartComponent(comp1);
    sourceBW3->SetEndLayer(iLyrWire, false);
    sourceBW3->SetCurrent(20);

    auto sourceBW4 = eDataMgr.CreateBondwire(sicLayout, "SourceBW4", sourceNet->GetNetId(), {4450000, 16000000}, {18000000, 16000000}, bwRadius);
    sourceBW4->SetStartComponent(comp1);
    sourceBW4->SetEndComponent(comp2);
    sourceBW4->SetCurrent(10);

    auto sourceBW5 = eDataMgr.CreateBondwire(sicLayout, "SourceBW5", sourceNet->GetNetId(), {4450000, 15000000}, {18000000, 15000000}, bwRadius);
    sourceBW5->SetStartComponent(comp1);
    sourceBW5->SetEndComponent(comp2);
    sourceBW5->SetCurrent(10);

    auto sourceBW6 = eDataMgr.CreateBondwire(sicLayout, "SourceBW6", sourceNet->GetNetId(), {4450000, 14000000}, {18000000, 14000000}, bwRadius);
    sourceBW6->SetStartComponent(comp1);
    sourceBW6->SetEndComponent(comp2);
    sourceBW6->SetCurrent(10);

    auto drainBW1 = eDataMgr.CreateBondwire(sicLayout, "DrainBW1", drainNet->GetNetId(), {10800000, 12200000}, {19350000, 7500000}, bwRadius);
    drainBW1->SetStartLayer(iLyrWire, false);
    drainBW1->SetEndLayer(iLyrWire, false);

    auto drainBW2 = eDataMgr.CreateBondwire(sicLayout, "DrainBW2", drainNet->GetNetId(), {10800000, 10200000}, {19350000, 5500000}, bwRadius);
    drainBW2->SetStartLayer(iLyrWire, false);
    drainBW2->SetEndLayer(iLyrWire, false);

    auto drainBW3 = eDataMgr.CreateBondwire(sicLayout, "DrainBW3", drainNet->GetNetId(), {10800000,  8200000}, {19350000, 3500000}, bwRadius);
    drainBW3->SetStartLayer(iLyrWire, false);
    drainBW3->SetEndLayer(iLyrWire, false);

    auto drainBW4 = eDataMgr.CreateBondwire(sicLayout, "DrainBW4", drainNet->GetNetId(), {10800000,  6200000}, {19350000, 1500000}, bwRadius);
    drainBW4->SetStartLayer(iLyrWire, false);
    drainBW4->SetEndLayer(iLyrWire, false);

    auto gateBW1 = eDataMgr.CreateBondwire(sicLayout, "GateBW1", gateNet->GetNetId(), {2450000, 14000000}, {3250000, 24000000}, bwRadius);
    gateBW1->SetBondwireType(EBondwireType::JEDEC4); 
    gateBW1->SetStartComponent(comp1);
    gateBW1->SetEndLayer(iLyrWire, false);

    auto gateBW2 = eDataMgr.CreateBondwire(sicLayout, "GateBW2", gateNet->GetNetId(), {2450000, 16000000}, {5750000, 24000000}, bwRadius);
    gateBW2->SetBondwireType(EBondwireType::JEDEC4);
    gateBW2->SetStartComponent(comp1);
    gateBW2->SetEndLayer(iLyrWire, false);

    auto gateBW3 = eDataMgr.CreateBondwire(sicLayout, "GateBW3", gateNet->GetNetId(), {20000000, 16000000}, {19750000, 24000000}, bwRadius);
    gateBW3->SetBondwireType(EBondwireType::JEDEC4);
    gateBW3->SetStartComponent(comp2);
    gateBW3->SetEndLayer(iLyrWire, false);

    auto gateBW4 = eDataMgr.CreateBondwire(sicLayout, "GateBW4", gateNet->GetNetId(), {20000000, 14000000}, {22250000, 24000000}, bwRadius);
    gateBW4->SetBondwireType(EBondwireType::JEDEC4);
    gateBW4->SetStartComponent(comp2);
    gateBW4->SetEndLayer(iLyrWire, false);
    

    auto bondwireSolderDef = eDataMgr.CreatePadstackDef(database, "Bondwire Solder Joints");
    auto bondwireSolderDefData = eDataMgr.CreatePadstackDefData();
    bondwireSolderDefData->SetTopSolderBumpMaterial(matSolder->GetName());
    bondwireSolderDefData->SetBotSolderBallMaterial(matSolder->GetName());
    
    auto bumpR = bwRadius * 1.2 * 1e3;
    auto topBump = eDataMgr.CreateShapeCircle({0, 0}, bumpR);
    bondwireSolderDefData->SetTopSolderBumpParameters(std::move(topBump), 100);
    
    auto botBall = eDataMgr.CreateShapeCircle({0, 0}, bumpR);
    bondwireSolderDefData->SetBotSolderBallParameters(std::move(botBall), 100);

    bondwireSolderDef->SetPadstackDefData(std::move(bondwireSolderDefData));

    auto primIter = sicLayout->GetPrimitiveIter();
    while (auto * prim = primIter->Next()) {
        if (auto * bw = prim->GetBondwireFromPrimitive(); bw) {
            bw->SetSolderJoints(bondwireSolderDef);
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
    extSettings.grid = {xGrid, static_cast<size_t>(xGrid * EFloat(bbox.Width()) / bbox.Length())};
    extSettings.mergeGeomBeforeMetalMapping = false;

    esim::EThermalNetworkExtraction ne;
    ne.SetExtractionSettings(extSettings);
    // auto model = ne.GenerateGridThermalModel(layout);
    auto model = ne.GeneratePrismaThermalModel(layout, generic::math::Rad(20), 10, 1e5, 1e5);

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