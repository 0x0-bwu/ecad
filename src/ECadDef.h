#pragma once
#include "ECadConfig.h"
#include <limits>
#include <string>
namespace ecad {

enum class EArchiveFormat { TXT = 0, XML = 1, BIN = 2};

enum class EObjType
{
    Invalid = -1
};

enum class ECollectionType
{
    Invalid = -1,
    Net,
    Cell,
    Layer,
    ConnObj,
    CellInst,
    LayerMap,
    Component,
    Primitive,
    Definition,
    MaterialDef,
    PadstackDef,
    ComponentDef,
    HierarchyObj,
    PadstackInst,
    ComponentDefPin,
    Collection
};

ECAD_ALWAYS_INLINE std::string toString(ECollectionType type)
{
    switch(type)
    {
        case ECollectionType::Invalid : return "Invalid";
        case ECollectionType::Net : return "Net";
        case ECollectionType::Cell : return "Cell";
        case ECollectionType::Layer : return "Layer";
        case ECollectionType::ConnObj : return "Connect Object";
        case ECollectionType::CellInst : return "Cell Instance";
        case ECollectionType::LayerMap : return "LayerMap";
        case ECollectionType::Component : return "Component";
        case ECollectionType::Primitive : return "Primitive";
        case ECollectionType::Definition : return "Definition";
        case ECollectionType::MaterialDef : return "Material Definition";
        case ECollectionType::PadstackDef : return "Padstack Definition";
        case ECollectionType::ComponentDef : return "Component Definition";
        case ECollectionType::HierarchyObj : return "Hierarchy Object";
        case ECollectionType::PadstackInst : return "Padstack Instance";
        case ECollectionType::ComponentDefPin : return "Component Definition Pin";
        case ECollectionType::Collection : return "Collection";
        default : { ECAD_ASSERT(false) return std::string{}; }
    }
}

enum class ECellType
{
    Invalid = -1,
    CircuitCell = 0
};

enum ELayerId { ComponentLayer = -10, noLayer = -1 };

enum ENetId { noNet = -1 };

enum EMaterialId { noMaterial = -1 };

enum EMaterialType { Rigid, Fluid };

enum EMaterialPropId                   // unit in SI
{
    Permittivity = 3,                  //F/m
    Permeability = 4,                  //H/m
    Conductivity = 5,                  //S/m
    DielectricLossTangent = 6,
    MagneticLossTangent = 7,
    Resistivity = 8,                   //ohm·m
    ThermalConductivity = 14,          //W/(m·K)
    MassDensity = 34,                  //kg/m^3
    SpecificHeat = 38,                 //J/(kg·K)
    YoungsModulus = 39,                //Pa
    PoissonsRatio = 40,
    ThermalExpansionCoefficient = 42,  //1/K
};

enum class EComponentType
{
    Invalid = -1,
    Other = 0,
    Resistor,
    Inductor,
    Capacitor,
    IC,
    IO,
    Molding
};

ECAD_ALWAYS_INLINE std::string toString(EComponentType type)
{
    switch(type)
    {
        case EComponentType::Invalid : return "Invalid";
        case EComponentType::Other : return "Other";
        case EComponentType::Resistor : return "Resistor";
        case EComponentType::Inductor : return "Inductor";
        case EComponentType::Capacitor : return "Capacitor";
        case EComponentType::IC : return "IC";
        case EComponentType::IO : return "IO";
        case EComponentType::Molding : return "Molding";
        default : { ECAD_ASSERT(false) return std::string{}; }
    }
}

enum class EPinIOType
{
    Invalid = -1,
    Driver,
    Receiver,
    BiDirectional,
    DirverTerminator,
    ReceiverTerminator
};

enum class ELayerType
{
    Invalid = -1,
    DielectricLayer,
    ConductingLayer,
    MetalizedSignal = ConductingLayer
};

enum class EPrimitiveType
{
    Invalid = -1,
    Geometry2D,
    Bondwire,
    Text
};

enum class EBondwireType
{
    Invalid = -1,
    Simple = 0,
    JEDEC4,
    // JEDEC5
};

ECAD_ALWAYS_INLINE std::string toString(EPrimitiveType type)
{
    switch(type)
    {
        case EPrimitiveType::Invalid : return "Invalid";
        case EPrimitiveType::Geometry2D : return "Geometry2D";
        case EPrimitiveType::Bondwire : return "Bondwire";
        case EPrimitiveType::Text : return "Text";
        default : { ECAD_ASSERT(false) return std::string{}; }
    }
}

enum class EShapeType
{
    Invalid = -1,
    Rectangle,
    Path,
    Circle,
    Polygon,
    PolygonWithHoles,
    FromTemplate
};

enum class EPadType
{
    Invalid = -1,
    RegularPad = 0,
    AntiPad = 1
};

enum class EDefinitionType
{
    Invalid = -1,
    ComponentDef,
    PadstackDef,
    MaterialDef,
    LayerMap,
    Cell
};

ECAD_ALWAYS_INLINE std::string toString(EDefinitionType type)
{
    switch(type)
    {
        case EDefinitionType::Invalid : return "Invalid";
        case EDefinitionType::ComponentDef : return "Component Def";
        case EDefinitionType::PadstackDef : return "Padstack Def";
        case EDefinitionType::MaterialDef : return "Material Def";
        case EDefinitionType::LayerMap : return "Layer Map";
        case EDefinitionType::Cell : return "Cell";
        default : { ECAD_ASSERT(false) return std::string{}; }
    }
}

enum class EFlattenOption
{

};

enum class ECloneOption
{
    Database = 0,
    Cell = 1,
    LayoutView = 2
};

enum class EModelType
{
    Invalid = -1,
    ThermalCTMv1 = 0,
    ThermalGrid = 1,
    ThermalPrisma = 2,
    // ThermalTet. //todo
};

}//namespace ecad