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

using namespace ecad;
auto & eDataMgr = EDataMgr::Instance();
inline static constexpr EFloat BONDWIRE_RADIUS = 0.15;
void SetupMaterials(Ptr<IDatabase> database)
{
    auto matAl = eDataMgr.CreateMaterialDef(database, "Al");
    matAl->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(238));
    matAl->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(880));
    matAl->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(2700));
    matAl->SetProperty(EMaterialPropId::Resistivity, eDataMgr.CreateSimpleMaterialProp(2.82e-8));

    auto matCu = eDataMgr.CreateMaterialDef(database, "Cu");
    matCu->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(398));
    matCu->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(380));
    matCu->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(8850));

    auto matAir = eDataMgr.CreateMaterialDef(database, "Air");
    matAir->SetMaterialType(EMaterialType::Fluid);
    matAir->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(0.026));
    matAir->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(1.003));
    matAir->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(1.225));

    auto matSiC = eDataMgr.CreateMaterialDef(database, "SiC");  
    matSiC->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(370));
    matSiC->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(750));
    matSiC->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(3210));

    auto matSi3N4 = eDataMgr.CreateMaterialDef(database, "Si3N4");
    matSi3N4->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(70));
    matSi3N4->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(691));
    matSi3N4->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(2400));

    auto matSn35Ag = eDataMgr.CreateMaterialDef(database, "Sn-3.5Ag");
    matSn35Ag->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(33));
    matSn35Ag->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(200));
    matSn35Ag->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(7360));
    matSn35Ag->SetProperty(EMaterialPropId::Resistivity, eDataMgr.CreateSimpleMaterialProp(11.4e-8));
}

Ptr<IPadstackDef> CreateBondwireSolderJoints(Ptr<IDatabase> database, const std::string & name, EFloat bwRadius)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto def = eDataMgr.CreatePadstackDef(database, name);
    auto defData = eDataMgr.CreatePadstackDefData();
    defData->SetTopSolderBumpMaterial("Sn-3.5Ag");
    defData->SetBotSolderBallMaterial("Sn-3.5Ag");
    
    auto bumpR = bwRadius * 1.2;
    auto topBump = eDataMgr.CreateShapeCircle(coordUnits, {0, 0}, bumpR);
    defData->SetTopSolderBumpParameters(std::move(topBump), 0.1);
    
    auto botBall = eDataMgr.CreateShapeCircle(coordUnits, {0, 0}, bumpR);
    defData->SetBotSolderBallParameters(std::move(botBall), 0.1);

    def->SetPadstackDefData(std::move(defData));   
    return def;
}

Ptr<IComponentDef> CreateSicDieComponentDef(Ptr<IDatabase> database)
{
    auto sicDie = eDataMgr.CreateComponentDef(database, "SicDie");
    sicDie->SetSolderBallBumpHeight(0.1);
    sicDie->SetSolderFillingMaterial("Sn-3.5Ag");
    sicDie->SetBondingBox(eDataMgr.CreateBox(database->GetCoordUnits(), FPoint2D(-2.545, -2.02), FPoint2D(2.545, 2.02)));
    sicDie->SetMaterial("SiC");
    sicDie->SetHeight(0.18);

    eDataMgr.CreateComponentDefPin(sicDie, "G", {-1.25, 1}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "A", {-1.25, 0}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "B", {-1.25, -1}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "C", {1.25, 1}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "D", {1.25, 0}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "E", {1.25, -1}, EPinIOType::Receiver);
    return sicDie;
}

