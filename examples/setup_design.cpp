#include <boost/stacktrace.hpp>
#include <string_view>
#include <filesystem>
#include <cassert>
#include <csignal>

#include "../test/TestData.hpp"
#include "simulation/thermal/EThermalSimulation.h"
#include "generic/tools/StringHelper.hpp"
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
    EPolygonWithHolesData pwh;
    pwh.outline = std::move(eDataMgr.CreatePolygon(coordUnits, {{-5000, -5000}, {86000, -5000}, {86000, 31000}, {-5000, 31000}}).shape);
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, {-4900, -4900}, 600)->GetContour());
    topLayout->SetBoundary(eDataMgr.CreateShapePolygonWithHoles(std::move(pwh)));

    [[maybe_unused]] auto iLyrTopCu = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCu", ELayerType::ConductingLayer, 0, 400, matCu->GetName(), matCu->GetName()));

    //component
    auto compDef = eDataMgr.CreateComponentDef(database, "CPMF-1200-S080B Z-FET");
    assert(compDef);
    compDef->SetBondingBox(eDataMgr.CreateBox(coordUnits, FPoint2D(-2000, -2000), FPoint2D(2000, 2000)));
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
    mergeSettings.threads = eDataMgr.Threads();
    mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->MergeLayerPolygons(mergeSettings);

    size_t xGrid = 3;
    auto bbox = layout->GetBoundary()->GetBBox();
    EGridThermalModelExtractionSettings gridSettings;
    gridSettings.workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    gridSettings.dumpHotmaps = true;
    gridSettings.dumpSpiceFile = true;
    gridSettings.dumpDensityFile = true;
    gridSettings.dumpTemperatureFile = true;
    gridSettings.metalFractionMappingSettings.grid =  {xGrid, static_cast<size_t>(xGrid * EFloat(bbox.Width()) / bbox.Length())};
    gridSettings.metalFractionMappingSettings.mergeGeomBeforeMapping = false;
    auto model1 = layout->ExtractThermalModel(gridSettings);

    EThermalTransientSimulationSetup transSimuSetup;
    transSimuSetup.settings.mor = false;
    transSimuSetup.environmentTemperature = 25;
    transSimuSetup.workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    EThermalTransientExcitation excitation = [](EFloat t){ return std::abs(std::sin(generic::math::pi * t / 0.5)); };
    transSimuSetup.settings.excitation = &excitation;
    layout->RunThermalSimulation(gridSettings, transSimuSetup);

    EPrismaThermalModelExtractionSettings prismaSettings;
    prismaSettings.workDir = gridSettings.workDir;
    auto model2 = layout->ExtractThermalModel(prismaSettings);

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
    EPolygonWithHolesData pwh;
    pwh.outline = std::move(eDataMgr.CreatePolygon(coordUnits, {{-5000, -5000}, {86000, -5000}, {86000, 31000}, {-5000, 31000}}).shape);
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, {-4000, -4000}, 750)->GetContour());
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, {85000, -4000}, 750)->GetContour());
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, {85000, 30000}, 750)->GetContour());
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, {-4000, 30000}, 750)->GetContour());
    topLayout->SetBoundary(eDataMgr.CreateShapePolygonWithHoles(std::move(pwh)));

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
    auto sicBonds = std::make_unique<EPolygon>(eDataMgr.CreatePolygon(coordUnits, {{0, 0}, {23000, 0}, {23000, 26000}, {0, 26000}}));
    sicLayout->SetBoundary(std::move(sicBonds));

    auto iLyrWire = sicLayout->AppendLayer(eDataMgr.CreateStackupLayer("Wire", ELayerType::ConductingLayer, 0, 400, matCu->GetName(), matAir->GetName()));
    assert(iLyrWire != ELayerId::noLayer);

    //component
    auto compDef = eDataMgr.CreateComponentDef(database, "CPMF-1200-S080B Z-FET");
    assert(compDef);
    compDef->SetSolderBallBumpHeight(100);
    compDef->SetSolderFillingMaterial(matSolder->GetName());
    compDef->SetBondingBox(eDataMgr.CreateBox(coordUnits, FPoint2D(-2000, -2000), FPoint2D(2000, 2000)));
    compDef->SetMaterial(matSiC->GetName());
    compDef->SetHeight(365);

    eDataMgr.CreateComponentDefPin(compDef, "Gate1", {-1000,  1000}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(compDef, "Gate2", {-1000, -1000}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(compDef, "Source1", {1000,  1000}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(compDef, "Source2", {1000, -1000}, EPinIOType::Receiver);
    
    bool flipped{false};
    EFloat comp1x = 2000, comp1y = 12650;
    EFloat comp2x = 17750, comp2y = 12650; 
    [[maybe_unused]] auto comp1 = eDataMgr.CreateComponent(sicLayout, "M1", compDef, iLyrWire, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {comp1x, comp1y}), flipped);
    [[maybe_unused]] auto comp2 = eDataMgr.CreateComponent(sicLayout, "M2", compDef, iLyrWire, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {comp2x, comp2y}, EMirror2D::Y), flipped);
    assert(comp1);
    assert(comp2);
    comp1->SetLossPower(33.8);
    comp2->SetLossPower(31.9);
 
    //net
    auto gateNet = eDataMgr.CreateNet(sicLayout, "Gate");
    auto drainNet = eDataMgr.CreateNet(sicLayout, "Drain");
    auto sourceNet = eDataMgr.CreateNet(sicLayout, "Source");

    //wire
    EFloat bwRadius = 250;//um
    std::vector<FPoint2D> ps1 {{0, 0}, {14200, 0}, {14200, 3500}, {5750, 3500}, {5750, 9150}, {0, 9150}};
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, sourceNet->GetNetId(), eDataMgr.CreateShapePolygon(coordUnits, std::move(ps1)));

    std::vector<FPoint2D> ps2 {{0, 10650}, {7300, 10650}, {7300, 5000}, {14300, 5000}, {14300, 19000}, {1450, 19000}, {1450, 26000}, {0, 26000}};
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, drainNet->GetNetId(), eDataMgr.CreateShapePolygon(coordUnits, std::move(ps2)));

    std::vector<FPoint2D> ps3 {{15750, 0}, {23000, 0}, {23000, 18850}, {18000, 18850}, {18000, 26000}, {14500, 26000}, {14500, 20500}, {15750, 20500}};
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, drainNet->GetNetId(), eDataMgr.CreateShapePolygon(coordUnits, std::move(ps3)));

    auto rec1 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(2500, 20500), FPoint2D(4000, 26000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec1));

    auto rec2 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(5000, 20500), FPoint2D(6500, 26000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec2));

    auto rec3 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(7500, 20500), FPoint2D(13500, 23000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec3));

    auto rec4 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(7500, 24000), FPoint2D(10000, 26000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec4));

    auto rec5 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(11000, 24000), FPoint2D(13500, 26000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec5));

    auto rec6 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(19000, 20500), FPoint2D(20500, 26000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec6));

    auto rec7 = eDataMgr.CreateShapeRectangle(coordUnits, FPoint2D(21500, 20500), FPoint2D(23000, 26000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, gateNet->GetNetId(), std::move(rec7));

    //bondwire
    auto sourceBW1 = eDataMgr.CreateBondwire(sicLayout, "SourceBW1", sourceNet->GetNetId(), bwRadius);
    sourceBW1->SetBondwireType(EBondwireType::JEDEC4);
    sourceBW1->SetStartComponent(comp1, "Source1");
    sourceBW1->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{2500, 8700}), false);
    sourceBW1->SetCurrent(20);

    auto sourceBW2 = eDataMgr.CreateBondwire(sicLayout, "SourceBW2", sourceNet->GetNetId(), bwRadius);
    sourceBW2->SetBondwireType(EBondwireType::JEDEC4);
    sourceBW2->SetStartComponent(comp1, "Source2");
    sourceBW2->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{3500, 8700}), false);
    sourceBW2->SetCurrent(20);

    auto sourceBW3 = eDataMgr.CreateBondwire(sicLayout, "SourceBW3", sourceNet->GetNetId(), bwRadius);
    sourceBW3->SetStartComponent(comp1, "Source1");
    sourceBW3->SetEndComponent(comp2, "Source1");
    sourceBW3->SetCurrent(10);

    auto sourceBW4 = eDataMgr.CreateBondwire(sicLayout, "SourceBW4", sourceNet->GetNetId(), bwRadius);
    sourceBW4->SetStartComponent(comp1, "Source2");
    sourceBW4->SetEndComponent(comp2, "Source2");
    sourceBW4->SetCurrent(10);

    EFloat drainBWStartX = 13500, drainBWEndX = 16500;
    auto drainBW1 = eDataMgr.CreateBondwire(sicLayout, "DrainBW1", drainNet->GetNetId(), bwRadius);
    drainBW1->SetStartLayer(iLyrWire, coordUnits.toCoord(FPoint2D{drainBWStartX,  8200}), false);
    drainBW1->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{drainBWEndX, 3500}), false);

    auto drainBW2 = eDataMgr.CreateBondwire(sicLayout, "DrainBW2", drainNet->GetNetId(), bwRadius);
    drainBW2->SetStartLayer(iLyrWire, coordUnits.toCoord(FPoint2D{drainBWStartX,  6200}), false);
    drainBW2->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{drainBWEndX, 1500}), false);

    EFloat gateBWEndY{21000};
    auto gateBW1 = eDataMgr.CreateBondwire(sicLayout, "GateBW1", gateNet->GetNetId(), bwRadius);
    gateBW1->SetBondwireType(EBondwireType::JEDEC4); 
    gateBW1->SetStartComponent(comp1, "Gate1");
    gateBW1->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{3250, gateBWEndY}), false);

    auto gateBW2 = eDataMgr.CreateBondwire(sicLayout, "GateBW2", gateNet->GetNetId(), bwRadius);
    gateBW2->SetBondwireType(EBondwireType::JEDEC4);
    gateBW2->SetStartComponent(comp1, "Gate2");
    gateBW2->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{5750, gateBWEndY}), false);

    auto gateBW3 = eDataMgr.CreateBondwire(sicLayout, "GateBW3", gateNet->GetNetId(), bwRadius);
    gateBW3->SetBondwireType(EBondwireType::JEDEC4);
    gateBW3->SetStartComponent(comp2, "Gate1");
    gateBW3->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{19750, gateBWEndY}), false);

    auto gateBW4 = eDataMgr.CreateBondwire(sicLayout, "GateBW4", gateNet->GetNetId(), bwRadius);
    gateBW4->SetBondwireType(EBondwireType::JEDEC4);
    gateBW4->SetStartComponent(comp2, "Gate2");
    gateBW4->SetEndLayer(iLyrWire, coordUnits.toCoord(FPoint2D{22250, gateBWEndY}), false);
    
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
    auto inst1 = eDataMgr.CreateCellInst(topLayout, "Inst1", sicLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {0, 0}));
    inst1->SetLayerMap(layerMap);

    auto inst2 = eDataMgr.CreateCellInst(topLayout, "Inst2", sicLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {29000, 0}));
    inst2->SetLayerMap(layerMap);

    auto inst3 = eDataMgr.CreateCellInst(topLayout, "Inst3", sicLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {58000, 0}));
    inst3->SetLayerMap(layerMap);

    //flatten
    database->Flatten(topCell);
    auto layout = topCell->GetFlattenedLayoutView();

    auto compIdxMap = std::unordered_map<size_t, std::string> {
            {0, "Inst1/M1"}, {1, "Inst2/M1"}, {2, "Inst3/M1"}, {3, "Inst1/M2"}, {4, "Inst2/M2"}, {5, "Inst3/M2"}
    };

    // std::vector<double> parameters{
    //     0.895351, 0.861775, 0.158995, 0.483945, 0.283657, 0.84234, 
    //     0.37849, 0.280513, 0.889224, 0.866983, 0.765416, 0.594158
    // };//max
    
    std::vector<double> parameters{
        0.10748, 0.446148, 0.305696, 0.454668, 0.331198, 0.495352,
        0.622851, 0.548639, 0.660065, 0.226427, 0.776296, 0.7115
    };//min

    for (size_t i = 0; i < 3; ++i) {
        auto comp = layout->FindComponentByName(compIdxMap.at(i));
        ECAD_TRACE("comp: %1%", comp->GetName())
        FVector2D shift(parameters[i * 2 + 0] * 10300, parameters[i * 2 + 1] * 4350);
        auto transform = EDataMgr::Instance().CreateTransform2D(coordUnits, 1.0, 0.0, shift);
        comp->AddTransform(transform);
    }

    for (size_t i = 3; i < 6; ++i) {
        auto comp = layout->FindComponentByName(compIdxMap.at(i));
        ECAD_TRACE("comp: %1%", comp->GetName())
        FVector2D shift(parameters[i * 2 + 0] * 3250, parameters[i * 2 + 1] * 4200);
        auto transform = EDataMgr::Instance().CreateTransform2D(coordUnits, 1.0, 0.0, shift);
        comp->AddTransform(transform);
    }

    ELayoutViewRendererSettings rendererSettings;
    rendererSettings.format = ELayoutViewRendererSettings::Format::PNG;
    rendererSettings.dirName = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->Renderer(rendererSettings);
    
    ELayoutPolygonMergeSettings mergeSettings;
    mergeSettings.threads = eDataMgr.Threads();
    mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->MergeLayerPolygons(mergeSettings);

    EPrismaThermalModelExtractionSettings prismaSettings;
    prismaSettings.workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    prismaSettings.meshSettings.iteration = 1e5;
    prismaSettings.meshSettings.minAlpha = 20;
    prismaSettings.meshSettings.minLen = 1e-2;
    prismaSettings.meshSettings.maxLen = 500;

    EThermalStaticSimulationSetup setup;
    setup.settings.dumpHotmaps = true;
    setup.environmentTemperature = 25;
    setup.workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    auto [minT, maxT] = layout->RunThermalSimulation(prismaSettings, setup);    
    ECAD_TRACE("minT: %1%, maxT: %2%", minT, maxT)
}

