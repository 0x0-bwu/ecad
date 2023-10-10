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
    Text
};

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

enum class EFlattenOption
{

};

enum class ECloneOption
{
    Database = 0,
    Cell = 1,
    LayoutView = 2
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
        default : return std::string{};
    }
}

}//namespace ecad