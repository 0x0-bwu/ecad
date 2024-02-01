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
inline static constexpr EFloat THIN_BONDWIRE_RADIUS = 0.0635;
inline static constexpr EFloat THICK_BONDWIRE_RADIUS = 0.15;
inline static constexpr std::string_view RES_GATE = "Rg";
inline static constexpr std::string_view NET_GATE = "Gate";
inline static constexpr std::string_view NET_DRAIN = "Drain";
inline static constexpr std::string_view NET_SOURCE = "Source";

inline static constexpr std::string_view MAT_AL = "Al";
inline static constexpr std::string_view MAT_CU = "Cu";
inline static constexpr std::string_view MAT_AIR = "Air";
inline static constexpr std::string_view MAT_ALN = "AlN";
inline static constexpr std::string_view MAT_SIC = "SiC";
inline static constexpr std::string_view MAT_SAC305 = "SAC305";

void SetupMaterials(Ptr<IDatabase> database)
{
    auto matAl = eDataMgr.CreateMaterialDef(database, MAT_AL.data());
    matAl->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(238));
    matAl->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(880));
    matAl->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(2700));
    matAl->SetProperty(EMaterialPropId::Resistivity, eDataMgr.CreateSimpleMaterialProp(2.82e-8));

    auto matCu = eDataMgr.CreateMaterialDef(database, MAT_CU.data());
    matCu->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreatePolynomialMaterialProp({{437.6, -0.165, 1.825e-4, -1.427e-7, 3.979e-11}}));
    matCu->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreatePolynomialMaterialProp({{342.8, 0.134, 5.535e-5, -1.971e-7, 1.141e-10}}));
    matCu->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(8850));

    auto matAir = eDataMgr.CreateMaterialDef(database, MAT_AIR.data());
    matAir->SetMaterialType(EMaterialType::Fluid);
    matAir->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(0.026));
    matAir->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(1.003));
    matAir->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(1.225));

    auto matSiC = eDataMgr.CreateMaterialDef(database, MAT_SIC.data());  
    matSiC->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreatePolynomialMaterialProp({{1860, -11.7, 0.03442, -4.869e-5, 2.675e-8}}));
    matSiC->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreatePolynomialMaterialProp({{-3338, 33.12, -0.1037, 0.0001522, -8.553e-8}}));
    matSiC->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(3210));

    auto matAlN= eDataMgr.CreateMaterialDef(database, MAT_ALN.data());
    matAlN->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreatePolynomialMaterialProp({{421.7867, -1.1262, 0.001}}));
    matAlN->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreatePolynomialMaterialProp({{170.2, -2.018, 0.032, -8.957e-5, 1.032e-7, -4.352e-11}}));
    matAlN->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(3260));

    auto matSolder = eDataMgr.CreateMaterialDef(database, MAT_SAC305.data());
    matSolder->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreateSimpleMaterialProp(55));
    matSolder->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreateSimpleMaterialProp(218));
    matSolder->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(7800));
    matSolder->SetProperty(EMaterialPropId::Resistivity, eDataMgr.CreateSimpleMaterialProp(11.4e-8));
}

Ptr<IPadstackDef> CreateBondwireSolderJoints(Ptr<IDatabase> database, const std::string & name, EFloat bwRadius)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto def = eDataMgr.CreatePadstackDef(database, name);
    auto defData = eDataMgr.CreatePadstackDefData();
    defData->SetTopSolderBumpMaterial(MAT_SAC305.data());
    defData->SetBotSolderBallMaterial(MAT_SAC305.data());
    
    auto bumpR = bwRadius * 1.1;
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
    sicDie->SetSolderFillingMaterial(MAT_SAC305.data());
    sicDie->SetBondingBox(eDataMgr.CreateBox(database->GetCoordUnits(), FPoint2D(-2.545, -2.02), FPoint2D(2.545, 2.02)));
    sicDie->SetMaterial(MAT_SIC.data());
    sicDie->SetHeight(0.18);

    eDataMgr.CreateComponentDefPin(sicDie, "G", {-1.25, 1}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "B", {-1.25, 0}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "D", {-1.25, -1}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "A", {1.25, 1}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "C", {1.25, 0}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "E", {1.25, -1}, EPinIOType::Receiver);
    eDataMgr.CreateComponentDefPin(sicDie, "K", {-2, -0.5}, EPinIOType::Receiver);
    return sicDie;
}