Ptr<IComponentDef> CreateDiodeComponentDef(Ptr<IDatabase> database)
{
    auto diode = eDataMgr.CreateComponentDef(database, "Diode");
    diode->SetSolderBallBumpHeight(0.1);
    diode->SetSolderFillingMaterial("Sn-3.5Ag");
    diode->SetBondingBox(eDataMgr.CreateBox(database->GetCoordUnits(), FPoint2D(-2.25, -2.25), FPoint2D(2.25, 2.25)));
    diode->SetMaterial("SiC");
    diode->SetHeight(0.18);

    eDataMgr.CreateComponentDefPin(diode, "A", {-1.125, 1.5}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(diode, "B", {-1.125, 0.75}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(diode, "C", {-1.125, 0}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(diode, "D", {-1.125, -0.75}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(diode, "E", {-1.125, -1.5}, EPinIOType::Receiver);

    eDataMgr.CreateComponentDefPin(diode, "F", {1.125, 1.5}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(diode, "G", {1.125, 0.75}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(diode, "H", {1.125, 0}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(diode, "I", {1.125, -0.75}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(diode, "G", {1.125, -1.5}, EPinIOType::Receiver);
    return diode;
}

Ptr<IComponentDef> CreateR1ComponentDef(Ptr<IDatabase> database)
{
    auto r1 = eDataMgr.CreateComponentDef(database, "R1");
    r1->SetBondingBox(eDataMgr.CreateBox(database->GetCoordUnits(), FPoint2D(-1.1, -0.7), FPoint2D(1.1, 0.7)));
    r1->SetMaterial("SiC");//wbtest
    r1->SetHeight(0.5);
    return r1;
}

Ptr<IComponentDef> CreateR2ComponentDef(Ptr<IDatabase> database)
{    auto r2 = eDataMgr.CreateComponentDef(database, "R2");
    r2->SetBondingBox(eDataMgr.CreateBox(database->GetCoordUnits(), FPoint2D(-1.05, -0.65), FPoint2D(1.05, 0.65)));
    r2->SetMaterial("SiC");//wbtest
    r2->SetHeight(0.5);
    return r2;
}

Ptr<ILayerMap> CreateDefaultLayerMap(Ptr<IDatabase> database, Ptr<ILayoutView> fromLayout, Ptr<ILayoutView> toLayout, const std::string & name)
{
    auto layerMap = eDataMgr.CreateLayerMap(database, name);
    auto fromIter = fromLayout->GetLayerIter();
    auto toIter = toLayout->GetLayerIter();
    auto fromLayer = fromIter->Next();
    auto toLayer = toIter->Next();
    while (fromLayer && toLayer) {
        layerMap->SetMapping(fromLayer->GetLayerId(), toLayer->GetLayerId());
        fromLayer = fromIter->Next();
        toLayer = toIter->Next();
    }
    return layerMap;
}
Ptr<ILayoutView> CreateBaseLayout(Ptr<IDatabase> database)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto baseCell = eDataMgr.CreateCircuitCell(database, "Base");
    auto baseLayout = baseCell->GetLayoutView();

    EPolygonWithHolesData pwh;
    pwh.outline = eDataMgr.CreateShapePolygon(database->GetCoordUnits(), {{-52.2, -29.7}, {52.2, -29.7}, {52.5, 29.7}, {-52.2, 29.7}}, 5.3)->GetContour();
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, {-46.5, -24}, 3.85)->GetContour());
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, { 46.5, -24}, 3.85)->GetContour());
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, { 46.5,  24}, 3.85)->GetContour());
    pwh.holes.emplace_back(eDataMgr.CreateShapeCircle(coordUnits, {-46.5,  24}, 3.85)->GetContour());
    baseLayout->SetBoundary(eDataMgr.CreateShapePolygonWithHoles(std::move(pwh)));

    eDataMgr.CreateNet(baseLayout, "Gate");
    eDataMgr.CreateNet(baseLayout, "Drain");
    eDataMgr.CreateNet(baseLayout, "Source");

    //base
    auto topCuLayer = baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCuLayer", ELayerType::ConductingLayer, 0, 0.3, "Cu", "Air"));
    baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("CeramicLayer",  ELayerType::DielectricLayer, -0.3, 0.38, "Si3N4", "Air"));
    baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("BotCuLayer", ELayerType::ConductingLayer, -0.68, 0.3, "Cu", "Air"));
    baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("SolderLayer", ELayerType::ConductingLayer, -0.98, 0.1, "Sn-3.5Ag", "Air"));
    baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("BaseLayer", ELayerType::ConductingLayer, -1.08, 3, "Cu", "Cu"));
    
    auto bw9 = eDataMgr.CreateBondwire(baseLayout, "BW9", ENetId::noNet, BONDWIRE_RADIUS);
    bw9->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 23.8}), false);
    bw9->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 23.8}), false);

    auto bw10 = eDataMgr.CreateBondwire(baseLayout, "BW10", ENetId::noNet, BONDWIRE_RADIUS);
    bw10->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 23}), false);
    bw10->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 23}), false);

    auto bw11 = eDataMgr.CreateBondwire(baseLayout, "BW11", ENetId::noNet, BONDWIRE_RADIUS);
    bw11->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 6.2}), false);
    bw11->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 6.2}), false);

    auto bw12 = eDataMgr.CreateBondwire(baseLayout, "BW12", ENetId::noNet, BONDWIRE_RADIUS);
    bw12->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 5.4}), false);
    bw12->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 5.4}), false);

    auto bw13 = eDataMgr.CreateBondwire(baseLayout, "BW13", ENetId::noNet, BONDWIRE_RADIUS);
    bw13->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 3}), false);
    bw13->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 3}), false);

    auto bw14 = eDataMgr.CreateBondwire(baseLayout, "BW14", ENetId::noNet, BONDWIRE_RADIUS);
    bw14->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, 1.8}), false);
    bw14->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, 1.8}), false);

    auto bw15 = eDataMgr.CreateBondwire(baseLayout, "BW15", ENetId::noNet, BONDWIRE_RADIUS);
    bw15->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -23.8}), false);
    bw15->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -23.8}), false);

    auto bw16 = eDataMgr.CreateBondwire(baseLayout, "BW16", ENetId::noNet, BONDWIRE_RADIUS);
    bw16->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -23}), false);
    bw16->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -23}), false);

    auto bw17 = eDataMgr.CreateBondwire(baseLayout, "BW17", ENetId::noNet, BONDWIRE_RADIUS);
    bw17->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -6.2}), false);
    bw17->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -6.2}), false);

    auto bw18 = eDataMgr.CreateBondwire(baseLayout, "BW18", ENetId::noNet, BONDWIRE_RADIUS);
    bw18->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -5.4}), false);
    bw18->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -5.4}), false);

    auto bw19 = eDataMgr.CreateBondwire(baseLayout, "BW19", ENetId::noNet, BONDWIRE_RADIUS);
    bw19->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -3}), false);
    bw19->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -3}), false);

    auto bw20 = eDataMgr.CreateBondwire(baseLayout, "BW20", ENetId::noNet, BONDWIRE_RADIUS);
    bw20->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D{-3, -1.8}), false);
    bw20->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D{3, -1.8}), false); 
    
    return baseLayout;
}

