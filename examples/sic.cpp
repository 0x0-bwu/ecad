#include <boost/stacktrace.hpp>
#include <string_view>
#include <filesystem>
#include <cassert>
#include <csignal>

#include "../test/TestData.hpp"
#include "generic/tools/StringHelper.hpp"
#include "utility/ELayoutRetriever.h"
#include "EDataMgr.h"

void SignalHandler(int signum)
{
    ::signal(signum, SIG_DFL);
    std::cout << boost::stacktrace::stacktrace();
    ::raise(SIGABRT);
}

using namespace ecad;
using namespace generic;
auto & eDataMgr = EDataMgr::Instance();
[[maybe_unused]] inline static constexpr EFloat AVERAGE_LOSS_POWER = 123.3333;
inline static constexpr std::string_view RES_GATE = "Rg";

inline static constexpr std::string_view MAT_AL = "Al";
inline static constexpr std::string_view MAT_CU = "Cu";
inline static constexpr std::string_view MAT_AIR = "Air";
inline static constexpr std::string_view MAT_ALN = "AlN";
inline static constexpr std::string_view MAT_SIC = "SiC";
inline static constexpr std::string_view MAT_SAC305 = "SAC305";

std::vector<std::pair<EFloat, EFloat>> GetSicDieTemperatureAndPowerConfig()
{
    return {
        {ETemperature::Celsius2Kelvins(25), 108.0},
        {ETemperature::Celsius2Kelvins(125), 124.0},
        {ETemperature::Celsius2Kelvins(150), 126.5}
    };
}

std::vector<std::pair<EFloat, EFloat>> GetDiodeTemperatureAndPowerConfig()
{
    return {
        {ETemperature::Celsius2Kelvins(25), 20.4},
        {ETemperature::Celsius2Kelvins(125), 21.7},
        {ETemperature::Celsius2Kelvins(150), 21.8}
    };
}

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

Ptr<IComponentDef> CreateSicDieComponentDef(Ptr<IDatabase> database)
{
    auto sicDie = eDataMgr.CreateComponentDef(database, "SicDie");
    sicDie->SetComponentType(EComponentType::IC);
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
    diode->SetComponentType(EComponentType::IC);
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

    [[maybe_unused]] auto ng = eDataMgr.CreateNet(baseLayout, "Gate");
    [[maybe_unused]] auto nd = eDataMgr.CreateNet(baseLayout, "Drain");
    [[maybe_unused]] auto ns = eDataMgr.CreateNet(baseLayout, "Source");
    [[maybe_unused]] auto nk = eDataMgr.CreateNet(baseLayout, "Kelvin");

    //base
    [[maybe_unused]] auto topCuLayer = baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCuLayer", ELayerType::ConductingLayer, 0, 0.3, MAT_CU.data(), MAT_AIR.data()));
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
    
    return baseLayout;
}

