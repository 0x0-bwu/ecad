#pragma once
#define BOOST_TEST_INCLUDED
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include "generic/tools/StringHelper.hpp"
#include "TestData.hpp"
#include "EDataMgr.h"
using namespace boost::unit_test;
using namespace ecad;

void t_thermal_static_flow1()
{
    EDataMgr::Instance().Init();
    auto & eDataMgr = EDataMgr::Instance();

    //database
    auto database = eDataMgr.CreateDatabase("Simple"); BOOST_CHECK(database);
    auto matCu = database->CreateMaterialDef("Cu"); BOOST_CHECK(matCu);
    matCu->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(398));
    matCu->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(380));
    matCu->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(8850));

    //coord units
    ECoordUnits coordUnits(ECoordUnits::Unit::Micrometer);
    database->SetCoordUnits(coordUnits);
    
    //top cell
    auto topCell = eDataMgr.CreateCircuitCell(database, "TopCell"); BOOST_CHECK(topCell);
    auto topLayout = topCell->GetLayoutView(); BOOST_CHECK(topLayout);
    auto topBouds = std::make_unique<EPolygon>(eDataMgr.CreatePolygon(coordUnits, {{-5000, -5000}, {86000, -5000}, {86000, 31000}, {-5000, 31000}}));
    topLayout->SetBoundary(std::move(topBouds));

    auto iLyrTopCu = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCu", ELayerType::ConductingLayer, 0, 400, matCu->GetName(), matCu->GetName()));
    BOOST_CHECK(iLyrTopCu != ELayerId::noLayer);

    //component
    auto compDef = eDataMgr.CreateComponentDef(database, "CPMF-1200-S080B Z-FET");
    BOOST_CHECK(compDef);
    compDef->SetBondingBox(eDataMgr.CreateBox(coordUnits, FPoint2D(-2000, -2000), FPoint2D(2000, 2000)));
    compDef->SetMaterial(matCu->GetName());
    compDef->SetHeight(365);
    compDef->SetSolderFillingMaterial(matCu->GetName());

    auto comp1 = eDataMgr.CreateComponent(topLayout, "M1", compDef, iLyrTopCu, makeETransform2D(1, 0, EVector2D(coordUnits.toCoord(0) , coordUnits.toCoord(0))), false);
    BOOST_CHECK(comp1);
    comp1->SetLossPower(ETemperature::Celsius2Kelvins(25), 33.8);

    //flatten
    database->Flatten(topCell, 1);
    auto layout = topCell->GetFlattenedLayoutView(); BOOST_CHECK(layout);

    ELayoutViewRendererSettings rendererSettings;
    rendererSettings.format = ELayoutViewRendererSettings::Format::PNG;
    rendererSettings.dirName = ecad_test::GetTestDataPath() + "/simulation/thermal";
    BOOST_CHECK(layout->Renderer(rendererSettings));
    
    ELayoutPolygonMergeSettings mergeSettings(4, {});
    mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    BOOST_CHECK(layout->MergeLayerPolygons(mergeSettings));

    size_t xGrid = 3;
    auto bbox = layout->GetBoundary()->GetBBox();
    EGridThermalModelExtractionSettings gridSettings(ecad_test::GetTestDataPath() + "/simulation/thermal", 4, {});
    gridSettings.dumpHotmaps = true;
    gridSettings.dumpDensityFile = true;
    gridSettings.dumpTemperatureFile = true;
    gridSettings.metalFractionMappingSettings.grid =  {xGrid, static_cast<size_t>(xGrid * EFloat(bbox.Width()) / bbox.Length())};
    gridSettings.metalFractionMappingSettings.mergeGeomBeforeMapping = false;
    gridSettings.botUniformBC.type = EThermalBondaryConditionType::HTC;
    gridSettings.botUniformBC.value = 2750;
    auto model1 = layout->ExtractThermalModel(gridSettings); BOOST_CHECK(model1);

    EPrismThermalModelExtractionSettings prismSettings(gridSettings.workDir, 4, {});
    auto model2 = layout->ExtractThermalModel(prismSettings); BOOST_CHECK(model2);
    EDataMgr::Instance().ShutDown();
}

