#ifndef ECAD_ECADDEF_H
#define ECAD_ECADDEF_H
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
    PadstackDef,
    HierarchyObj,
    PadstackInst,
    Collection
};

enum class ECellType
{
    Invalid = -1,
    CircuitCell = 0
};

enum ELayerId { noLayer = -1 };

enum ENetId { noNet = -1 };

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

enum class EPadType
{
    Invalid = -1,
    RegularPad = 0,
    AntiPad = 1
};

enum class EDefinitionType
{
    Invalid = -1,
    PadstackDef,
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

}//namespace
#endif//ECAD_ECADDEF_H