Ptr<ILayoutView> CreateBotBridgeLayout(Ptr<IDatabase> database, const std::vector<FPoint2D> & locations)
{
    //placement area: {-9.65, -3.65}, {-2.5, -3.65}, {-2.5, -8.7}, {7.85, -8.7}, {7.85, 11.35}, {-9.65, 11.35}
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
    [[maybe_unused]] auto ng = eDataMgr.CreateNet(botBridgeLayout, "Gate");
    [[maybe_unused]] auto nd = eDataMgr.CreateNet(botBridgeLayout, "Drain");
    [[maybe_unused]] auto ns = eDataMgr.CreateNet(botBridgeLayout, "Source");

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
    dieComp[0] = eDataMgr.CreateComponent(botBridgeLayout, "Die1", sicDie, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(0)), false);
    dieComp[1] = eDataMgr.CreateComponent(botBridgeLayout, "Die2", sicDie, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(1)), false);
    dieComp[2] = eDataMgr.CreateComponent(botBridgeLayout, "Die3", sicDie, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(2)), false);
    for (auto [t, p] :  GetSicDieTemperatureAndPowerConfig()) {
        for (auto die : dieComp) {
            die->SetLossPower(t, p);
            die->SetDynamicPowerScenario(2);
        }
    }

    std::array<Ptr<IComponent>, 3> diodeComp;
    auto diode = eDataMgr.FindComponentDefByName(database, "Diode");
    diodeComp[0] = eDataMgr.CreateComponent(botBridgeLayout, "Diode1", diode, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(3)), false);
    diodeComp[1] = eDataMgr.CreateComponent(botBridgeLayout, "Diode2", diode, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(4)), false);
    diodeComp[2] = eDataMgr.CreateComponent(botBridgeLayout, "Diode3", diode, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(5)), false);

    for (auto [t, p] :  GetDiodeTemperatureAndPowerConfig()) {
        for (auto diode : diodeComp) {
            diode->SetLossPower(t, p);
            diode->SetDynamicPowerScenario(0);
        }
    }

    auto resGate = eDataMgr.FindComponentDefByName(database, RES_GATE.data());
    eDataMgr.CreateComponent(botBridgeLayout, "R1", resGate, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-14.17, 10.5}), false);
    eDataMgr.CreateComponent(botBridgeLayout, "R2", resGate, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-14.17, 6.075}), false);
    eDataMgr.CreateComponent(botBridgeLayout, "R3", resGate, botBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-14.17, 1.65}), false);

    return botBridgeLayout;
}

Ptr<ILayoutView> CreateTopBridgeLayout(Ptr<IDatabase> database, const std::vector<FPoint2D> & locations)
{
    //placement area: {-7.85, -8.7}, {9.65, -8.7}, {9.65, 11.35}, {-7.85, 11.35}
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
    [[maybe_unused]] auto ns = eDataMgr.CreateNet(topBridgeLayout, "Gate");
    [[maybe_unused]] auto nd = eDataMgr.CreateNet(topBridgeLayout, "Drain");
    [[maybe_unused]] auto ng = eDataMgr.CreateNet(topBridgeLayout, "Source");

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
    dieComp[0] = eDataMgr.CreateComponent(topBridgeLayout, "Die1", sicDie, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(0), EMirror2D::Y), false);
    dieComp[1] = eDataMgr.CreateComponent(topBridgeLayout, "Die2", sicDie, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(1), EMirror2D::Y), false);
    dieComp[2] = eDataMgr.CreateComponent(topBridgeLayout, "Die3", sicDie, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(2), EMirror2D::Y), false);
    for (auto [t, p] :  GetSicDieTemperatureAndPowerConfig()) {
        for (auto die : dieComp) {
            die->SetLossPower(t, p);
            die->SetDynamicPowerScenario(1);
        }
    }

    std::array<Ptr<IComponent>, 3> diodeComp;
    auto diode = eDataMgr.FindComponentDefByName(database, "Diode");
    diodeComp[0] = eDataMgr.CreateComponent(topBridgeLayout, "Diode1", diode, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(3), EMirror2D::Y), false);
    diodeComp[1] = eDataMgr.CreateComponent(topBridgeLayout, "Diode2", diode, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(4), EMirror2D::Y), false);
    diodeComp[2] = eDataMgr.CreateComponent(topBridgeLayout, "Diode3", diode, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, locations.at(5), EMirror2D::Y), false);

    for (auto [t, p] :  GetDiodeTemperatureAndPowerConfig()) {
        for (auto diode : diodeComp) {
            diode->SetLossPower(t, p);
            diode->SetDynamicPowerScenario(0);
        }
    }

    auto resGate = eDataMgr.FindComponentDefByName(database, RES_GATE.data());
    eDataMgr.CreateComponent(topBridgeLayout, "R1", resGate, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {14.17, 8.35}), false);
    eDataMgr.CreateComponent(topBridgeLayout, "R2", resGate, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {14.17, 1.9}), false);
    eDataMgr.CreateComponent(topBridgeLayout, "R3", resGate, topBridgeLayer1, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {14.17, -4.55}), false);

    return topBridgeLayout;
}