void test2()
{
    using namespace ecad;
    auto & eDataMgr = EDataMgr::Instance();

    //database
    auto database = eDataMgr.CreateDatabase("CREE62mm");

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
    ECoordUnits coordUnits(ECoordUnits::Unit::Millimeter);
    database->SetCoordUnits(coordUnits);

    auto bondwireSolderDef = eDataMgr.CreatePadstackDef(database, "Bondwire Solder Joints");
    auto bondwireSolderDefData = eDataMgr.CreatePadstackDefData();
    bondwireSolderDefData->SetTopSolderBumpMaterial(matSolder->GetName());
    bondwireSolderDefData->SetBotSolderBallMaterial(matSolder->GetName());
    
    EFloat bwRadius = 0.15;
    auto bumpR = bwRadius * 1.2;
    auto topBump = eDataMgr.CreateShapeCircle(coordUnits, {0, 0}, bumpR);
    bondwireSolderDefData->SetTopSolderBumpParameters(std::move(topBump), 0.1);
    
    auto botBall = eDataMgr.CreateShapeCircle(coordUnits, {0, 0}, bumpR);
    bondwireSolderDefData->SetBotSolderBallParameters(std::move(botBall), 0.1);

    bondwireSolderDef->SetPadstackDefData(std::move(bondwireSolderDefData));

    //component
    auto sicDie = eDataMgr.CreateComponentDef(database, "SicDie");
    sicDie->SetSolderBallBumpHeight(0.1);
    sicDie->SetSolderFillingMaterial(matSolder->GetName());
    sicDie->SetBondingBox(eDataMgr.CreateBox(coordUnits, FPoint2D(-2.545, -2.02), FPoint2D(2.545, 2.02)));
    sicDie->SetMaterial(matSiC->GetName());
    sicDie->SetHeight(0.18);

    auto pin1 = eDataMgr.CreateComponentDefPin(sicDie, "Pin1", {0, 1}, EPinIOType::Receiver);
    auto pin2 = eDataMgr.CreateComponentDefPin(sicDie, "Pin2", {1.25, 1}, EPinIOType::Receiver);
    auto pin3 = eDataMgr.CreateComponentDefPin(sicDie, "Pin3", {0, -1}, EPinIOType::Receiver);
    auto pin4 = eDataMgr.CreateComponentDefPin(sicDie, "Pin4", {1.25, -1}, EPinIOType::Receiver);

    auto r1 = eDataMgr.CreateComponentDef(database, "R1");
    r1->SetBondingBox(eDataMgr.CreateBox(coordUnits, FPoint2D(-1.1, -0.7), FPoint2D(1.1, 0.7)));
    r1->SetMaterial(matSiC->GetName());
    r1->SetHeight(0.5);

    auto r2 = eDataMgr.CreateComponentDef(database, "R2");
    r2->SetBondingBox(eDataMgr.CreateBox(coordUnits, FPoint2D(-1.05, -0.65), FPoint2D(1.05, 0.65)));
    r2->SetMaterial(matSiC->GetName());
    r2->SetHeight(0.5);

    //top cell
    auto baseCell = eDataMgr.CreateCircuitCell(database, "Base");
    auto baseLayout = baseCell->GetLayoutView();

    EPolygonWithHolesData pwh;
    pwh.outline = std::move(eDataMgr.CreatePolygon(coordUnits, {{-52.2, -29.7}, {52.2, -29.7}, {52.5, 29.7}, {-52.2, 29.7}}).shape);//R=5.3
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, {-46.5, -24}, 3.85)->GetContour());
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, { 46.5, -24}, 3.85)->GetContour());
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, { 46.5,  24}, 3.85)->GetContour());
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, {-46.5,  24}, 3.85)->GetContour());
    baseLayout->SetBoundary(eDataMgr.CreateShapePolygonWithHoles(std::move(pwh)));

    eDataMgr.CreateNet(baseLayout, "Gate");
    eDataMgr.CreateNet(baseLayout, "Drain");
    eDataMgr.CreateNet(baseLayout, "Source");

    //base
    [[maybe_unused]] auto topCuLayer = baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCuLayer", ELayerType::ConductingLayer, 0, 0.3, matCu->GetName(), matAir->GetName()));
    [[maybe_unused]] auto ceramicLayer = baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("CeramicLayer",  ELayerType::DielectricLayer, -0.3, 0.38, matSi3N4->GetName(), matAir->GetName()));
    [[maybe_unused]] auto botCuLayer = baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("BotCuLayer", ELayerType::ConductingLayer, -0.68, 0.3, matCu->GetName(), matAir->GetName()));
    [[maybe_unused]] auto solderLayer = baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("SolderLayer", ELayerType::ConductingLayer, -0.98, 0.1, matSolder->GetName(), matAir->GetName()));
    [[maybe_unused]] auto baseLayer = baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("BaseLayer", ELayerType::ConductingLayer, -1.08, 3, matCu->GetName(), matCu->GetName()));

    //driver
    auto driverCell = eDataMgr.CreateCircuitCell(database, "Driver");
    auto driverLayout = driverCell->GetLayoutView();

    //layer
    [[maybe_unused]] auto driverLayer1 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer1", ELayerType::DielectricLayer, -0.3, 0.38, matSi3N4->GetName(), matAir->GetName()));
    [[maybe_unused]] auto driverLayer2 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer2", ELayerType::ConductingLayer, -0.68, 0.3, matCu->GetName(), matAir->GetName()));
    [[maybe_unused]] auto driverLayer3 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer3", ELayerType::ConductingLayer, -0.98, 0.1, matSolder->GetName(), matAir->GetName()));
    [[maybe_unused]] auto driverLayer4 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer4", ELayerType::ConductingLayer, -0.98, 0.1, matSolder->GetName(), matAir->GetName()));

    //layer map
    auto driverLayerMap = eDataMgr.CreateLayerMap(database, "DriverLayerMap");
    driverLayerMap->SetMapping(driverLayer1, topCuLayer);
    driverLayerMap->SetMapping(driverLayer2, ceramicLayer);
    driverLayerMap->SetMapping(driverLayer3, botCuLayer);
    driverLayerMap->SetMapping(driverLayer4, solderLayer);

    //boundary   
    driverLayout->SetBoundary(eDataMgr.CreateShapePolygon(coordUnits, {{-5.5, -14.725}, {5.5, -14.725}, {5.5, 14.725}, {-5.5, 14.725}}));

    //net
    eDataMgr.CreateNet(driverLayout, "Gate");
    eDataMgr.CreateNet(driverLayout, "Drain");
    eDataMgr.CreateNet(driverLayout, "Source");

    //wire
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, 9.625}, {4.45, 9.625}, {4.45, 13.925}, {-4.7, 13.925}}));//R0.25
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, 4.325}, {4.45, 4.325}, {4.45,  8.625}, {-4.7,  8.625}}));//R0.25
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, -13.925}, {4.7, -13.925}, {4.7, 1.075}, {3.2, 1.075},
        {3.2, -1.775}, {4.2, -1.775}, {4.2, -4.925}, {3.2, -4.925}, {3.2, -7.025}, {4.2, -7.025}, {4.2, -11.425}, {-1.5, -11.425}, {-1.5, -9.725}, {-4.7, -9.725}})
    );//R0.25
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, -8.525}, {1.7, -8.525}, {1.7, -10.325}, {3.2, -10.325},
        {3.2, -8.225}, {2.2, -8.225}, {2.2, -3.875}, {3.2, -3.875}, {3.2, -2.825}, {2.2, -2.825}, {2.2, 2.175}, {4.7, 2.175}, {4.7, 3.225}, {1.7, 3.225}, {1.7, -4.325}, {-4.7, -4.325}})
    );//R0.25

    eDataMgr.CreateGeometry2D(driverLayout, driverLayer2, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-5.5, -14.725}, {5.5, -14.725}, {5.5, 14.725}, {-5.5, 14.725}}));
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, -13.925}, {4.7, -13.925}, {4.7, 13.925}, {-4.7, 13.925}}));//R0.25
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer4, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-5.5, -14.725}, {5.5, -14.725}, {5.5, 14.725}, {-5.5, 14.725}}));

    eDataMgr.CreateComponent(driverLayout, "R1", r1, driverLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, math::Rad(90), {2.75, 9.125}), false)->SetLossPower(5);

    //instance
    auto driverL = eDataMgr.CreateCellInst(baseLayout, "DriverL", driverLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-44, 0}));
    driverL->SetLayerMap(driverLayerMap);

    auto driverR = eDataMgr.CreateCellInst(baseLayout, "DriverR", driverLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {44, 0}, EMirror2D::XY));
    driverR->SetLayerMap(driverLayerMap);

    //bridge cell
    auto botBridgeCell = eDataMgr.CreateCircuitCell(database, "BotBridgeCell");
    auto botBridgeLayout = botBridgeCell->GetLayoutView();

    //layer
    [[maybe_unused]] auto botBridgeLayer1 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer1", ELayerType::DielectricLayer, -0.3, 0.38, matSi3N4->GetName(), matAir->GetName()));
    [[maybe_unused]] auto botBridgeLayer2 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer2", ELayerType::ConductingLayer, -0.68, 0.3, matCu->GetName(), matAir->GetName()));
    [[maybe_unused]] auto botBridgeLayer3 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer3", ELayerType::ConductingLayer, -0.98, 0.1, matSolder->GetName(), matAir->GetName()));
    [[maybe_unused]] auto botBridgeLayer4 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer4", ELayerType::ConductingLayer, -0.98, 0.1, matSolder->GetName(), matAir->GetName()));

    //layer map
    auto botBridgeLayerMap = eDataMgr.CreateLayerMap(database, "BotBridgeLayerMap");
    botBridgeLayerMap->SetMapping(botBridgeLayer1, topCuLayer);
    botBridgeLayerMap->SetMapping(botBridgeLayer2, ceramicLayer);
    botBridgeLayerMap->SetMapping(botBridgeLayer3, botCuLayer);
    botBridgeLayerMap->SetMapping(botBridgeLayer4, solderLayer);

    //boundary   
    botBridgeLayout->SetBoundary(eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    
    //net
    eDataMgr.CreateNet(botBridgeLayout, "Gate");
    eDataMgr.CreateNet(botBridgeLayout, "Drain");
    eDataMgr.CreateNet(botBridgeLayout, "Source");

    //wire
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -11.2}, {13.35, -11.2}, {13.95, -11.6}, {15.45, -11.6},
        {15.45, -10.8}, {-15.05, -10.8}, {-15.05, -1.3}, {-14.7, -0.7}, {-14.7, 11.45}, {-15.45, 11.45}})
    );//R0.25
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-14.2, -10.4}, {15.45, -10.4}, {15.45, -9.6}, {13.95, -9.6},
        {13.35, -10}, {-13.8, -10}, {-13.8, -2.55}, {-11.1, -2.55}, {-11.1, 11.45}, {-11.85, 11.45}, {-11.85, -2}, {-14.2, -2}})
    );//R0.25
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-12.6, -8.8}, {12.35, -8.8}, {12.95, -8.4}, {15.45, -8.4},
        {15.45, -5.97}, {7.95, -5.97}, {7.95, 9.03}, {15.45, 9.03}, {15.45, 11.45}, {-9.75, 11.45}, {-9.75, -3.75}, {-12.6, -3.75}})
    );//R0.25
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-13.65, -0.72}, {-12.9, -0.72}, {-12.9, 2.58}, {-13.65, 2.58}}));//R0.25
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-13.65, 3.6}, {-12.9, 3.6}, {-12.9, 6.9}, {-13.65, 6.9}}));//R0.25
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-13.65, 8.15}, {-12.9, 8.15}, {-12.9, 11.45}, {-13.65, 11.45}}));//R0.25

    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{9.5, -4.77}, {15.8, -4.77}, {15.8, 7.83}, {9.5, 7.83}}));//R0.25

    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer2, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.25, -12}, {16.25, -12}, {16.25, 12}, {-16.25, 12}}));//R0.25
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer4, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));

    auto botDie1 = eDataMgr.CreateComponent(botBridgeLayout, "BotDie1", sicDie, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-2.205, 0}), false);
    botDie1->SetLossPower(50);

    eDataMgr.CreateComponent(botBridgeLayout, "R2", r2, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-14.17, 0}), false)->SetLossPower(5);

    auto bw1 = eDataMgr.CreateBondwire(botBridgeLayout, "BW1", ENetId::noNet, bwRadius);
    bw1->SetStartComponent(botDie1, pin1->GetName());
    bw1->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{11.36, 5.94}), false);

    auto bw2 = eDataMgr.CreateBondwire(botBridgeLayout, "BW2", ENetId::noNet, bwRadius);
    bw2->SetStartComponent(botDie1, pin2->GetName());
    bw2->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{12.86, 5.94}), false);

    auto bw3 = eDataMgr.CreateBondwire(botBridgeLayout, "BW3", ENetId::noNet, bwRadius);
    bw3->SetStartComponent(botDie1, pin3->GetName());
    bw3->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{12.05, -3.74}), false);

    auto bw4 = eDataMgr.CreateBondwire(botBridgeLayout, "BW4", ENetId::noNet, bwRadius);
    bw4->SetStartComponent(botDie1, pin4->GetName());
    bw4->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{13.73, -2.46}), false);

    //instance
    auto botBridge1 = eDataMgr.CreateCellInst(baseLayout, "BotBridge1", botBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-17.75, 13}));
    botBridge1->SetLayerMap(botBridgeLayerMap);

    auto botBridge2 = eDataMgr.CreateCellInst(baseLayout, "BotBridge2", botBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-17.75, -13}, EMirror2D::X));
    botBridge2->SetLayerMap(botBridgeLayerMap);

    auto topBridgeCell = eDataMgr.CreateCircuitCell(database, "TopBridgeCell");
    auto topBridgeLayout = topBridgeCell->GetLayoutView();

    //layer
    [[maybe_unused]] auto topBridgeLayer1 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer1", ELayerType::DielectricLayer, -0.3, 0.38, matSi3N4->GetName(), matAir->GetName()));
    [[maybe_unused]] auto topBridgeLayer2 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer2", ELayerType::ConductingLayer, -0.68, 0.3, matCu->GetName(), matAir->GetName()));
    [[maybe_unused]] auto topBridgeLayer3 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer3", ELayerType::ConductingLayer, -0.98, 0.1, matSolder->GetName(), matAir->GetName()));
    [[maybe_unused]] auto topBridgeLayer4 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer4", ELayerType::ConductingLayer, -0.98, 0.1, matSolder->GetName(), matAir->GetName()));

    //layer map
    auto topBridgeLayerMap = eDataMgr.CreateLayerMap(database, "TopBridgeLayerMap");
    topBridgeLayerMap->SetMapping(topBridgeLayer1, topCuLayer);
    topBridgeLayerMap->SetMapping(topBridgeLayer2, ceramicLayer);
    topBridgeLayerMap->SetMapping(topBridgeLayer3, botCuLayer);
    topBridgeLayerMap->SetMapping(topBridgeLayer4, solderLayer);

    //boundary   
    topBridgeLayout->SetBoundary(eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    
    //net
    eDataMgr.CreateNet(topBridgeLayout, "Gate");
    eDataMgr.CreateNet(topBridgeLayout, "Drain");
    eDataMgr.CreateNet(topBridgeLayout, "Source");

    //wire
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -11.6}, {-13.95, -11.6}, {-13.35, -11.2}, {13.35, -11.2},
        {13.95, -11.6}, {15.45, -11.6}, {15.45, -10.8}, {-15.45, -10.8}})
    );//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -10.4}, {15.45, -10.4}, {15.45, -9.6}, {13.95, -9.6},
        {13.35, -10}, {-13.35, -10}, {-13.95, -9.6}, {-15.45, -9.6}})
    );//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -8.4}, {-12.95, -8.4}, {-12.35, -8.8}, {-9.15, -8.8},
        {-9.15, -3.1}, {-15.45, -3.1}})
    );//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -1.9}, {-7.95, -1.9}, {-7.95, -8.8}, {9.75, -8.8},
        {9.75, 11.45}, {-7.95, 11.45}, {-7.95, 4.45}, {-15.45, 4.45}})
    );//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, 5.65}, {-9.15, 5.65}, {-9.15, 11.45}, {-15.45, 11.45}}));//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{11.1, -8.5}, {11.85, -8.5}, {11.85, 11.45}, {11.1, 11.45}}));//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, -5.5}, {13.65, -5.5}, {13.65, -2.2}, {12.9, -2.2}}));//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, -0.7}, {13.65, -0.7}, {13.65, 2.6}, {12.9, 2.6}}));//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, 7.4}, {13.65, 7.4}, {13.65, 10.7}, {12.9, 10.7}}));//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, -8.5}, {15.45, -8.5}, {15.45, 11.45}, {14.7, 11.45},
        {14.7, -6.55}, {12.9, -6.55}})
    );//R0.25

    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer2, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.25, -12}, {16.25, -12}, {16.25, 12}, {-16.25, 12}}));//R0.25
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer4, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));

    auto topDie1 = eDataMgr.CreateComponent(topBridgeLayout, "TopDie1", sicDie, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {2.205, 0}, EMirror2D::Y), false);
    topDie1->SetLossPower(50);

    eDataMgr.CreateComponent(topBridgeLayout, "R3", r2, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {14.17, 0}), false)->SetLossPower(5);

    auto bw5 = eDataMgr.CreateBondwire(topBridgeLayout, "BW5", ENetId::noNet, bwRadius);
    bw5->SetStartComponent(topDie1, pin1->GetName());
    bw5->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{-10.98, 7.41}), false);

    auto bw6 = eDataMgr.CreateBondwire(topBridgeLayout, "BW6", ENetId::noNet, bwRadius);
    bw6->SetStartComponent(topDie1, pin2->GetName());
    bw6->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{-12.48, 7.41}), false);

    auto bw7 = eDataMgr.CreateBondwire(topBridgeLayout, "BW7", ENetId::noNet, bwRadius);
    bw7->SetStartComponent(topDie1, pin3->GetName());
    bw7->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{-10.98, -7.41}), false);

    auto bw8 = eDataMgr.CreateBondwire(topBridgeLayout, "BW8", ENetId::noNet, bwRadius);
    bw8->SetStartComponent(topDie1, pin4->GetName());
    bw8->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{-12.48, -7.41}), false);

    //instance
    auto topBridge1 = eDataMgr.CreateCellInst(baseLayout, "topBridge1", topBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {17.75, 13}));
    topBridge1->SetLayerMap(botBridgeLayerMap);

    auto topBridge2 = eDataMgr.CreateCellInst(baseLayout, "topBridge2", topBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {17.75, -13}, EMirror2D::X));
    topBridge2->SetLayerMap(botBridgeLayerMap);

    auto bw9 = eDataMgr.CreateBondwire(baseLayout, "BW9", ENetId::noNet, bwRadius);
    bw9->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 23.8}), false);
    bw9->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 23.8}), false);

    auto bw10 = eDataMgr.CreateBondwire(baseLayout, "BW10", ENetId::noNet, bwRadius);
    bw10->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 23}), false);
    bw10->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 23}), false);

    auto bw11 = eDataMgr.CreateBondwire(baseLayout, "BW11", ENetId::noNet, bwRadius);
    bw11->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 6.2}), false);
    bw11->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 6.2}), false);

    auto bw12 = eDataMgr.CreateBondwire(baseLayout, "BW12", ENetId::noNet, bwRadius);
    bw12->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 5.4}), false);
    bw12->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 5.4}), false);

    auto bw13 = eDataMgr.CreateBondwire(baseLayout, "BW13", ENetId::noNet, bwRadius);
    bw13->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 3}), false);
    bw13->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 3}), false);

    auto bw14 = eDataMgr.CreateBondwire(baseLayout, "BW14", ENetId::noNet, bwRadius);
    bw14->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 1.8}), false);
    bw14->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 1.8}), false);

    auto bw15 = eDataMgr.CreateBondwire(baseLayout, "BW15", ENetId::noNet, bwRadius);
    bw15->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -23.8}), false);
    bw15->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -23.8}), false);

    auto bw16 = eDataMgr.CreateBondwire(baseLayout, "BW16", ENetId::noNet, bwRadius);
    bw16->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -23}), false);
    bw16->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -23}), false);

    auto bw17 = eDataMgr.CreateBondwire(baseLayout, "BW17", ENetId::noNet, bwRadius);
    bw17->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -6.2}), false);
    bw17->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -6.2}), false);

    auto bw18 = eDataMgr.CreateBondwire(baseLayout, "BW18", ENetId::noNet, bwRadius);
    bw18->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -5.4}), false);
    bw18->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -5.4}), false);

    auto bw19 = eDataMgr.CreateBondwire(baseLayout, "BW19", ENetId::noNet, bwRadius);
    bw19->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -3}), false);
    bw19->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -3}), false);

    auto bw20 = eDataMgr.CreateBondwire(baseLayout, "BW20", ENetId::noNet, bwRadius);
    bw20->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -1.8}), false);
    bw20->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -1.8}), false);

    //flatten
    database->Flatten(baseCell);
    auto layout = baseCell->GetFlattenedLayoutView();

    auto primIter = baseLayout->GetPrimitiveIter();
    while (auto * prim = primIter->Next()) {
        if (auto * bw = prim->GetBondwireFromPrimitive(); bw) {
            bw->SetBondwireType(EBondwireType::Simple);
            bw->SetSolderJoints(bondwireSolderDef);
            bw->SetMaterial(matAl->GetName());
            bw->SetHeight(0.5);
        }
    }
      
    EPrismaThermalModelExtractionSettings prismaSettings;
    prismaSettings.workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    prismaSettings.meshSettings.iteration = 1e5;
    prismaSettings.meshSettings.minAlpha = 20;
    prismaSettings.meshSettings.minLen = 1e-5;
    prismaSettings.meshSettings.maxLen = 0.5;

    EThermalStaticSimulationSetup setup;
    setup.settings.dumpHotmaps = true;
    setup.environmentTemperature = 25;
    setup.workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    auto [minT, maxT] = layout->RunThermalSimulation(prismaSettings, setup);    
    ECAD_TRACE("minT: %1%, maxT: %2%", minT, maxT)
}

int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &SignalHandler);
    ::signal(SIGABRT, &SignalHandler);

    ecad::EDataMgr::Instance().Init(ecad::ELogLevel::Trace);
    // test0();
    // test1();
    test2();
    ecad::EDataMgr::Instance().ShutDown();
    return EXIT_SUCCESS;
}