Ptr<ILayoutView> CreateDriverLayout(Ptr<IDatabase> database)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto driverCell = eDataMgr.CreateCircuitCell(database, "Driver");
    auto driverLayout = driverCell->GetLayoutView();

    //layer
    auto driverLayer1 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer1", ELayerType::DielectricLayer, -0.3, 0.38, "Si3N4", "Air"));
    auto driverLayer2 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer2", ELayerType::ConductingLayer, -0.68, 0.3, "Cu", "Air"));
    auto driverLayer3 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer3", ELayerType::ConductingLayer, -0.98, 0.1, "Sn-3.5Ag", "Air"));
    auto driverLayer4 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer4", ELayerType::ConductingLayer, -0.98, 0.1, "Sn-3.5Ag", "Air"));

    //boundary   
    driverLayout->SetBoundary(eDataMgr.CreateShapePolygon(coordUnits, {{-5.5, -14.725}, {5.5, -14.725}, {5.5, 14.725}, {-5.5, 14.725}}));

    //net
    eDataMgr.CreateNet(driverLayout, "Gate");
    eDataMgr.CreateNet(driverLayout, "Drain");
    eDataMgr.CreateNet(driverLayout, "Source");

    //wire
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, 9.625}, {4.7, 9.625}, {4.7, 13.925}, {-4.7, 13.925}}, 0.25));
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, 4.325}, {4.7, 4.325}, {4.7,  8.625}, {-4.7,  8.625}}, 0.25));
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, -13.925}, {4.7, -13.925}, {4.7, 1.075}, {3.2, 1.075},
        {3.2, -1.775}, {4.2, -1.775}, {4.2, -4.925}, {3.2, -4.925}, {3.2, -7.025}, {4.2, -7.025}, {4.2, -11.425}, {-1.5, -11.425}, {-1.5, -9.725}, {-4.7, -9.725}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, -8.525}, {1.7, -8.525}, {1.7, -10.325}, {3.2, -10.325},
        {3.2, -8.225}, {2.2, -8.225}, {2.2, -3.875}, {3.2, -3.875}, {3.2, -2.825}, {2.2, -2.825}, {2.2, 2.175}, {4.7, 2.175}, {4.7, 3.225}, {1.7, 3.225}, {1.7, -4.325}, {-4.7, -4.325}}, 0.25)
    );

    eDataMgr.CreateGeometry2D(driverLayout, driverLayer2, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-5.5, -14.725}, {5.5, -14.725}, {5.5, 14.725}, {-5.5, 14.725}}, 0.25));
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, -13.925}, {4.7, -13.925}, {4.7, 13.925}, {-4.7, 13.925}}, 0.25));
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer4, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-5.5, -14.725}, {5.5, -14.725}, {5.5, 14.725}, {-5.5, 14.725}}, 0.25));

    auto r1 = eDataMgr.FindComponentDefByName(database, "R1");
    eDataMgr.CreateComponent(driverLayout, "R1", r1, driverLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, math::Rad(90), {2.75, 9.125}), false)->SetLossPower(5);

    return driverLayout;
}