Ptr<ILayoutView> SetupDesign(const std::string & name, const std::vector<EFloat> & parameters)
{
    //database
    eDataMgr.RemoveDatabase(name);
    auto database = eDataMgr.CreateDatabase(name);

    //material
    SetupMaterials(database);
    
    //coord units
    ECoordUnits coordUnits(ECoordUnits::Unit::Millimeter);
    database->SetCoordUnits(coordUnits);

    //component
    CreateSicDieComponentDef(database);
    CreateDiodeComponentDef(database);
    CreateGateResistanceComponentDef(database);

    auto baseLayout = CreateBaseLayout(database);    
    std::vector<FPoint2D> botCompLoc;
    for (size_t i = 0; i < 6; ++i)
        botCompLoc.emplace_back(FPoint2D(parameters.at(i * 2), parameters.at(i * 2 + 1)));
    auto botBridgeLayout = CreateBotBridgeLayout(database, botCompLoc);
    auto botBridgeLayerMap = CreateDefaultLayerMap(database, botBridgeLayout, baseLayout, "BotBridgeLayerMap");

    //instance
    auto botBridge1 = eDataMgr.CreateCellInst(baseLayout, "BotBridge1", botBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-17.75, 13}));
    botBridge1->SetLayerMap(botBridgeLayerMap);

    auto botBridge2 = eDataMgr.CreateCellInst(baseLayout, "BotBridge2", botBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {-17.75, -13}, EMirror2D::X));
    botBridge2->SetLayerMap(botBridgeLayerMap);

    std::vector<FPoint2D> topCompLoc;
    for (size_t i = 0; i < 6; ++i)
        topCompLoc.emplace_back(FPoint2D(parameters.at(i * 2 + 12), parameters.at(i * 2 + 13)));
    auto topBridgeLayout = CreateTopBridgeLayout(database, topCompLoc);
    auto topBridgeLayerMap = CreateDefaultLayerMap(database, topBridgeLayout, baseLayout, "TopBridgeLayerMap");

    //instance
    auto topBridge1 = eDataMgr.CreateCellInst(baseLayout, "TopBridge1", topBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {17.75, 13}));
    topBridge1->SetLayerMap(topBridgeLayerMap);

    auto topBridge2 = eDataMgr.CreateCellInst(baseLayout, "TopBridge2", topBridgeLayout, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {17.75, -13}, EMirror2D::X));
    topBridge2->SetLayerMap(topBridgeLayerMap);

    //flatten
    baseLayout->Flatten(EFlattenOption{});
    return baseLayout;
}

std::vector<FPoint3D> GetDieMonitors(CPtr<ILayoutView> layout)
{
    std::vector<FPoint3D> monitors;
    utils::ELayoutRetriever retriever(layout);
    std::vector<std::string> cellInsts{"TopBridge1", "TopBridge2", "BotBridge1", "BotBridge2"};
    std::vector<std::string> components{"Die1", "Die2", "Die3"};
    for(const auto & cellInst : cellInsts) {
        for (const auto & component : components) {
            std::string name = cellInst + eDataMgr.HierSep() + component;
            auto comp = layout->FindComponentByName(name);
            EFloat elevation, thickness;
            retriever.GetComponentHeightThickness(comp, elevation, thickness);
            auto loc = layout->GetCoordUnits().toUnit(comp->GetBoundingBox().Center());
            monitors.emplace_back(loc[0], loc[1], elevation - 0.1 * thickness);
        }
    }
    return monitors;
}

