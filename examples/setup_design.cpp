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

int main(int argc, char * argv[])
{
    ::signal(SIGSEGV, &SignalHandler);
    ::signal(SIGABRT, &SignalHandler);

    using namespace ecad;
    auto & eDataMgr = EDataMgr::Instance();
    eDataMgr.SetDefaultThreads(1);

    //database
    auto database = eDataMgr.CreateDatabase("RobGrant");

    //coord units
    ECoordUnits coordUnits(ECoordUnits::Unit::Micrometer);
    database->SetCoordUnits(coordUnits);
    
    //top cell
    auto topCell = eDataMgr.CreateCircuitCell(database, "TopCell");
    auto topLayout = topCell->GetLayoutView();
    auto topBouds = std::make_unique<EPolygon>(std::vector<EPoint2D>{{-5000000, -5000000}, {86000000, -5000000}, {86000000, 31000000}, {0, 31000000}});
    topLayout->SetBoundary(std::move(topBouds));

    eDataMgr.CreateNet(topLayout, "Gate");
    eDataMgr.CreateNet(topLayout, "Drain");
    eDataMgr.CreateNet(topLayout, "Source");

    //substrate
    [[maybe_unused]] auto iLyrTopCu = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("TopCu", ELayerType::ConductingLayer, 0, 0.4, "Cu", "Air"));
    [[maybe_unused]] auto iLyrSubstrate = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("Substrate", ELayerType::DielectricLayer, -0.4, 0.635, "", "Si3N4"));
    [[maybe_unused]] auto iLyrCuPlate = topLayout->AppendLayer(eDataMgr.CreateStackupLayer("CuPlate", ELayerType::ConductingLayer, -1.035, 0.3, "Cu", ""));

    //sic die
    auto sicCell = eDataMgr.CreateCircuitCell(database, "SicDie");
    auto sicLayout = sicCell->GetLayoutView();

    //boundary
    auto sicBonds = std::make_unique<EPolygon>(std::vector<EPoint2D>{{0, 0}, {23000000, 0}, {23000000, 26000000}, {0, 26000000}});
    sicLayout->SetBoundary(std::move(sicBonds));

    auto iLyrWire = sicLayout->AppendLayer(eDataMgr.CreateStackupLayer("Wire", ELayerType::ConductingLayer, 0, 0.4, "Cu", ""));

    //net
    auto gateNet = eDataMgr.CreateNet(sicLayout, "Gate");
    auto drainNet = eDataMgr.CreateNet(sicLayout, "Drain");
    auto sourceNet = eDataMgr.CreateNet(sicLayout, "Source");

    //wire
    //todo, bondwire (3, 2.5) (3, 7.5)
    std::vector<EPoint2D> ps1 {{0, 0}, {14200000, 0}, {14200000, 3500000}, {5750000, 3500000}, {5750000, 9150000}, {0, 9150000}};
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, sourceNet->GetNetId(), eDataMgr.CreateShapePolygon(std::move(ps1)));

    //todo, bondwire (10.8, 7.5) (10.8, 12.2), die (1.45, 13) (5.45, 17)
    std::vector<EPoint2D> ps2 {{0, 10650000}, {7300000, 10650000}, {7300000, 5000000}, {14300000, 5000000}, {14300000, 19000000}, {1450000, 19000000}, {1450000, 26000000}, {0, 26000000}};
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, drainNet->GetNetId(), eDataMgr.CreateShapePolygon(std::move(ps2)));

    //todo, bondwire (19.35, 2.5) (19.35, 7.5), die (17, 13) (21, 17)
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

    //todo, bondwire (19.75, 24)
    auto rec6 = eDataMgr.CreateShapeRectangle(EPoint2D(19000000, 20500000), EPoint2D(20500000, 26000000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec6));

    //todo, bondwire (22.25, 24)
    auto rec7 = eDataMgr.CreateShapeRectangle(EPoint2D(21500000, 20500000), EPoint2D(23000000, 26000000));
    eDataMgr.CreateGeometry2D(sicLayout, iLyrWire, ENetId::noNet, std::move(rec7));

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
    
    auto netIter = layout->GetNetIter();
    while (auto net = netIter->Next()) {
        std::cout << "net: " << net->GetName() << std::endl;
    }
    auto primIter = layout->GetPrimitiveIter();
    while (auto prim = primIter->Next()) {
        std::cout << *prim << std::endl;
    }
    auto iter = layout->GetLayerCollection()->GetLayerIter();
    while (auto layer = iter->Next())
        std::cout << "thickness: " << layer->GetStackupLayerFromLayer()->GetThickness() << std::endl;

    ELayoutPolygonMergeSettings mergeSettings;
    mergeSettings.outFile = ecad_test::GetTestDataPath() + "/simulation/thermal";
    layout->MergeLayerPolygons(mergeSettings);

    EThermalNetworkExtractionSettings extSettings;
    extSettings.outDir = ecad_test::GetTestDataPath() + "/simulation/thermal";
#ifdef BOOST_GIL_IO_PNG_SUPPORT
    extSettings.dumpHotmaps = true;
#endif//#ifdef BOOST_GIL_IO_PNG_SUPPORT
    extSettings.dumpDensityFile = true;
    extSettings.dumpTemperatureFile = true;

    size_t xGrid = 500;
    auto bbox = layout->GetBoundary()->GetBBox();
    extSettings.grid = {xGrid, static_cast<size_t>(xGrid * EValue(bbox.Width()) / bbox.Length())};
    extSettings.mergeGeomBeforeMetalMapping = false;

    esim::EThermalNetworkExtraction ne;
    ne.SetExtractionSettings(extSettings);
    ne.GenerateThermalNetwork(layout);

    EDataMgr::Instance().ShutDown();

    return EXIT_SUCCESS;
}