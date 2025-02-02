#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace transport {

struct RoutingSettings {
    double bus_velocity = 0.0;
    int bus_wait_time = 0;
};

struct EdgeInfo {
    double weight = 0.0;
    std::string_view bus_name;
    int span_count = 0;
    std::string_view start_stop;
    std::string_view finish_stop;
};
    
template <typename Weight>    
struct GraphAndItsTransportData {
    graph::DirectedWeightedGraph<Weight> graph;
    std::unordered_map<std::string_view, graph::VertexId> vertex_id_by_stop_name = {};
    std::unordered_map<graph::EdgeId, EdgeInfo> edge_info_by_edge_id = {};
};

struct PathInfo {
    std::vector<EdgeInfo> items;
    int bus_wait_time = 0;
    double total_time = 0;
};

class TransportRouter {
public:
    void SetSettings(RoutingSettings routing_settings);
    void UploadTransportData(const transport::TransportCatalogue& catalogue) const;
    std::optional<PathInfo> BuildPath(std::string_view stop_from, std::string_view stop_to) const;    
 
private:   
    RoutingSettings routing_settings_;
    mutable GraphAndItsTransportData<double> graph_;
    mutable std::unique_ptr<graph::Router<double>> router_;
};
    
} // namespace transport