void t_thermal_static_flow2()
{
    EDataMgr::Instance().Init();
    auto & eDataMgr = EDataMgr::Instance();

    //database
    auto database = eDataMgr.CreateDatabase("RobGrant"); BOOST_CHECK(database);
    auto matAl = database->CreateMaterialDef("Al"); BOOST_CHECK(matAl);
    matAl->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(238));
    matAl->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(880));
    matAl->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(2700));
    matAl->SetProperty(EMaterialPropId::Resistivity, eDataMgr.CreateSimpleMaterialProp(2.82e-8));

    auto matCu = database->CreateMaterialDef("Cu"); BOOST_CHECK(matCu);
    matCu->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreatePolynomialMaterialProp({{437.6, -0.165, 1.825e-4, -1.427e-7, 3.979e-11}}));
    matCu->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreatePolynomialMaterialProp({{342.8, 0.134, 5.535e-5, -1.971e-7, 1.141e-10}}));
    matCu->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(8850));

    auto matAir = database->CreateMaterialDef("Air"); BOOST_CHECK(matAir);
    matAir->SetMaterialType(EMaterialType::Fluid);
    matAir->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(0.026));
    matAir->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(1.003));
    matAir->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(1.225));

    auto matSiC = database->CreateMaterialDef("SiC"); BOOST_CHECK(matSiC);
    matSiC->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreatePolynomialMaterialProp({{1860, -11.7, 0.03442, -4.869e-5, 2.675e-8}}));
    matSiC->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreatePolynomialMaterialProp({{-3338, 33.12, -0.1037, 0.0001522, -8.553e-8}}));
    matSiC->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(3210));

    auto matSi3N4 = database->CreateMaterialDef("Si3N4"); BOOST_CHECK(matSi3N4);
    matSi3N4->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(70));
    matSi3N4->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(691));
    matSi3N4->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(2400));

    auto matSolder = database->CreateMaterialDef("Sn-3.5Ag"); BOOST_CHECK(matSolder);
    matSolder->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(33));
    matSolder->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(200));
    matSolder->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(7360));
    matSolder->SetProperty(EMaterialPropId::Resistivity, eDataMgr.CreateSimpleMaterialProp(11.4e-8));

    //coord units
    ECoordUnits coordUnits(ECoordUnits::Unit::Micrometer);
    database->SetCoordUnits(coordUnits);
    
    //top cell
    auto topCell = eDataMgr.CreateCircuitCell(database, "TopCell"); BOOST_CHECK(topCell);
    auto topLayout = topCell->GetLayoutView(); BOOST_CHECK(topLayout);
    auto topBouds = std::make_unique<EPolygon>(eDataMgr.CreatePolygon(coordUnits, {{-5000, -5000}, {86000, -5000}, {86000, 31000}, {-5000, 31000}}));
    topLayout->SetBoundary(std::move(topBouds));

    BOOST_CHECK(eDataMgr.CreateNet(topLayout, "Gate"));
    BOOST_CHECK(eDataMgr.CreateNet(topLayout, "Drain"));
    BOOST_CHECK(eDataMgr.CreateNet(topLayout, "Source"));

    //substrate
    auto iLyrTopCu = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCu", ELayerType::ConductingLayer, 0, 400, matCu->GetName(), matAir->GetName()));
    auto iLyrSubstrate = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("Substrate", ELayerType::DielectricLayer, -400, 635, matSi3N4->GetName(), matSi3N4->GetName()));
    auto iLyrCuPlate = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("CuPlate", ELayerType::ConductingLayer, -1035, 300, matCu->GetName(), matCu->GetName()));
    BOOST_CHECK(iLyrTopCu != ELayerId::noLayer);
    BOOST_CHECK(iLyrSubstrate != ELayerId::noLayer);
    BOOST_CHECK(iLyrCuPlate != ELayerId::noLayer);

    //sic die
    auto sicCell = eDataMgr.CreateCircuitCell(database, "SicDie");
    BOOST_CHECK(sicCell);
    auto sicLayout = sicCell->GetLayoutView();
    BOOST_CHECK(sicLayout);

    //boundary
    auto sicBonds = std::make_unique<EPolygon>(eDataMgr.CreatePolygon(coordUnits, {{0, 0}, {23000, 0}, {23000, 26000}, {0, 26000}}));
    sicLayout->SetBoundary(std::move(sicBonds));

    auto iLyrWire = sicLayout->AppendLayer(eDataMgr.CreateStackupLayer("Wire", ELayerType::ConductingLayer, 0, 400, matCu->GetName(), matAir->GetName()));
    BOOST_CHECK(iLyrWire != ELayerId::noLayer);

    //component
    auto compDef = eDataMgr.CreateComponentDef(database, "CPMF-1200-S080B Z-FET");
    BOOST_CHECK(compDef);
    compDef->SetSolderBallBumpHeight(100);
    compDef->SetSolderFillingMaterial(matSolder->GetName());
    compDef->SetBondingBox(eDataMgr.CreateBox(coordUnits, FPoint2D(-2000, -2000), FPoint2D(2000, 2000)));
    compDef->SetMaterial(matSiC->GetName());
    compDef->SetHeight(365);

    BOOST_CHECK(eDataMgr.CreateComponentDefPin(compDef, "Gate1", {-1000,  1000}, EPinIOType::Receiver));
    BOOST_CHECK(eDataMgr.CreateComponentDefPin(compDef, "Gate2", {-1000, -1000}, EPinIOType::Receiver));
    BOOST_CHECK(eDataMgr.CreateComponentDefPin(compDef, "Source1", {1000,  1000}, EPinIOType::Receiver));
    BOOST_CHECK(eDataMgr.CreateComponentDefPin(compDef, "Source2", {1000, -1000}, EPinIOType::Receiver));
    
    bool flipped{false};
    EFloat comp1x = 2000, comp1y = 12650;
    EFloat comp2x = 17750, comp2y = 12650; 
    auto comp1 = eDataMgr.CreateComponent(sicLayout, "M1", compDef, iLyrWire, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {comp1x, comp1y}), flipped);
    auto comp2 = eDataMgr.CreateComponent(sicLayout, "M2", compDef, iLyrWire, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {comp2x, comp2y}, EMirror2D::Y), flipped);
    BOOST_CHECK(comp1);
    BOOST_CHECK(comp2);
    comp1->SetLossPower(ETemperature::Celsius2Kelvins(25), 33.8);
    comp2->SetLossPower(ETemperature::Celsius2Kelvins(25), 31.9);
 
    //net
    auto gateNet = eDataMgr.CreateNet(sicLayout, "Gate"); BOOST_CHECK(gateNet);
    auto drainNet = eDataMgr.CreateNet(sicLayout, "Drain"); BOOST_CHECK(drainNet);
    auto sourceNet = eDataMgr.CreateNet(sicLayout, "Source"); BOOST_CHECK(sourceNet);

    //wire
    EFloat bwRadius = 150;//um
    std::vector<FPoint2D> ps1 {{0, 0}, {14200, 0}, {14200, 3500}, {5750, 3500}, {5750, 9150}, {0, 9150}};
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, sourceNet->GetNetId(), eDataMgr.CreateShapePolygon(coordUnits, std::move(ps1))));

    std::vector<FPoint2D> ps2 {{0, 10650}, {7300, 10650}, {7300, 5000}, {14300, 5000}, {14300, 19000}, {1450, 19000}, {1450, 26000}, {0, 26000}};
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, drainNet->GetNetId(), eDataMgr.CreateShapePolygon(coordUnits, std::move(ps2))));

    std::vector<FPoint2D> ps3 {{15750, 0}, {23000, 0}, {23000, 18850}, {18000, 18850}, {18000, 26000}, {14500, 26000}, {14500, 20500}, {15750, 20500}};
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, drainNet->GetNetId(), eDataMgr.CreateShapePolygon(coordUnits, std::move(ps3))));

    auto rec1 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(2500, 20500), FPoint2D(4000, 26000));
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec1)));

    auto rec2 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(5000, 20500), FPoint2D(6500, 26000));
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec2)));

    auto rec3 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(7500, 20500), FPoint2D(13500, 23000));
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec3)));

    auto rec4 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(7500, 24000), FPoint2D(10000, 26000));
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec4)));

    auto rec5 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(11000, 24000), FPoint2D(13500, 26000));
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec5)));

    auto rec6 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(19000, 20500), FPoint2D(20500, 26000));
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec6)));

    auto rec7 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(21500, 20500), FPoint2D(23000, 26000));
    BOOST_CHECK(eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec7)));

    //bondwire
    auto sourceBW1 = eDataMgr.CreateBondwire(sicLayout, "SourceBW1", sourceNet->GetNetId(), bwRadius);
    BOOST_CHECK(sourceBW1);
    sourceBW1->SetBondwireType(EBondwireType::JEDEC4);
    sourceBW1->SetStartComponent(comp1, "Source1");
    sourceBW1->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{2500, 8700}), false);
    sourceBW1->SetCurrent(10);

    auto sourceBW2 = eDataMgr.CreateBondwire(sicLayout, "SourceBW2", sourceNet->GetNetId(), bwRadius);
    BOOST_CHECK(sourceBW2);
    sourceBW2->SetBondwireType(EBondwireType::JEDEC4);
    sourceBW2->SetStartComponent(comp1, "Source2");
    sourceBW2->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{3500, 8700}), false);
    sourceBW2->SetCurrent(10);

    auto sourceBW3 = eDataMgr.CreateBondwire(sicLayout, "SourceBW3", sourceNet->GetNetId(), bwRadius);
    BOOST_CHECK(sourceBW3);
    sourceBW3->SetStartComponent(comp1, "Source1");
    sourceBW3->SetEndComponent(comp2, "Source1");
    sourceBW3->SetCurrent(10);

    auto sourceBW4 = eDataMgr.CreateBondwire(sicLayout, "SourceBW4", sourceNet->GetNetId(), bwRadius);
    BOOST_CHECK(sourceBW4);
    sourceBW4->SetStartComponent(comp1, "Source2");
    sourceBW4->SetEndComponent(comp2, "Source2");
    sourceBW4->SetCurrent(10);

    EFloat drainBWStartX = 13500, drainBWEndX = 16500;
    auto drainBW1 = eDataMgr.CreateBondwire(sicLayout, "DrainBW1", drainNet->GetNetId(), bwRadius);
    BOOST_CHECK(drainBW1);
    drainBW1->SetStartLayer(iLyrWire, coordUnits.toCoord(FPoint2D{drainBWStartX,  8200}), false);
    drainBW1->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{drainBWEndX, 3500}), false);

    auto drainBW2 = eDataMgr.CreateBondwire(sicLayout, "DrainBW2", drainNet->GetNetId(), bwRadius);
    BOOST_CHECK(drainBW2);
    drainBW2->SetStartLayer(iLyrWire, coordUnits.toCoord(FPoint2D{drainBWStartX,  6200}), false);
    drainBW2->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{drainBWEndX, 1500}), false);

    EFloat gateBWEndY{21000};
    auto gateBW1 = eDataMgr.CreateBondwire(sicLayout, "GateBW1", gateNet->GetNetId(), bwRadius);
    BOOST_CHECK(gateBW1);
    gateBW1->SetBondwireType(EBondwireType::JEDEC4); 
    gateBW1->SetStartComponent(comp1, "Gate1");
    gateBW1->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{3250, gateBWEndY}), false);

    auto gateBW2 = eDataMgr.CreateBondwire(sicLayout, "GateBW2", gateNet->GetNetId(), bwRadius);
    BOOST_CHECK(gateBW2);
    gateBW2->SetBondwireType(EBondwireType::JEDEC4);
    gateBW2->SetStartComponent(comp1, "Gate2");
    gateBW2->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{5750, gateBWEndY}), false);

    auto gateBW3 = eDataMgr.CreateBondwire(sicLayout, "GateBW3", gateNet->GetNetId(), bwRadius);
    BOOST_CHECK(gateBW3);
    gateBW3->SetBondwireType(EBondwireType::JEDEC4);
    gateBW3->SetStartComponent(comp2, "Gate1");
    gateBW3->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{19750, gateBWEndY}), false);

    auto gateBW4 = eDataMgr.CreateBondwire(sicLayout, "GateBW4", gateNet->GetNetId(), bwRadius);
    BOOST_CHECK(gateBW4);
    gateBW4->SetBondwireType(EBondwireType::JEDEC4);
    gateBW4->SetStartComponent(comp2, "Gate2");
    gateBW4->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{22250, gateBWEndY}), false);
    
    auto bondwireSolderDef = eDataMgr.CreatePadstackDef(database, "Bondwire Solder Joints");
    BOOST_CHECK(bondwireSolderDef);
    auto bondwireSolderDefData = eDataMgr.CreatePadstackDefData();
    BOOST_CHECK(bondwireSolderDefData);
    bondwireSolderDefData->SetTopSolderBumpMaterial(matSolder->GetName());
    bondwireSolderDefData->SetBotSolderBallMaterial(matSolder->GetName());
    
    auto bumpR = bwRadius * 1.1;
    auto topBump = eDataMgr.CreateShapeCircle(coordUnits,{0, 0}, bumpR); BOOST_CHECK(topBump);
    bondwireSolderDefData->SetTopSolderBumpParameters(std::move(topBump), 100);
    
    auto botBall = eDataMgr.CreateShapeCircle(coordUnits,{0, 0}, bumpR); BOOST_CHECK(botBall);
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
    BOOST_CHECK(layerMap);
    layerMap->SetMapping(iLyrWire, iLyrTopCu);

    //instance
    auto inst1 = eDataMgr.CreateCellInst(topLayout, "Inst1", sicLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {0, 0}));
    BOOST_CHECK(inst1);
    inst1->SetLayerMap(layerMap);

    auto inst2 = eDataMgr.CreateCellInst(topLayout, "Inst2", sicLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {29000, 0}));
    BOOST_CHECK(inst2); 
    inst2->SetLayerMap(layerMap);

    auto inst3 = eDataMgr.CreateCellInst(topLayout, "Inst3", sicLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {58000, 0}));
    BOOST_CHECK(inst3); 
    inst3->SetLayerMap(layerMap);

    //flatten
    database->Flatten(topCell, 1);
    auto layout = topCell->GetFlattenedLayoutView();
    BOOST_CHECK(layout);

    auto compIter = layout->GetComponentIter();
    while (auto * comp = compIter->Next()) {
        BOOST_CHECK(generic::str::StartsWith(comp->GetName(), "Inst"));
        if (generic::str::EndsWith(comp->GetName(), "M1"))
            comp->AddTransform(eDataMgr.CreateTransform2D(coordUnits, 1.0, 0.0, {0, 0})); //10300, 4350
        else comp->AddTransform(eDataMgr.CreateTransform2D(coordUnits, 1.0, 0.0, {3250, 4200})); //3250, 4200
    }

    ELayoutViewRendererSettings rendererSettings;
    rendererSettings.format = ELayoutViewRendererSettings::Format::PNG;
    rendererSettings.dirName = ecad_test::GetTestDataPath() + "/simulation/thermal";
    BOOST_CHECK(layout->Renderer(rendererSettings));
    
    ELayoutPolygonMergeSettings mergeSettings(4, {});
    mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    BOOST_CHECK(layout->MergeLayerPolygons(mergeSettings));

    EPrismThermalModelExtractionSettings prismSettings(ecad_test::GetTestDataPath() + "/simulation/thermal", 4, {});
    prismSettings.meshSettings.iteration = 1e5;
    prismSettings.meshSettings.minAlpha = 20;
    prismSettings.meshSettings.minLen = 1e-2;
    prismSettings.meshSettings.maxLen = 500;
    prismSettings.botUniformBC.type = EThermalBondaryConditionType::HTC;
    prismSettings.botUniformBC.value = 2750;

    EThermalStaticSimulationSetup setup(prismSettings.workDir, 4, {});
    setup.settings.iteration = 10;
    setup.settings.dumpHotmaps = true;
    setup.settings.envTemperature = {25, ETemperatureUnit::Celsius};
    setup.extractionSettings = std::make_unique<EPrismThermalModelExtractionSettings>(std::move(prismSettings));
    std::vector<EFloat> temperatures;
    auto [minT, maxT] = layout->RunThermalSimulation(setup, temperatures);    
    BOOST_CHECK_CLOSE(minT, 33.7, 2);
    BOOST_CHECK_CLOSE(maxT, 242, 2);
    EDataMgr::Instance().ShutDown();
}

test_suite * create_ecad_flow_test_suite()
{
    test_suite * simulation_suite = BOOST_TEST_SUITE("s_flow_test");
    //
    simulation_suite->add(BOOST_TEST_CASE(&t_thermal_static_flow1));
    simulation_suite->add(BOOST_TEST_CASE(&t_thermal_static_flow2));
    //
    return simulation_suite;
}