Ptr<IComponentDef> CreateDiodeComponentDef(Ptr<IDatabase> database)
{
    auto diode = eDataMgr.CreateComponentDef(database, "Diode");
    diode->SetSolderBallBumpHeight(0.1);
    diode->SetSolderFillingMaterial(MAT_SAC305.data());
    diode->SetBondingBox(eDataMgr.CreateBox(database->GetCoordUnits(), FPoint2D(-2.25, -2.25), FPoint2D(2.25, 2.25)));
    diode->SetMaterial(MAT_SIC.data());
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
    eDataMgr.CreateComponentDefPin(diode, "J", {1.125, -1.5}, EPinIOType::Receiver);
    return diode;
}

Ptr<IComponentDef> CreateR1ComponentDef(Ptr<IDatabase> database)
{
    auto r1 = eDataMgr.CreateComponentDef(database, "R1");
    r1->SetBondingBox(eDataMgr.CreateBox(database->GetCoordUnits(), FPoint2D(-1.1, -0.7), FPoint2D(1.1, 0.7)));
    r1->SetMaterial(MAT_SIC.data());//wbtest
    r1->SetHeight(0.5);
    return r1;
}

Ptr<IComponentDef> CreateGateResistanceComponentDef(Ptr<IDatabase> database)
{  
    auto r = eDataMgr.CreateComponentDef(database, RES_GATE.data());
    r->SetBondingBox(eDataMgr.CreateBox(database->GetCoordUnits(), FPoint2D(-1.05, -0.65), FPoint2D(1.05, 0.65)));
    r->SetMaterial(MAT_SIC.data());//wbtest
    r->SetHeight(0.5);
    return r;
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

    auto ng = eDataMgr.CreateNet(baseLayout, "Gate");
    auto nd = eDataMgr.CreateNet(baseLayout, "Drain");
    auto ns = eDataMgr.CreateNet(baseLayout, "Source");
    auto nk = eDataMgr.CreateNet(baseLayout, "Kelvin");

    //base
    auto topCuLayer = baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCuLayer", ELayerType::ConductingLayer, 0, 0.3, MAT_CU.data(), MAT_AIR.data()));
    baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("CeramicLayer",  ELayerType::DielectricLayer, -0.3, 0.38, MAT_ALN.data(), MAT_AIR.data()));
    baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("BotCuLayer", ELayerType::ConductingLayer, -0.68, 0.3, MAT_CU.data(), MAT_AIR.data()));
    baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("SolderLayer", ELayerType::ConductingLayer, -0.98, 0.1, MAT_SAC305.data(), MAT_AIR.data()));
    baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("BaseLayer", ELayerType::ConductingLayer, -1.08, 3, MAT_CU.data(), MAT_CU.data()));
    
    std::vector<FPoint2D> dPLoc{
        {-3, 24}, {-3, 23.275}, {-3, 22.55}, {-4, 23.275}, {-4, 22.55}, {-3, 6.525}, {-3, 5.8}, {-3, 5.075}, {-4, 6.525}, {-4, 5.8},
    };
    std::vector<FPoint2D> sPLoc{
        {3, 24}, {3, 23.275}, {3, 22.55}, {4, 21.825}, {3, 21.825}, {3, 6.525}, {3, 5.8}, {3, 5.075}, {3, 7.25}, {4, 7.25},

    };
    for (size_t i = 0; i < dPLoc.size(); ++i) {
        auto bw1 = eDataMgr.CreateBondwire(baseLayout, "DS1_" + std::to_string(i + 1), ENetId::noNet, THICK_BONDWIRE_RADIUS);
        bw1->SetStartLayer(topCuLayer, coordUnits.toCoord(dPLoc.at(i)), false);
        bw1->SetEndLayer(topCuLayer, coordUnits.toCoord(sPLoc.at(i)), false); 

        dPLoc[i][1] *= -1; sPLoc[i][1] *= -1;
        auto bw2 = eDataMgr.CreateBondwire(baseLayout, "DS2_" + std::to_string(i + 1), ENetId::noNet, THICK_BONDWIRE_RADIUS);
        bw2->SetStartLayer(topCuLayer, coordUnits.toCoord(dPLoc.at(i)), false);
        bw2->SetEndLayer(topCuLayer, coordUnits.toCoord(sPLoc.at(i)), false); 
    }
    std::vector<FPoint2D> gPLoc{{-3, 3}, {-3, 1.8}};
    for (size_t i = 0; i < gPLoc.size(); ++i) {
        const auto & p = gPLoc.at(i);
        auto bw1 = eDataMgr.CreateBondwire(baseLayout, "G1_" + std::to_string(i + 1), ENetId::noNet, THIN_BONDWIRE_RADIUS);
        bw1->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(p[0], p[1])), false);
        bw1->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(-p[0], p[1])), false); 

        auto bw2 = eDataMgr.CreateBondwire(baseLayout, "G2_" + std::to_string(i + 1), ENetId::noNet, THIN_BONDWIRE_RADIUS);
        bw2->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(p[0], -p[1])), false);
        bw2->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(-p[0], -p[1])), false); 
    }

    auto KelvinBw0 = eDataMgr.CreateBondwire(baseLayout, "KelvinBw0", nk->GetNetId(), THIN_BONDWIRE_RADIUS);
    KelvinBw0->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(30.15, -5.95)), false);
    KelvinBw0->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(40.05, -5.95)), false);

    auto KelvinBw = eDataMgr.CreateBondwire(baseLayout, "KelvinBw", nk->GetNetId(), THIN_BONDWIRE_RADIUS);
    KelvinBw->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(30.15, 5)), false);
    KelvinBw->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(30.15, -5)), false);

    auto gateBw0 = eDataMgr.CreateBondwire(baseLayout, "GateBw0", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw0->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(32, -12.375)), false);
    gateBw0->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(40.05, -12.375)), false);

    auto gateBw = eDataMgr.CreateBondwire(baseLayout, "GateBw", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(32, 5)), false);
    gateBw->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(32, -5)), false);

    auto gateBw1 = eDataMgr.CreateBondwire(baseLayout, "GateBw1", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw1->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(32.5, 3.0)), false);
    gateBw1->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(41.3, 3.35)), false);

    auto gateBw2 = eDataMgr.CreateBondwire(baseLayout, "GateBw2", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw2->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(32.5, 1.8)), false);
    gateBw2->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(40.05, 1.0375)), false);

    auto gateBw3 = eDataMgr.CreateBondwire(baseLayout, "GateBw3", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw3->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(32.5, -1.8)), false);
    gateBw3->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(40.05, -0.3625)), false);

    auto gateBw4 = eDataMgr.CreateBondwire(baseLayout, "GateBw4", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw4->SetStartLayer(topCuLayer, coordUnits.toCoord(FPoint2D(32.5, -3)), false);
    gateBw4->SetEndLayer(topCuLayer, coordUnits.toCoord(FPoint2D(40.05, -2.7)), false);

    return baseLayout;
}