Ptr<ILayoutView> CreateBottomBridgeLayout(Ptr<IDatabase> database)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto botBridgeCell = eDataMgr.CreateCircuitCell(database, "BotBridgeCell");
    auto botBridgeLayout = botBridgeCell->GetLayoutView();

    //layer
    auto botBridgeLayer1 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer1", ELayerType::DielectricLayer, -0.3, 0.38, "Si3N4", "Air"));
    auto botBridgeLayer2 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer2", ELayerType::ConductingLayer, -0.68, 0.3, "Cu", "Air"));
    auto botBridgeLayer3 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer3", ELayerType::ConductingLayer, -0.98, 0.1, "Sn-3.5Ag", "Air"));
    auto botBridgeLayer4 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer4", ELayerType::ConductingLayer, -0.98, 0.1, "Sn-3.5Ag", "Air"));

    //boundary   
    botBridgeLayout->SetBoundary(eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    
    //net
    eDataMgr.CreateNet(botBridgeLayout, "Gate");
    eDataMgr.CreateNet(botBridgeLayout, "Drain");
    eDataMgr.CreateNet(botBridgeLayout, "Source");

    //wire
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -11.2}, {13.35, -11.2}, {13.95, -11.6}, {15.45, -11.6},
        {15.45, -10.8}, {-15.05, -10.8}, {-15.05, -1.3}, {-14.7, -0.7}, {-14.7, 11.45}, {-15.45, 11.45}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-14.2, -10.4}, {15.45, -10.4}, {15.45, -9.6}, {13.95, -9.6},
        {13.35, -10}, {-13.8, -10}, {-13.8, -2.55}, {-11.1, -2.55}, {-11.1, 11.45}, {-11.85, 11.45}, {-11.85, -2}, {-14.2, -2}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-12.6, -8.8}, {12.35, -8.8}, {12.95, -8.4}, {15.45, -8.4},
        {15.45, -5.97}, {7.95, -5.97}, {7.95, 9.03}, {15.45, 9.03}, {15.45, 11.45}, {-9.75, 11.45}, {-9.75, -3.75}, {-12.6, -3.75}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-13.65, -0.72}, {-12.9, -0.72}, {-12.9, 2.58}, {-13.65, 2.58}}, 0.25));
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-13.65, 3.6}, {-12.9, 3.6}, {-12.9, 6.9}, {-13.65, 6.9}}, 0.25));
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-13.65, 8.15}, {-12.9, 8.15}, {-12.9, 11.45}, {-13.65, 11.45}}, 0.25));

    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{9.5, -4.77}, {15.8, -4.77}, {15.8, 7.83}, {9.5, 7.83}}, 0.25));

    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer2, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}, 0.25));
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.25, -12}, {16.25, -12}, {16.25, 12}, {-16.25, 12}}, 0.25));
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer4, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}, 0.25));

    auto sicDie = eDataMgr.FindComponentDefByName(database, "SicDie");
    auto botDie1 = eDataMgr.CreateComponent(botBridgeLayout, "BotDie1", sicDie, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-2.205, 0}), false);
    botDie1->SetLossPower(50);

    auto r2 = eDataMgr.FindComponentDefByName(database, "R2");
    eDataMgr.CreateComponent(botBridgeLayout, "R2", r2, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-14.17, 0}), false)->SetLossPower(5);

    auto bw1 = eDataMgr.CreateBondwire(botBridgeLayout, "BW1", ENetId::noNet, BONDWIRE_RADIUS);
    bw1->SetStartComponent(botDie1, "A");
    bw1->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{11.36, 5.94}), false);

    auto bw2 = eDataMgr.CreateBondwire(botBridgeLayout, "BW2", ENetId::noNet, BONDWIRE_RADIUS);
    bw2->SetStartComponent(botDie1, "B");
    bw2->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{12.86, 5.94}), false);

    auto bw3 = eDataMgr.CreateBondwire(botBridgeLayout, "BW3", ENetId::noNet, BONDWIRE_RADIUS);
    bw3->SetStartComponent(botDie1, "C");
    bw3->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{12.05, -3.74}), false);

    auto bw4 = eDataMgr.CreateBondwire(botBridgeLayout, "BW4", ENetId::noNet, BONDWIRE_RADIUS);
    bw4->SetStartComponent(botDie1, "D");
    bw4->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{13.73, -2.46}), false);

    return botBridgeLayout;
}

