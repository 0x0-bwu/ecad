#pragma once
#include "basic/ECadCommon.h"
#include <string_view>
#include <istream>

namespace ecad::ext::kicad {

struct Pin
{

};

struct Via
{

};

struct Segment
{

};

struct Net
{
    EInt64 id{-1};
    EInt64 netClassId{-1};
    std::string name{"unknown"};
    std::vector<Pin> pins;
    std::vector<Via> vias;
    std::vector<Segment> segments;
    EPair<EInt64, EInt64> diffPair;
    
    Net() = default;
    Net(EInt64 id, std::string name, EInt64 netClassId, EPair<EInt64, EInt64> diffPair)
     : id(id), netClassId(netClassId), name(std::move(name)), diffPair(std::move(diffPair))
    {
    }
};

struct NetClass
{
    EInt64 id{-1};
    std::string name{"unknown"};
    EFloat clearance{0};
    EFloat traceWidth{0};
    EFloat viaDia{0};
    EFloat viaDrill{0};
    EFloat uViaDia{0};
    EFloat uViaDrill{0};

    NetClass() = default;
    NetClass(EInt64 id, std::string name, EFloat clearance, EFloat traceWidth, EFloat viaDia, EFloat viaDrill, EFloat uViaDia, EFloat uViaDrill)
     : id(id), name(std::move(name)), clearance(clearance), traceWidth(traceWidth), viaDia(viaDia), viaDrill(viaDrill), uViaDia(uViaDia), uViaDrill(uViaDrill)
    {
    }
};
struct EKiCadDB
{
    std::vector<Net> nets;
    std::vector<NetClass> netClasses;
};

} // namespace ecad::ext::kicad