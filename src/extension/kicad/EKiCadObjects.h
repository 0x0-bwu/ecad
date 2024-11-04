#pragma once
#include "basic/ECadCommon.h"
#include "basic/ELookupTable.h"
#include <string_view>
#include <istream>

namespace ecad::ext::kicad {

ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_BOTTOM_ADHES_ID = 32;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_FRONT_ADHES_ID = 33;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_BOTTOM_PASTE_ID = 34;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_FRONT_PASTE_ID  = 35;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_BOTTOM_SILKS_ID = 36;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_FRONT_SILKS_ID = 37;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_BOTTOM_MASK_ID = 38;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_FRONT_MASK_ID = 39;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_EDGE_CUT_ID = 44;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_BOTTOM_CRTYD_ID = 46;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_FRONT_CRTYD_ID = 47;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_BOTTOM_FAB_ID = 48;
ECAD_ALWAYS_INLINE static constexpr EIndex ECAD_KICAD_PCB_LAYER_FRONT_FAB_ID = 49;
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_BOTTOM_ADHES_STR = "B.Adhes";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_FRONT_ADHES_STR = "F.Adhes";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_BOTTOM_PASTE_STR = "B.Paste";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_FRONT_PASTE_STR = "F.Paste";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_BOTTOM_SILKS_STR = "B.SilkS";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_FRONT_SILKS_STR = "F.SilkS";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_BOTTOM_MASK_STR = "B.Mask";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_FRONT_MASK_STR = "F.Mask";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_EDGE_CUT_STR = "Edge.Cuts";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_BOTTOM_CRTYD_STR = "B.CrtYd";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_FRONT_CRTYD_STR = "F.CrtYd";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_BOTTOM_FAB_STR = "B.Fab";
ECAD_ALWAYS_INLINE static constexpr std::string_view ECAD_KICAD_PCB_LAYER_FRONT_FAB_STR = "F.Fab";

using Points = std::vector<FPoint2D>;
struct Stroke
{
    enum class Type { UNKNOWN, SOLID };
    enum class Fill { UNKNOWN, SOLID };
    Type type{Type::UNKNOWN};
    Fill fill{Fill::UNKNOWN}; 
    EIndex layer{invalidIndex};
    EFloat width{0};
    virtual ~Stroke() = default;
    virtual void SetType(const std::string & str);
    virtual void SetFill(const std::string & str);
};

struct Arc : public Stroke
{
    EFloat angle{0};
    FPoint2D start{0, 0}, end{0, 0};
};

struct Line : public Stroke
{ 
    ECoord angle{0};
    FPoint2D start{0, 0}, end{0, 0};
};

struct Circle : public Stroke
{
    //width = 2 * radius
    FPoint2D center{0, 0}, end{0, 0};
};

struct Poly : public Stroke
{   
    Points shape;
};

struct Text
{
    bool refOrValue{true};//ref=true
    bool hide{false};
    EIndex layer{invalidIndex};
    FPoint2D loc{0, 0};
    std::string text{};
};

struct Layer
{
    enum class Group { UNKNOWN, POWER, SIGNAL, USER };
    enum class Type { UNKNOWN, SILK_SCREEN, SOLDER_PASTE, SOLDER_MASK, CONDUCTING, DIELECTRIC, MIXED };
    EIndex id{invalidIndex};
    Group group{Group::UNKNOWN};
    Type type{Type::UNKNOWN};
    EFloat thickness{0};
    EFloat epsilonR{0};
    EFloat lossTangent{0};
    std::string attr;
    std::string name;
    std::string material;

    Layer(EIndex id, std::string name) : id(id), name(std::move(name)) {}

    void SetGroup(const std::string & str);
    void SetType(const std::string & str);
};

struct Via
{
    enum class Type { UNKNOWN, THROUGH, MICRO, BLIND_BURIED };
    Type type{Type::UNKNOWN};    
    EIndex netId{invalidIndex};
    EFloat size{.0};
    EFloat drillSize{.0};
    FPoint2D pos{0, 0};
    std::array<EIndex, 2> layers{invalidIndex, invalidIndex};
};
 
struct Segment
{
    EIndex netId{invalidIndex};
    EIndex layerId{invalidIndex};
    EFloat width{0};
    FPoint2D start{0, 0};
    FPoint2D end{0, 0};
};

struct Zone
{
    EIndex netId{invalidIndex};
    EIndex layerId{invalidIndex};
    Points polygon;
    std::vector<Points> filledPolygons;
};

struct Pad
{
    enum class Type { UNKNOWN, SMD, THRU_HOLE, CONNECT, NP_THRU_HOLE };
    enum class Shape { UNKNOWN, RECT, ROUNDRECT, CIRCLE, OVAL, TRAPEZOID }; 
    Type type{Type::UNKNOWN};
    Shape shape{Shape::UNKNOWN};
    EIndex netId;
    EFloat angle{0};
    EFloat roundrectRatio{0};
    FPoint2D pos;
    FPoint2D size;
    std::string name{};
    Points shapePolygon;
    std::vector<EIndex> layers;
    void SetType(const std::string & str);
    void SetShape(const std::string & str);
};

struct Net
{
    EIndex id{invalidIndex};
    EIndex netClassId{invalidIndex};
    std::string name;
    std::vector<Via> vias;
    std::vector<Segment> segments;
    EPair<EIndex, EIndex> diffPair;
    Net(EIndex id, std::string name) : id(id), name(std::move(name)) {}
};

struct Component
{
    bool flipped{false};
    EIndex layerId{invalidIndex};
    FPoint2D location{0, 0};
    EFloat angle{0};
    EFloat width{0};
    EFloat height{0};
    std::string name;
    std::vector<Via> vias;
    std::vector<Arc> arcs;
    std::vector<Pad> pads;
    std::vector<Line> lines;
    std::vector<Poly> polys;
    std::vector<Zone> zones;
    std::vector<Circle> circles;
    std::vector<Segment> segments;

    Component() = default;
    Component(std::string name) : name(std::move(name)) {}
    virtual ~Component() = default;
};

struct Database : public Component
{
    std::unordered_map<std::string, Component> components;
    std::unordered_map<std::string, Layer> layers;
    std::unordered_map<EIndex, Net> nets;

    // lut
    std::unordered_map<std::string_view, Ptr<Net>> netLut;

    // add
    Layer & AddLayer(EIndex id, std::string name);
    Net & AddNet(EIndex id, std::string name);
    Component & AddComponent(std::string name);

    // find
    Ptr<Net> FindNet(EIndex id);
    Ptr<Net> FindNet(const std::string & name);
    Ptr<Layer> FindLayer(const std::string & name);   
};

} // namespace ecad::ext::kicad