Ptr<ILayoutView> CreateDriverLayout(Ptr<IDatabase> database)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto driverCell = eDataMgr.CreateCircuitCell(database, "Driver");
    auto driverLayout = driverCell->GetLayoutView();

    //layer
    auto driverLayer1 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer1", ELayerType::DielectricLayer, -0.3, 0.38, MAT_ALN.data(), MAT_AIR.data()));
    auto driverLayer2 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer2", ELayerType::ConductingLayer, -0.68, 0.3, MAT_CU.data(), MAT_AIR.data()));
    auto driverLayer3 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer3", ELayerType::ConductingLayer, -0.98, 0.1, MAT_SAC305.data(), MAT_AIR.data()));
    auto driverLayer4 = driverLayout->AppendLayer(eDataMgr.CreateStackupLayer("DriverLayer4", ELayerType::ConductingLayer, -0.98, 0.1, MAT_SAC305.data(), MAT_AIR.data()));

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
    // eDataMgr.CreateGeometry2D(driverLayout, driverLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-4.7, -13.925}, {4.7, -13.925}, {4.7, 13.925}, {-4.7, 13.925}}, 0.25));
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-5, -14.225}, {5, -14.225}, {5, 14.225}, {-5, 14.225}}, 0.25));
    eDataMgr.CreateGeometry2D(driverLayout, driverLayer4, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-5.5, -14.725}, {5.5, -14.725}, {5.5, 14.725}, {-5.5, 14.725}}, 0.25));

    auto r1 = eDataMgr.FindComponentDefByName(database, "R1");
    eDataMgr.CreateComponent(driverLayout, "R1", r1, driverLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, math::Rad(90), {2.75, 9.125}), false);

    return driverLayout;
}