UPtr<EPrismThermalModelExtractionSettings> ExtractionSettings(const std::string & workDir)
{
    EFloat htc = 5000;
    EPrismThermalModelExtractionSettings prismSettings;
    prismSettings.threads = EDataMgr::Instance().Threads();
    prismSettings.workDir = workDir;
    prismSettings.botUniformBC.type = EThermalBondaryCondition::BCType::HTC;
    prismSettings.botUniformBC.value = htc;
    prismSettings.meshSettings.genMeshByLayer = true;
    if (prismSettings.meshSettings.genMeshByLayer)
        prismSettings.meshSettings.imprintUpperLayer = false;
    prismSettings.meshSettings.iteration = 3e3;
    prismSettings.meshSettings.minAlpha = 15;
    prismSettings.meshSettings.minLen = 1e-3;
    prismSettings.meshSettings.maxLen = 100;
    prismSettings.meshSettings.tolerance = 0;
    prismSettings.layerCutSettings.layerTransitionRatio = 3;
    prismSettings.meshSettings.dumpMeshFile = true;
    prismSettings.layerCutSettings.dumpSketchImg = true;

    EFloat topHTC = htc;
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, 4.7}, {-20.35, 8.7}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-29.35, -8.7}, {-20.35, -4.7}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, 11.5}, {9.75, 17}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({2.75, -17}, {9.75, -11.5}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, 11.5}, {-2.55, 17}), EThermalBondaryCondition::BCType::HTC, topHTC);
    prismSettings.AddBlockBC(EOrientation::Top, FBox2D({-7.75, -17}, {-2.55, -11.5}), EThermalBondaryCondition::BCType::HTC, topHTC);
    return std::make_unique<EPrismThermalModelExtractionSettings>(prismSettings);
}

void TransientThermalFlow(Ptr<ILayoutView> layout, const std::string & workDir, EFloat period, EFloat duty)
{
    EThermalTransientSimulationSetup setup;
    setup.workDir = workDir;
    setup.monitors = GetDieMonitors(layout);
    setup.settings.envTemperature = {25, ETemperatureUnit::Celsius};
    setup.settings.mor.order = 10;
    setup.settings.verbose = true;
    setup.settings.dumpResults = true;
    setup.settings.duration = 10;
    setup.settings.step = period * duty / 10;
    setup.settings.temperatureDepend = false;
    setup.settings.samplingWindow = setup.settings.duration;
    setup.settings.minSamplingInterval = period * duty / 10;
    setup.settings.absoluteError = 1e-5;
    setup.settings.relativeError = 1e-5;
    setup.settings.threads = eDataMgr.Threads();
    setup.extractionSettings = ExtractionSettings(workDir);
    EThermalTransientExcitation excitation = [period, duty](EFloat t, size_t scen) -> EFloat { 
        EFloat tm = std::fmod(t, period); 
        switch (scen) {
            case 0 : return 1;
            case 1 : return tm < duty * period ? 1 : 0;
            case 2 : return 0.5 * period < tm && tm < (duty + 0.5) * period ? 1 : 0; 
            default : { ECAD_ASSERT(false) return 0; }
        }
    };
    auto [minT, maxT] = layout->RunThermalSimulation(setup, excitation);    
    ECAD_TRACE("minT: %1%, maxT: %2%", minT, maxT)
}

int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &SignalHandler);
    ::signal(SIGABRT, &SignalHandler);

    ecad::EDataMgr::Instance().Init(ecad::ELogLevel::Trace);

    std::string workDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
    std::vector<EFloat> parameters = {
        -5.23, 8.93, -5.23, 3.86, -5.23, -1.21, 3.71, 8.08, 3.71, 1.33, 3.71, -5.42,
        5.23, 8.08, 5.23, 1.33, 5.23, -5.42, -3.7, 8.08, -3.7, 1.33, -3.7, -5.42,
    };
    // std::vector<EFloat> parameters{-6.28323,9.1159,-6.13371,3.41274,-4.54684,-1.35806,4.60621,8.17798,3.8993,1.42668,2.64954,-4.95307,4.75943,8.76399,4.25377,0.922324,5.61007,-5.06671,-4.21649,7.63576,-4.1069,2.27594,-3.74797,-5.25336,};
    auto layout = SetupDesign("CREE62mm", parameters);
    // StaticThermalFlow(layout, workDir);
    TransientThermalFlow(layout, workDir, 1, 0.5);
    ecad::EDataMgr::Instance().ShutDown();
    return EXIT_SUCCESS;
}