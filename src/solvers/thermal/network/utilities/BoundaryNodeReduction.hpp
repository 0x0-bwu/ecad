#pragma once
#include "solvers/thermal/network/ThermalNetwork.hpp"
namespace thermal::utils {

using namespace model;
using namespace generic;
template <typename num_type> 
class BoundaryNodeReduction
{
public:
    inline static constexpr num_type FAKE_RES = 0.01;
    inline static constexpr num_type FAKE_CAP = 0.01;
    explicit BoundaryNodeReduction(ThermalNetwork<num_type> & network)
     : m_network(network)
    {
    }

    void Reduce(num_type minT, num_type maxT, size_t discretization = 10)
    {
        size_t nodes = m_network.Size();
        std::unordered_map<size_t, size_t> nodeMap;
        num_type range = (maxT - minT) / discretization;
        for (size_t i = 0; i < nodes; ++i) {
            auto & node = m_network[i];
            if (node.t < minT || maxT < node.t) continue;
            auto id = static_cast<size_t>((node.t - minT) / range);
            auto iter = nodeMap.find(id);
            if (iter == nodeMap.cend()) {
                auto nid = m_network.AppendNode();
                iter = nodeMap.emplace(id, nid).first;
            }
            m_network.SetC(iter->second, FAKE_CAP);
            m_network.SetR(iter->second, i, FAKE_RES);
            m_network.AddHTC(iter->second, node.htc);
            m_network.SetHTC(i, 0);
        }
    }

private:
    ThermalNetwork<num_type> & m_network;
};
} // thermal::utils