Ptr<ILayoutView> CreateBotBridgeLayout(Ptr<IDatabase> database)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto botBridgeCell = eDataMgr.CreateCircuitCell(database, "BotBridgeCell");
    auto botBridgeLayout = botBridgeCell->GetLayoutView();

    //layer
    auto botBridgeLayer1 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer1", ELayerType::DielectricLayer, -0.3, 0.38, MAT_ALN.data(), MAT_AIR.data()));
    auto botBridgeLayer2 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer2", ELayerType::ConductingLayer, -0.68, 0.3, MAT_CU.data(), MAT_AIR.data()));
    auto botBridgeLayer3 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer3", ELayerType::ConductingLayer, -0.98, 0.1, MAT_SAC305.data(), MAT_AIR.data()));
    auto botBridgeLayer4 = botBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer4", ELayerType::ConductingLayer, -0.98, 0.1, MAT_SAC305.data(), MAT_AIR.data()));

    //boundary   
    botBridgeLayout->SetBoundary(eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    
    //net
    auto ng = eDataMgr.CreateNet(botBridgeLayout, "Gate");
    auto nd = eDataMgr.CreateNet(botBridgeLayout, "Drain");
    auto ns = eDataMgr.CreateNet(botBridgeLayout, "Source");

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
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-13.65, -0.7}, {-12.9, -0.7}, {-12.9, 2.6}, {-13.65, 2.6}}, 0.25));
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-13.65, 3.725}, {-12.9, 3.725}, {-12.9, 7.025}, {-13.65, 7.025}}, 0.25));
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-13.65, 8.15}, {-12.9, 8.15}, {-12.9, 11.45}, {-13.65, 11.45}}, 0.25));

    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{9.5, -4.77}, {15.8, -4.77}, {15.8, 7.83}, {9.5, 7.83}}, 0.25));

    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer2, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}, 0.25));
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.25, -12}, {16.25, -12}, {16.25, 12}, {-16.25, 12}}, 0.25));
    eDataMgr.CreateGeometry2D(botBridgeLayout, botBridgeLayer4, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}, 0.25));

    std::array<Ptr<IComponent>, 3> dieComp;
    auto sicDie = eDataMgr.FindComponentDefByName(database, "SicDie");
    dieComp[0] = eDataMgr.CreateComponent(botBridgeLayout, "Die1", sicDie, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-5.23, 8.93}), false);
    dieComp[0]->SetLossPower(50);

    dieComp[1] = eDataMgr.CreateComponent(botBridgeLayout, "Die2", sicDie, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-5.23, 3.86}), false);
    dieComp[1]->SetLossPower(50);

    dieComp[2] = eDataMgr.CreateComponent(botBridgeLayout, "Die3", sicDie, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-5.23, -1.21}), false);
    dieComp[2]->SetLossPower(50);

    std::array<Ptr<IComponent>, 3> diodeComp;
    auto diode = eDataMgr.FindComponentDefByName(database, "Diode");
    diodeComp[0] = eDataMgr.CreateComponent(botBridgeLayout, "Diode1", diode, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {3.71, 8.08}), false);
    diodeComp[1] = eDataMgr.CreateComponent(botBridgeLayout, "Diode2", diode, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {3.71, 1.33}), false);
    diodeComp[2] = eDataMgr.CreateComponent(botBridgeLayout, "Diode3", diode, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {3.71, -5.42}), false);

    auto resGate = eDataMgr.FindComponentDefByName(database, RES_GATE.data());
    eDataMgr.CreateComponent(botBridgeLayout, "R1", resGate, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-14.17, 10.5}), false);
    eDataMgr.CreateComponent(botBridgeLayout, "R2", resGate, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-14.17, 6.075}), false);
    eDataMgr.CreateComponent(botBridgeLayout, "R3", resGate, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-14.17, 1.65}), false);

    auto gateBw1 = eDataMgr.CreateBondwire(botBridgeLayout, "GateBw1", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw1->SetStartComponent(dieComp[0], "G");
    gateBw1->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{-13.275, 8.6}), false);

    auto gateBw2 = eDataMgr.CreateBondwire(botBridgeLayout, "GateBw2", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw2->SetStartComponent(dieComp[1], "G");
    gateBw2->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{-13.275, 4.05}), false);

    auto gateBw3 = eDataMgr.CreateBondwire(botBridgeLayout, "GateBw1", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw3->SetStartComponent(dieComp[2], "G");
    gateBw3->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(FPoint2D{-13.275, -0.27}), false);

    std::vector<std::string> iPins{"A", "B", "C", "D", "E"};
    for (size_t i = 0; i < dieComp.size(); ++i) {
        for (size_t j = 0; j < iPins.size(); ++j) {
            auto bw = eDataMgr.CreateBondwire(botBridgeLayout, iPins.at(j), ns->GetNetId(), THICK_BONDWIRE_RADIUS);
            bw->SetStartComponent(dieComp[i], iPins.at(j));
            bw->SetEndComponent(diodeComp[i], iPins.at(j));
        }
    }

    std::vector<std::string> oPins{"F", "G", "H", "I", "J"};
    std::array<std::vector<FPoint2D>, 3> diodePLocs{
        std::vector<FPoint2D>{{12.65, 7.105}, {12.65, 6.38}, {11.075, 7.105}, {11.075, 6.38}, {11.075, 5.655}},
        std::vector<FPoint2D>{{14.225, 7.105}, {14.225, 6.38}, {12.65, -2.595}, {11.075, -2.595}, {11.075, -3.32}},
        std::vector<FPoint2D>{{12.65, -3.32}, {14.225, -3.32}, {11.075, -4.045}, {12.65, -4.045}, {14.225, -4.045}}
    };
    for (size_t i = 0; i < diodePLocs.size(); ++i) {
        for (size_t j = 0; j < oPins.size(); ++j) {
            auto bw = eDataMgr.CreateBondwire(botBridgeLayout, oPins.at(j), ns->GetNetId(), THICK_BONDWIRE_RADIUS);
            bw->SetStartComponent(diodeComp[i], oPins.at(j));
            bw->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(diodePLocs.at(i).at(j)), false);
        }
    }

    std::vector<FPoint2D> kelvinBwPLocs{{-11.475, 8.15}, {-11.475, 3.6}, {-11.475, -0.72}};
    for (size_t i = 0; i < kelvinBwPLocs.size(); ++i) {
        auto bw = eDataMgr.CreateBondwire(botBridgeLayout, "KelvinBw" + std::to_string(i + 1), ns->GetNetId(), THIN_BONDWIRE_RADIUS);
        bw->SetStartComponent(dieComp[i], "K");
        bw->SetEndLayer(botBridgeLayer1, coordUnits.toCoord(kelvinBwPLocs.at(i)), false);
    }
    return botBridgeLayout;
}

