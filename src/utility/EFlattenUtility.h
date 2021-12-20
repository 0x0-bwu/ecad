#ifndef ECAD_EUIT_EFLATTENUTILITY_H
#define ECAD_EUIT_EFLATTENUTILITY_H
#include "ECadCommon.h"
#include <unordered_map>
#include <mutex>
#include <list>
namespace generic { namespace thread { namespace taskflow { class TaskNode; class TaskFlow; } } }
namespace ecad {
class ICell;
class IDatabase;
namespace euti {
class ECAD_API ECellNode
{
public:
    explicit ECellNode(Ptr<ICell> _cell) : cell(_cell) {}
    virtual ~ECellNode() = default;
    Ptr<ICell> cell;
    std::list<CPtr<ECellNode> > successors;
    std::list<CPtr<ECellNode> > dependents;
};

using ECellNodeMap = std::unordered_map<Ptr<ICell>, UPtr<ECellNode> >;
class ECAD_API EFlattenUtility
{
    using FlattenNode = generic::thread::taskflow::TaskNode;
    using FlattenFlow = generic::thread::taskflow::TaskFlow;
public:
    virtual ~EFlattenUtility() = default;
    bool Flatten(Ptr<IDatabase> database, Ptr<ICell> cell, size_t threads = 1);
    static UPtr<ECellNodeMap> BuildCellNodeMap(Ptr<IDatabase> database);
    static bool GetTopCells(Ptr<IDatabase> database, std::vector<Ptr<ICell> > & tops);

private:
    void ScheduleFlattenTasks(Ptr<FlattenFlow> flattenFlow, Ptr<FlattenNode> successor, const ECellNode & node);
    void FlattenOneCell(Ptr<ICell> cell);
private:
    mutable std::mutex m_flattenMutex;
};

}//namespace utility
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "utility/EFlattenUtility.cpp"
#endif

#endif//ECAD_EUIT_EFLATTENUTILITY_H