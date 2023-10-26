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
        default : { GENERIC_ASSERT(false) return std::string{}; }
    }
}

enum class ECellType
{
    Invalid = -1,
    CircuitCell = 0
};

enum ELayerId { noLayer = -1 };

enum ENetId { noNet = -1 };

enum EMaterialPropId
{
    Permittivity = 3,
    Permeability = 4,
    Conductivity = 5,
    DielectricLossTangent = 6,
    MagneticLossTangent = 7,
    ThermalConductivity = 14,
    MassDensity = 34,
    SpecificHeat = 38,
    YoungsModulus = 39,
    PoissonsRatio = 40,
    ThermalExpansionCoefficient = 42,
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

ECAD_ALWAYS_INLINE std::string toString(EPrimitiveType type)
{
    switch(type)
    {
        case EPrimitiveType::Invalid : return "Invalid";
        case EPrimitiveType::Geometry2D : return "Geometry2D";
        case EPrimitiveType::Bondwire : return "Bondwire";
        case EPrimitiveType::Text : return "Text";
        default : { GENERIC_ASSERT(false) return std::string{}; }
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
        default : { GENERIC_ASSERT(false) return std::string{}; }
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

}//namespace ecad