Ptr<ILayoutView> CreateTopBridgeLayout(Ptr<IDatabase> database)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto topBridgeCell = eDataMgr.CreateCircuitCell(database, "TopBridgeCell");
    auto topBridgeLayout = topBridgeCell->GetLayoutView();

    //layer
    auto topBridgeLayer1 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer1", ELayerType::DielectricLayer, -0.3, 0.38,MAT_ALN.data(), MAT_AIR.data()));
    auto topBridgeLayer2 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer2", ELayerType::ConductingLayer, -0.68, 0.3, MAT_CU.data(), MAT_AIR.data()));
    auto topBridgeLayer3 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer3", ELayerType::ConductingLayer, -0.98, 0.1, MAT_SAC305.data(), MAT_AIR.data()));
    auto topBridgeLayer4 = topBridgeLayout->AppendLayer(eDataMgr.CreateStackupLayer("Layer4", ELayerType::ConductingLayer, -0.98, 0.1, MAT_SAC305.data(), MAT_AIR.data()));

    //boundary   
    topBridgeLayout->SetBoundary(eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    
    //net
    auto ns = eDataMgr.CreateNet(topBridgeLayout, "Gate");
    auto nd = eDataMgr.CreateNet(topBridgeLayout, "Drain");
    auto ng = eDataMgr.CreateNet(topBridgeLayout, "Source");

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
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{11.1, -8.5}, {12.9, -8.5}, {12.9, -6.55}, {11.85, -6.55},
        {11.85, 11.45}, {11.1, 11.45}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, -5.5}, {13.65, -5.5}, {13.65, -2.2}, {12.9, -2.2}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9, 0.95}, {13.65, 0.95}, {13.65, 4.25}, {12.9, 4.25}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{12.9,  7.4}, {13.65,  7.4}, {13.65, 10.7}, {12.9, 10.7}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer1, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{13.65, -8.5}, {15.45, -8.5}, {15.45, 11.45}, {14.7, 11.45},
       {14.7, -0.125}, {13.65, -0.125}, {13.65, -1.125}, {14.7, -1.125}, {14.7, -6.55}, {13.65, -6.55}}, 0.25)
    );
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer2, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer3, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.25, -12}, {16.25, -12}, {16.25, 12}, {-16.25, 12}}, 0.25));
    eDataMgr.CreateGeometry2D(topBridgeLayout, topBridgeLayer4, ENetId::noNet, eDataMgr.CreateShapePolygon(coordUnits, {{-16.75, -12.5}, {16.75, -12.5}, {16.75, 12.5}, {-16.75, 12.5}}));

    std::array<Ptr<IComponent>, 3> dieComp; 
    auto sicDie = eDataMgr.FindComponentDefByName(database, "SicDie");
    dieComp[0] = eDataMgr.CreateComponent(topBridgeLayout, "Die1", sicDie, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {5.23, 8.08}, EMirror2D::Y), false);
    dieComp[0]->SetLossPower(50);

    dieComp[1] = eDataMgr.CreateComponent(topBridgeLayout, "Die2", sicDie, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {5.23, 1.33}, EMirror2D::Y), false);
    dieComp[1]->SetLossPower(50);

    dieComp[2] = eDataMgr.CreateComponent(topBridgeLayout, "Die3", sicDie, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {5.23, -5.42}, EMirror2D::Y), false);
    dieComp[2]->SetLossPower(50);

    std::array<Ptr<IComponent>, 3> diodeComp;
    auto diode = eDataMgr.FindComponentDefByName(database, "Diode");
    diodeComp[0] = eDataMgr.CreateComponent(topBridgeLayout, "Diode1", diode, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-3.7, 8.08}, EMirror2D::Y), false);
    diodeComp[1] = eDataMgr.CreateComponent(topBridgeLayout, "Diode2", diode, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-3.7, 1.33}, EMirror2D::Y), false);
    diodeComp[2] = eDataMgr.CreateComponent(topBridgeLayout, "Diode3", diode, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-3.7, -5.42}, EMirror2D::Y), false);

    auto resGate = eDataMgr.FindComponentDefByName(database, RES_GATE.data());
    eDataMgr.CreateComponent(topBridgeLayout, "R1", resGate, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {14.17, 8.35}), false);
    eDataMgr.CreateComponent(topBridgeLayout, "R2", resGate, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {14.17, 1.9}), false);
    eDataMgr.CreateComponent(topBridgeLayout, "R3", resGate, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {14.17, -4.55}), false);

    auto gateBw1 = eDataMgr.CreateBondwire(topBridgeLayout, "GateBw1", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw1->SetStartComponent(dieComp[0], "G");
    gateBw1->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{13.275, 10.25}), false);

    auto gateBw2 = eDataMgr.CreateBondwire(topBridgeLayout, "GateBw2", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw2->SetStartComponent(dieComp[1], "G");
    gateBw2->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{13.275, 3.8}), false);

    auto gateBw3 = eDataMgr.CreateBondwire(topBridgeLayout, "GateBw1", ng->GetNetId(), THIN_BONDWIRE_RADIUS);
    gateBw3->SetStartComponent(dieComp[2], "G");
    gateBw3->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(FPoint2D{13.275, -2.65}), false);

    std::vector<std::string> pins{"A", "B", "C", "D", "E"};
    for (size_t i = 0; i < dieComp.size(); ++i) {
        for (size_t j = 0; j < pins.size(); ++j) {
            auto bw = eDataMgr.CreateBondwire(topBridgeLayout, pins.at(j), ns->GetNetId(), THICK_BONDWIRE_RADIUS);
            bw->SetStartComponent(dieComp[i], pins.at(j));
            bw->SetEndComponent(diodeComp[i], pins.at(j));
        }
    }

    std::vector<std::string> oPins{"F", "G", "H", "I", "J"};
    std::array<std::vector<FPoint2D>, 3> diodePLocs{
        std::vector<FPoint2D>{{-10.15, 10.725}, {-10.15, 10}, {-10.15, 9.27}, {-10.15, 8.55}, {-10.15, 7.825}},
        std::vector<FPoint2D>{{-10.15, 7.1}, {-10.15, 6.375}, {-11.15, 6.375}, {-10.15, -3.8125}, {-10.15, -4.525}},
        std::vector<FPoint2D>{{-10.15, -5.2375}, {-10.15, -5.95}, {-10.15, -6.6625}, {-10.15, -7.375}, {-10.15, -8.0875}}
    };
    for (size_t i = 0; i < diodePLocs.size(); ++i) {
        for (size_t j = 0; j < oPins.size(); ++j) {
            auto bw = eDataMgr.CreateBondwire(topBridgeLayout, oPins.at(j), ns->GetNetId(), THICK_BONDWIRE_RADIUS);
            bw->SetStartComponent(diodeComp[i], oPins.at(j));
            bw->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(diodePLocs.at(i).at(j)), false);
        }
    }
    std::vector<FPoint2D> kelvinBwPLocs{{11.475, 8.08}, {11.475, 1.33}, {11.475, -5.42}};
    for (size_t i = 0; i < kelvinBwPLocs.size(); ++i) {
        auto bw = eDataMgr.CreateBondwire(topBridgeLayout, "KelvinBw" + std::to_string(i + 1), ns->GetNetId(), THIN_BONDWIRE_RADIUS);
        bw->SetStartComponent(dieComp[i], "K");
        bw->SetEndLayer(topBridgeLayer1, coordUnits.toCoord(kelvinBwPLocs.at(i)), false);
    }
    return topBridgeLayout;
}

