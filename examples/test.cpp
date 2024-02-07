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
    auto matCu = eDataMgr.CreateMaterialDef(database, MAT_CU.data());
    matCu->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreatePolynomialMaterialProp({{437.6, -0.165, 1.825e-4, -1.427e-7, 3.979e-11}}));
    matCu->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreatePolynomialMaterialProp({{342.8, 0.134, 5.535e-5, -1.971e-7, 1.141e-10}}));
    matCu->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(8850));

    auto matSiC = eDataMgr.CreateMaterialDef(database, MAT_SIC.data());  
    matSiC->SetProperty(EMaterialPropId::ThermalConductivity, eDataMgr.CreatePolynomialMaterialProp({{1860, -11.7, 0.03442, -4.869e-5, 2.675e-8}}));
    matSiC->SetProperty(EMaterialPropId::SpecificHeat, eDataMgr.CreatePolynomialMaterialProp({{-3338, 33.12, -0.1037, 0.0001522, -8.553e-8}}));
    matSiC->SetProperty(EMaterialPropId::MassDensity, eDataMgr.CreateSimpleMaterialProp(3210));
}

Ptr<IComponentDef> CreateSicDieComponentDef(Ptr<IDatabase> database)
{
    auto sicDie = eDataMgr.CreateComponentDef(database, "SicDie");
    sicDie->SetSolderFillingMaterial(MAT_SAC305.data());
    sicDie->SetBondingBox(eDataMgr.CreateBox(database->GetCoordUnits(), FPoint2D(-2.545, -2.02), FPoint2D(2.545, 2.02)));
    sicDie->SetMaterial(MAT_SIC.data());
    sicDie->SetHeight(0.18);
    return sicDie;
}

Ptr<ILayoutView> CreateBaseLayout(Ptr<IDatabase> database)
{
    const auto & coordUnits = database->GetCoordUnits();
    auto baseCell = eDataMgr.CreateCircuitCell(database, "Base");
    auto baseLayout = baseCell->GetLayoutView();

    auto polygon = eDataMgr.CreateShapePolygon(database->GetCoordUnits(), {{-52.2, -29.7}, {52.2, -29.7}, {52.5, 29.7}, {-52.2, 29.7}})->GetContour();
    baseLayout->SetBoundary(eDataMgr.CreateShapePolygon(std::move(polygon)));

    auto topCuLayer = baseLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCuLayer", ELayerType::ConductingLayer, 0, 0.3, MAT_CU.data(), MAT_CU.data()));

    auto sicDie = eDataMgr.FindComponentDefByName(database, "SicDie");
    auto dieComp = eDataMgr.CreateComponent(baseLayout, "Die1", sicDie, topCuLayer, eDataMgr.CreateTransform2D(coordUnits, 1, 0, {0, 0}), false);
    dieComp->SetLossPower(50);

    return baseLayout;
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

    //component def
    CreateSicDieComponentDef(database);

    return CreateBaseLayout(database);
}

EPrismaThermalModelExtractionSettings ExtractionSettings(const std::string & workDir)
{
    EPrismaThermalModelExtractionSettings prismaSettings;
    prismaSettings.workDir = workDir;
    prismaSettings.botUniformBC.type = EThermalBondaryCondition::BCType::HTC;
    prismaSettings.botUniformBC.value = 2750;
    prismaSettings.meshSettings.iteration = 1e4;
    prismaSettings.meshSettings.minAlpha = 20;
    prismaSettings.meshSettings.minLen = 1e-2;
    prismaSettings.meshSettings.maxLen = 2;
    prismaSettings.meshSettings.tolerance = 1e-6;
    prismaSettings.layerCutSettings.layerTransitionRatio = 0;
    return prismaSettings;
}

void StaticThermalFlow(Ptr<ILayoutView> layout, const std::string & workDir)
{
    EThermalStaticSimulationSetup setup;
    setup.settings.iteration = 10;
    setup.settings.dumpHotmaps = true;
    setup.settings.envTemperature = {25, ETemperatureUnit::Celsius};
    setup.settings.solverType = EThermalNetworkStaticSolverType::SparseLU;
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
    setup.settings.adaptive = false;
    setup.settings.dumpResults = true;
    setup.settings.duration = 10;
    setup.settings.step = 1e-1;
    setup.settings.samplingWindow = 0.1;
    setup.settings.minSamplingInterval = 0.0001;
    setup.settings.absoluteError = 1e-1;
    setup.settings.relativeError = 1e-1;
    setup.settings.threads = eDataMgr.Threads();
    EThermalTransientExcitation excitation = [](EFloat t){ return std::abs(std::cos(generic::math::pi * t / 0.05)); };
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
    auto layout = SetupDesign("test");
    StaticThermalFlow(layout, workDir);
    // TransientThermalFlow(layout, workDir);
    ecad::EDataMgr::Instance().ShutDown();
    return EXIT_SUCCESS;
}