Ptr<ILayoutView> CreateTopBridgeLayout(Ptr<IDatabase> database)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto topBridgeCell = eDataMgr.CreateCircuitCell(database, "TopBridgeCell");
    auto topBridgeLayout = topBridgeCell->GetLayoutView();

    //layer
    auto topBridgeLayer1 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer1", ELayerType::DielectricLayer, -0.3, 0.38,"Si3N4", "Air"));
    auto topBridgeLayer2 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer2", ELayerType::ConductingLayer, -0.68, 0.3, "Cu", "Air"));
    auto topBridgeLayer3 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer3", ELayerType::ConductingLayer, -0.98, 0.1, "Sn-3.5Ag", "Air"));
    auto topBridgeLayer4 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer4", ELayerType::ConductingLayer, -0.98, 0.1, "Sn-3.5Ag", "Air"));

    //boundary   
    topBridgeLayout->SetBoundary(eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    
    //net
    eDataMgr.CreateNet(topBridgeLayout, "Gate");
    eDataMgr.CreateNet(topBridgeLayout, "Drain");
    eDataMgr.CreateNet(topBridgeLayout, "Source");

    //wire
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -11.6}, {-13.95, -11.6}, {-13.35, -11.2}, {13.35, -11.2},
        {13.95, -11.6}, {15.45, -11.6}, {15.45, -10.8}, {-15.45, -10.8}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -10.4}, {15.45, -10.4}, {15.45, -9.6}, {13.95, -9.6},
        {13.35, -10}, {-13.35, -10}, {-13.95, -9.6}, {-15.45, -9.6}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -8.4}, {-12.95, -8.4}, {-12.35, -8.8}, {-9.15, -8.8},
        {-9.15, -3.1}, {-15.45, -3.1}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, -1.9}, {-7.95, -1.9}, {-7.95, -8.8}, {9.75, -8.8},
        {9.75, 11.45}, {-7.95, 11.45}, {-7.95, 4.45}, {-15.45, 4.45}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-15.45, 5.65}, {-9.15, 5.65}, {-9.15, 11.45}, {-15.45, 11.45}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{11.1, -8.5}, {11.85, -8.5}, {11.85, 11.45}, {11.1, 11.45}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, -5.5}, {13.65, -5.5}, {13.65, -2.2}, {12.9, -2.2}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, -0.7}, {13.65, -0.7}, {13.65, 2.6}, {12.9, 2.6}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, 7.4}, {13.65, 7.4}, {13.65, 10.7}, {12.9, 10.7}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, -8.5}, {15.45, -8.5}, {15.45, 11.45}, {14.7, 11.45},
        {14.7, -6.55}, {12.9, -6.55}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer2, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.25, -12}, {16.25, -12}, {16.25, 12}, {-16.25, 12}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer4, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));

    auto sicDie = eDataMgr.FindComponentDefByName(database, "SicDie");
    auto topDie1 = eDataMgr.CreateComponent(topBridgeLayout, "TopDie1", sicDie, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {2.205, 0}, EMirror2D::Y), false);
    topDie1->SetLossPower(50);

    auto r2 = eDataMgr.FindComponentDefByName(database, "R2");
    eDataMgr.CreateComponent(topBridgeLayout, "R3", r2, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {14.17, 0}), false)->SetLossPower(5);

    auto bw5 = eDataMgr.CreateBondwire(topBridgeLayout, "BW5", ENetId::noNet, BONDWIRE_RADIUS);
    bw5->SetStartComponent(topDie1, "A");
    bw5->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{-10.98, 7.41}), false);

    auto bw6 = eDataMgr.CreateBondwire(topBridgeLayout, "BW6", ENetId::noNet, BONDWIRE_RADIUS);
    bw6->SetStartComponent(topDie1, "B");
    bw6->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{-12.48, 7.41}), false);

    auto bw7 = eDataMgr.CreateBondwire(topBridgeLayout, "BW7", ENetId::noNet, BONDWIRE_RADIUS);
    bw7->SetStartComponent(topDie1, "C");
    bw7->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{-10.98, -7.41}), false);

    auto bw8 = eDataMgr.CreateBondwire(topBridgeLayout, "BW8", ENetId::noNet, BONDWIRE_RADIUS);
    bw8->SetStartComponent(topDie1, "D");
    bw8->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{-12.48, -7.41}), false);

    return topBridgeLayout;
}