Ptr<ILayoutView> SetupDesign(const std::string & name)
{
    //database
    auto database = eDataMgr.CreateDatabase(name);

    //material
    SetupMaterials(database);
    
    //coord units
    ECoordUnits coordUnits(ECoordUnits::Unit::Millimeter);
    database->SetCoordUnits(coordUnits);

    auto thinBwSolderDef = CreateBondwireSolderJoints(database, "Thin Solder Joints", THIN_BONDWIRE_RADIUS);
    auto thickBwSolderDef = CreateBondwireSolderJoints(database, "Thick Solder Joins", THICK_BONDWIRE_RADIUS);

    //component
    CreateSicDieComponentDef(database);
    CreateDiodeComponentDef(database);
    CreateR1ComponentDef(database);
    CreateGateResistanceComponentDef(database);

    auto baseLayout = CreateBaseLayout(database);
    auto driverLayout = CreateDriverLayout(database);
    auto driverLayerMap = CreateDefaultLayerMap(database, driverLayout, baseLayout, "DriverLayerMap");
    
    //instance
    auto driverL = eDataMgr.CreateCellInst(baseLayout, "DriverL", driverLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-44, 0}));
    driverL->SetLayerMap(driverLayerMap);
    auto driverR = eDataMgr.CreateCellInst(baseLayout, "DriverR", driverLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {44, 0}, EMirror2D::XY));
    driverR->SetLayerMap( driverLayerMap);

    auto botBridgeLayout = CreateBotBridgeLayout(database);
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
    topBridge1->SetLayerMap(topBridgeLayerMap);

    auto topBridge2 = eDataMgr.CreateCellInst(baseLayout, "topBridge2", topBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {17.75, -13}, EMirror2D::X));
    topBridge2->SetLayerMap(topBridgeLayerMap);

    //flatten
    baseLayout->Flatten(EFlattenOption{});
    auto primIter = baseLayout->GetPrimitiveIter();
    while (auto * prim = primIter->Next()) {
        if (auto * bw = prim->GetBondwireFromPrimitive(); bw) {
            bw->SetBondwireType(EBondwireType::Simple);
            if (bw->GetRadius() > THIN_BONDWIRE_RADIUS)
                bw->SetSolderJoints(thickBwSolderDef);
            else bw->SetSolderJoints(thinBwSolderDef);
            bw->SetMaterial(MAT_AL.data());
            bw->SetHeight(0.5);
        }
    }

    return baseLayout;
}

