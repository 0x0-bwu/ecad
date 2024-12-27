#include "utility/EFlattenUtility.h"

#include "generic/thread/TaskFlow.hpp"
#include "interface/ILayoutView.h"
#include "interface/IDatabase.h"
#include "interface/ICellInst.h"
#include "interface/ICell.h"
namespace ecad {
namespace utils {

bool EFlattenUtility::Flatten(Ptr<IDatabase> database, Ptr<ICell> cell, size_t threads)
{
    ECAD_EFFICIENCY_TRACK("flatten")

    using namespace ::generic::thread;
    if(nullptr == cell || nullptr == database) return false;
    auto cellNodeMap = BuildCellNodeMap(database);
    if(!(cellNodeMap->count(cell))) return false;

    auto flattenFlow = UPtr<FlattenFlow>(new FlattenFlow);
    ScheduleFlattenTasks(flattenFlow.get(), nullptr, *(cellNodeMap->at(cell)));

    taskflow::Executor executor(threads);
    return executor.Run(*flattenFlow);
}

UPtr<ECellNodeMap> EFlattenUtility::BuildCellNodeMap(Ptr<IDatabase> database)
{
    if(nullptr == database) return nullptr;

    auto cellNodeMap = UPtr<ECellNodeMap>(new ECellNodeMap);
    auto cellIter = database->GetCellIter();
    while(auto cell = cellIter->Next()){
        auto cellNode = UPtr<ECellNode>(new ECellNode(cell));
        cellNodeMap->insert(std::make_pair(cell, std::move(cellNode)));
    }

    cellIter = database->GetCellIter();
    while(auto cell = cellIter->Next()){
        const auto & successor = cellNodeMap->at(cell);
        auto cellInstIter = cell->GetLayoutView()->GetCellInstIter();
        while(auto cellInst = cellInstIter->Next()){
            auto defCell = cellInst->GetDefLayoutView()->GetCell();
            const auto & dependent = cellNodeMap->at(defCell);
            successor->dependents.push_back(dependent.get());
            dependent->successors.push_back(successor.get());
        }
    }
    return cellNodeMap;
}

bool EFlattenUtility::GetTopCells(Ptr<IDatabase> database, std::vector<Ptr<ICell> > & tops)
{
    tops.clear();
    auto cellNodeMap = BuildCellNodeMap(database);
    if(nullptr == cellNodeMap) return false;

    auto iter = cellNodeMap->begin();
    for(; iter != cellNodeMap->end(); ++iter){
        const auto & cellNode = iter->second;
        if(cellNode->successors.empty())
            tops.push_back(iter->first);
    }
    return not tops.empty();
}

void EFlattenUtility::ScheduleFlattenTasks(Ptr<FlattenFlow> flattenFlow, Ptr<FlattenNode> successor, const ECellNode & node)
{
    auto task = flattenFlow->Emplace(std::bind(&EFlattenUtility::FlattenOneCell, this, node.cell), node.cell->GetName());
    if(successor) task->Precede(successor);
    for(auto dependent : node.dependents)
        ScheduleFlattenTasks(flattenFlow, task, *dependent);
}

void EFlattenUtility::FlattenOneCell(Ptr<ICell> cell)
{
    std::lock_guard<std::mutex> lock(m_flattenMutex);
    cell->GetFlattenedLayoutView();
}

}//namespace utils
}//namespace ecad