void test2()
{
    //database
    auto database = eDataMgr.CreateDatabase("CREE62mm");

    //material
    SetupMaterials(database);
    
    //coord units
    ECoordUnits coordUnits(ECoordUnits::Unit::Millimeter);
    database->SetCoordUnits(coordUnits);

    auto bondwireSolderDef = CreateBondwireSolderJoints(database, "Source Solder Joints", BONDWIRE_RADIUS);

    //component
    auto sicDie = CreateSicDieComponentDef(database);
    auto r1 = CreateR1ComponentDef(database);
    auto r2 = CreateR2ComponentDef(database);

    auto baseLayout = CreateBaseLayout(database);
    auto driverLayout = CreateDriverLayout(database);
    auto driverLayerMap = CreateDefaultLayerMap(database, driverLayout, baseLayout, "DriverLayerMap");
    
    //instance
    auto driverL = eDataMgr.CreateCellInst(baseLayout, "DriverL", driverLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-44, 0}));
    driverL->SetLayerMap(driverLayerMap);
    auto driverR = eDataMgr.CreateCellInst(baseLayout, "DriverR", driverLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {44, 0}, EMirror2D::XY));
    driverR->SetLayerMap( driverLayerMap);

    auto botBridgeLayout = CreateBottomBridgeLayout(database);
    auto botBridgeLayerMap = CreateDefaultLayerMap(database, botBridgeLayout, baseLayout, "BotBridgeLayerMap");

    //instance
    auto botBridge1 = eDataMgr.CreateCellInst(baseLayout, "BotBridge1", botBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-17.75, 13}));
    botBridge1->SetLayerMap(botBridgeLayerMap);

    auto botBridge2 = eDataMgr.CreateCellInst(baseLayout, "BotBridge2", botBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-17.75, -13}, EMirror2D::X));
    botBridge2->SetLayerMap(botBridgeLayerMap);

    auto topBridgeLayout = CreateTopBridgeLayout(database);
    auto topBridgeLayerMap = CreateDefaultLayerMap(database, topBridgeLayout, baseLayout, "TopBridgeLayerMap");

    //instance
    auto topBridge1 = eDataMgr.CreateCellInst(baseLayout, "topBridge1", topBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {17.75, 13}));
    topBridge1->SetLayerMap(botBridgeLayerMap);

    auto topBridge2 = eDataMgr.CreateCellInst(baseLayout, "topBridge2", topBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {17.75, -13}, EMirror2D::X));
    topBridge2->SetLayerMap(botBridgeLayerMap);

    //flatten
    baseLayout->Flatten(EFlattenOption{});
    auto primIter = baseLayout->GetPrimitiveIter();
    while (auto * prim = primIter->Next()) {
        if (auto * bw = prim->GetBondwireFromPrimitive(); bw) {
            bw->SetBondwireType(EBondwireType::Simple);
            bw->SetSolderJoints(bondwireSolderDef);
            bw->SetMaterial("Al");
            bw->SetHeight(0.5);
        }
    }
      
    EPrismaThermalModelExtractionSettings prismaSettings;
    prismaSettings.workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    prismaSettings.meshSettings.iteration = 1e5;
    prismaSettings.meshSettings.minAlpha = 20;
    prismaSettings.meshSettings.minLen = 1e-4;
    prismaSettings.meshSettings.maxLen = 0.5;
    prismaSettings.meshSettings.tolerance = 1e-6;

    EThermalStaticSimulationSetup setup;
    setup.settings.dumpHotmaps = true;
    setup.environmentTemperature = 25;
    setup.workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    auto [minT, maxT] = baseLayout->RunThermalSimulation(prismaSettings, setup);    
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