EPrismaThermalModelExtractionSettings ExtractionSettings(const std::string & workDir)
{
    EPrismaThermalModelExtractionSettings prismaSettings;
    prismaSettings.workDir = workDir;
    prismaSettings.botUniformBC.type = EThermalBondaryCondition::BCType::HTC;
    prismaSettings.botUniformBC.value = 2750;
    prismaSettings.meshSettings.iteration = 1e5;
    prismaSettings.meshSettings.minAlpha = 20;
    prismaSettings.meshSettings.minLen = 1e-4;
    prismaSettings.meshSettings.maxLen = 2;
    prismaSettings.meshSettings.tolerance = 1e-6;
    prismaSettings.layerCutSettings.layerTransitionRatio = 3;

    EFloat topHTC = 1e3;
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, 4.7}, {-20.35, 8.7}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, -8.7}, {-20.35, -4.7}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, 11.5}, {9.75, 17}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, -17}, {9.75, -11.5}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, 11.5}, {-2.55, 17}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismaSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, -17}, {-2.55, -11.5}), EThermalBondaryCondition::BCType::HTC, topHTC);
    return prismaSettings;
}

void StaticThermalFlow(Ptr<ILayoutView> layout, const std::string & workDir)
{
    EThermalStaticSimulationSetup setup;
    setup.settings.iteration = 10;
    setup.settings.dumpHotmaps = true;
    setup.settings.envTemperature = {25, ETemperatureUnit::Celsius};
    setup.workDir = workDir;
    auto [minT, maxT] = layout->RunThermalSimulation(ExtractionSettings(workDir), setup);    
    ECAD_TRACE("minT: %1%, maxT: %2%", minT, maxT)
}

