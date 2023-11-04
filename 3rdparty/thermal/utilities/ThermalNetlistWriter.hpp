#pragma once
#include "thermal/model/ThermalNetwork.hpp"
#include "generic/math/MathUtility.hpp"
#include <filesystem>
#include <fstream>
#include <ios>
namespace thermal::utils {

using namespace model;
using namespace generic;
template <typename num_type> 
class ThermalNetlistWriter
{
public:
    struct SimulationSettings
    {
        num_type refT{25};
        num_type stepTime{0.1};
        num_type totalTime{10};
    };

    explicit ThermalNetlistWriter(const ThermalNetwork<num_type> & network)
     : m_network(network) {}

    virtual ~ThermalNetlistWriter() = default;

    void SetReferenceTemperature(num_type t) { m_settings.refT = t; }

    bool WriteSpiceNetlist(std::string_view filename) const
    {
        namespace sf = std::filesystem;
        sf::create_directories(sf::path(filename).parent_path());

        const char sp(32);
        const char eol('\n');
        const std::string ref("REF");
        std::ofstream out(filename.data());
        if (not out.is_open()) return false;
        
        auto node = [](size_t id){ return std::string("N") + std::to_string(id); };        
        auto formatOs = [&out](size_t w, auto&&... args) { ((out << sp << std::setw(w) << std::left << args), ...) << eol; };

        const size_t w = 10;
        formatOs(0, "* SPICE SIMULATION");
        formatOs(w, "V" + ref, ref, 0, m_settings.refT);        
        const auto & nodes = m_network.GetNodes();
        size_t sIndex = nodes.size();
        for (size_t i = 0; i < nodes.size(); ++i) {
            const auto & nd = nodes.at(i);

            //cap
            if (nd.c > 0) {
                auto name = "C" + std::to_string(i);
                formatOs(w, name, node(i), ref, nd.c);
            }

            //res
            for (size_t j = 0; j < nd.ns.size(); ++j) {
                if (auto n = nd.ns.at(j); n > i) {
                    auto name = "R" + std::to_string(i) + "_" + std::to_string(n);
                    formatOs(w, name, node(i), node(n), nd.rs.at(j));
                }
            }

            //t
            if (math::NE(nd.t, ThermalNetwork<num_type>::unknownT)) {
                auto name  = "V" + std::to_string(i);
                formatOs(w, name, node(i), ref, nd.t);
            }

            //hf
            if (nd.hf != 0) {
                auto name = "I" + std::to_string(sIndex) + "_" + std::to_string(i);
                formatOs(w, name, node(sIndex), node(i), nd.hf);//todo dynamic
            }

            //htc
            if (nd.htc != 0) {
                auto name = "R" + std::to_string(i) + "_" + ref;
                formatOs(w, name, node(i), ref, 1 / nd.htc);
            }
        }

        formatOs(0, ".ic", "V(" + node(0) + ")=" + std::to_string(m_settings.refT));
        formatOs(0, ".tran", m_settings.stepTime, m_settings.totalTime);
        
        out.close();
        return true;
    }
private:
    SimulationSettings m_settings;
    const ThermalNetwork<num_type> & m_network;
};

} // namespace thermal::utils