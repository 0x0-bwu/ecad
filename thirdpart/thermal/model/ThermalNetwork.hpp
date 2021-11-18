#ifndef THERMAL_MODEL_THERMALNETWORK_HPP
#define THERMAL_MODEL_THERMALNETWORK_HPP

#include "generic/thread/ThreadPool.hpp"
#include "generic/math/MathUtility.hpp"
#include "generic/topology/IndexGraph.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <unordered_map>
#include <queue>
#include <map>
// namespace boost {
//     enum vertex_thermal_heat_flux_t { vertex_thermal_heat_flux = 10001 };
//     enum vertex_thermal_heat_transfor_coeff_t { vertex_thermal_heat_transfor_coeff = 10002 };
//     enum edge_thermal_resistance_coeff_t { edge_thermal_resistance_coeff = 10003 };

//     BOOST_INSTALL_PROPERTY(vertex, thermal_heat_flux);
//     BOOST_INSTALL_PROPERTY(vertex, thermal_heat_transfor_coeff);
//     BOOST_INSTALL_PROPERTY(edge, thermal_resistance_coeff);
// }
namespace thermal {
namespace model {

// using ThermalNetwork =  boost::adjacency_list<
//                         boost::setS,
//                         boost::vecS,
//                         boost::undirectedS,
//                         //vertex properties
//                         boost::property<boost::vertex_index_t, size_t, 
//                         boost::property<boost::vertex_thermal_heat_flux_t, double,
//                         boost::property<boost::vertex_thermal_heat_transfor_coeff_t, double> > >,
//                         //edge property
//                         boost::property<boost::edge_thermal_resistance_coeff_t, double>
//                         >;

class ThermalNetwork
{
public:
    struct Node
    {
        double hf = 0;
        double htc = 0;
        std::vector<size_t> ns;
        std::vector<double> rs;
        size_t Freedom() const
        {
            return 6 - ns.size();
        }
    };

    struct Edge
    {
        size_t x, y;
        double r;
    };

    explicit ThermalNetwork(size_t nodes)
    {
        m_nodes.assign(nodes, Node{});
    }
    virtual ~ThermalNetwork() = default;

    size_t Size() const
    {
        return m_nodes.size();
    }

    const std::vector<size_t> & NS(size_t node) const
    {
        return m_nodes[node].ns;
    }

    size_t NodeFreedom(size_t node) const
    {
        return m_nodes[node].Freedom();
    }

    void SetHF(size_t node, double hf)
    {
        m_nodes[node].hf = hf;
    }

    double GetHF(size_t node) const
    {
        return m_nodes[node].hf;
    }

    void SetHTC(size_t node, double htc)
    {
        m_nodes[node].htc = htc;
    }

    double GetHTC(size_t node) const
    {
        return m_nodes[node].htc;
    }

    void SetR(size_t node1, size_t node2, double r)
    {
        m_nodes[node1].ns.push_back(node2);
        m_nodes[node1].rs.push_back(r);

        m_nodes[node2].ns.push_back(node1);
        m_nodes[node2].rs.push_back(r);
    }

    double GetDiagCoeff(size_t index) const
    {
        double coeff = 0;
        for(const auto & r : m_nodes[index].rs)
            coeff += r;
        coeff += m_nodes[index].htc;
        return coeff;
    }

    double GetCoeff(size_t row, size_t col) const
    {
        const auto & ns = m_nodes[row].ns;
        for(size_t i = 0; i < ns.size(); ++i){
            if(ns[i] == col) return -(m_nodes[row].rs[i]);
        }
        return 0;
    }

    double GetRhs(size_t node, const double & refT) const
    {
        return m_nodes[node].hf + m_nodes[node].htc * refT;
    }

    void GetCoeffsBFS(std::list<Edge> & edges) const
    {
        edges.clear();
        auto size = m_nodes.size();
        auto mark = std::unique_ptr<std::vector<bool> >(new std::vector<bool>(size, false));
        GetCoeffsBFS(0, *mark, edges);
    }

    void GetCoeffsBFS(size_t node, std::vector<bool> & mark, std::list<Edge> & edges) const
    {
        auto queue = std::unique_ptr<std::queue<size_t> >(new std::queue<size_t>{});
        mark[node] = true;
        queue->push(node);
        while(!(queue->empty())){
            size_t v = queue->front();
            queue->pop();
            const auto & ns = m_nodes[v].ns;
            const auto & rs = m_nodes[v].rs;
            for(size_t i = 0; i < ns.size(); ++i){
                edges.push_back(Edge{v, ns[i], -rs[i]});
                if(!mark[ns[i]]){
                    mark[ns[i]] = true;
                    queue->push(ns[i]);
                }
            }
        }
    }
    
    void GetDiagCoeffsMT(std::vector<double> & diagCoeffs) const
    {
        size_t size = m_nodes.size();
        diagCoeffs.resize(size);

        generic::thread::ThreadPool pool;
        size_t blocks = pool.Threads();
        if(0 == blocks) blocks = 1;
        size_t blockSize = size / blocks;

        size_t begin = 0, end = 0;
        for(size_t i = 0; i < blocks && blockSize > 0; ++i){
            end = begin + blockSize;
            pool.Submit(std::bind(&ThermalNetwork::GetDiagCoeffs, this, std::ref(diagCoeffs), begin, end));
            begin = end;
        }
        end = size;
        if(begin != end)
            pool.Submit(std::bind(&ThermalNetwork::GetDiagCoeffs, this, std::ref(diagCoeffs), begin, end));
        
        pool.Wait();
    }

    void GetDiagCoeffs(std::vector<double> & diagCoeffs, size_t begin, size_t end) const
    {
        for(size_t i = begin; i < end; ++i)
            diagCoeffs[i] = GetDiagCoeff(i);
    }

private:
    std::vector<Node> m_nodes;
};

}//namespace model
}//namespace thermal
#endif//THERMAL_MODEL_THERMALNETWORK_HPP