void TransientThermalFlow(Ptr<ILayoutView> layout, const std::string & workDir)
{
    EThermalTransientSimulationSetup setup;
    setup.settings.envTemperature = {25, ETemperatureUnit::Celsius};
    setup.workDir = workDir;
    setup.settings.mor = false;
    setup.settings.verbose = true;
    setup.settings.adaptive = true;
    setup.settings.dumpRawData = true;
    setup.settings.duration = 1;
    setup.settings.step = 0.01;
    setup.settings.samplingWindow = 0.1;
    setup.settings.minSamplingInterval = 0.0001;
    setup.settings.absoluteError = 1e-1;
    setup.settings.relativeError = 1e-1;
    EThermalTransientExcitation excitation = [](EFloat t){ return std::abs(std::sin(generic::math::pi * t / 0.05)); };
    setup.settings.excitation = &excitation;
    setup.settings.verbose = true;
    auto [minT, maxT] = layout->RunThermalSimulation(ExtractionSettings(workDir), setup);    
    ECAD_TRACE("minT: %1%, maxT: %2%", minT, maxT)
}

int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &SignalHandler);
    ::signal(SIGABRT, &SignalHandler);

    ecad::EDataMgr::Instance().Init(ecad::ELogLevel::Trace);

    std::string workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    auto layout = SetupDesign("CREE62mm");
    StaticThermalFlow(layout, workDir);
    // TransientThermalFlow(layout, workDir);
    ecad::EDataMgr::Instance().ShutDown();
    return EXIT_SUCCESS;
}