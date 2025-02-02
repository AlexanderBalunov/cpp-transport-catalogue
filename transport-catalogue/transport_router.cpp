#include "transport_router.h"

#include <cmath>
#include <stdexcept>

namespace transport {
    
void TransportRouter::SetSettings(RoutingSettings routing_settings) {
    routing_settings_ = routing_settings;
}

void TransportRouter::UploadTransportData(const transport::TransportCatalogue& catalogue) const {
    const std::unordered_map<std::string_view, const Stop*>& all_stops = catalogue.GetAllStops();
    const std::unordered_map<std::string_view, const Route*>& all_routes = catalogue.GetAllRoutes();
    GraphAndItsTransportData<double> graph_temp{graph::DirectedWeightedGraph<double>(all_stops.size())};
    size_t index_number_of_stop = 0;
    for (auto [stop_name, stop_ptr] : all_stops) {
        graph_temp.vertex_id_by_stop_name[stop_ptr->name] = index_number_of_stop;
        ++index_number_of_stop;
    }
    for (auto [route_name, route_ptr] : all_routes) {
        const auto& vec_stops = route_ptr->stops;
        if (route_ptr->is_roundtrip) {
            for (size_t index_stop_from = 0; index_stop_from < vec_stops.size(); ++index_stop_from) {
                for (size_t index_stop_to = 0; index_stop_to < vec_stops.size(); ++index_stop_to) {
                    if (index_stop_from >= index_stop_to || (index_stop_from == 0 && index_stop_to == vec_stops.size() - 1)) {
                        continue;
                    }
                    const graph::VertexId id_stop_from = graph_temp.vertex_id_by_stop_name[vec_stops[index_stop_from]];
                    const graph::VertexId id_stop_to = graph_temp.vertex_id_by_stop_name[vec_stops[index_stop_to]];
                    uint32_t distance = 0;
                    for (size_t i = index_stop_from; i != index_stop_to; ++i) {
                        distance += catalogue.GetDistance(vec_stops[i], vec_stops[i + 1]);     
                    }                       
                    const double weight = ((distance*60)/(1000*routing_settings_.bus_velocity)) + routing_settings_.bus_wait_time;
                    graph_temp.graph.AddEdge({id_stop_from, id_stop_to, weight});
                    const int span_count = index_stop_to - index_stop_from;
                    graph_temp.edge_info_by_edge_id[graph_temp.graph.GetEdgeCount() - 1] = 
                    {weight - routing_settings_.bus_wait_time, route_name, span_count, vec_stops[index_stop_from], vec_stops[index_stop_to]};
               }
            }
        } else {
            for (size_t index_stop_from = 0; index_stop_from < vec_stops.size(); ++index_stop_from) {
                for (size_t index_stop_to = 0; index_stop_to < vec_stops.size(); ++index_stop_to) {
                    if (index_stop_from != index_stop_to) {
                        const graph::VertexId id_stop_from = graph_temp.vertex_id_by_stop_name[vec_stops[index_stop_from]];
                        const graph::VertexId id_stop_to = graph_temp.vertex_id_by_stop_name[vec_stops[index_stop_to]];
                        uint32_t distance = 0;
                        if (index_stop_from < index_stop_to) {
                            for (size_t i = index_stop_from; i != index_stop_to; ++i) {
                                distance += catalogue.GetDistance(vec_stops[i], vec_stops[i + 1]);     
                            }    
                        } else {
                            for (size_t i = index_stop_from; i != index_stop_to; --i) {
                                distance += catalogue.GetDistance(vec_stops[i], vec_stops[i - 1]);     
                            }         
                        }                       
                        const double weight = ((distance*60)/(1000*routing_settings_.bus_velocity)) + routing_settings_.bus_wait_time;
                        graph_temp.graph.AddEdge({id_stop_from, id_stop_to, weight});
                        const int span_count = std::abs(static_cast<int>(index_stop_to - index_stop_from));
                        graph_temp.edge_info_by_edge_id[graph_temp.graph.GetEdgeCount() - 1] = 
                        {weight - routing_settings_.bus_wait_time, route_name, span_count, vec_stops[index_stop_from], vec_stops[index_stop_to]};
                    }
                }
            }
        }
    }
    graph_ = std::move(graph_temp);
    router_ = std::make_unique<graph::Router<double>>(graph_.graph);
}

std::optional<PathInfo> TransportRouter::BuildPath(std::string_view stop_from, std::string_view stop_to) const {
    if (!router_) {
        throw std::logic_error(std::string("Information about stops and routes has not been added to the TransportRouter"));
    }        
    const auto route_info = router_->BuildRoute(graph_.vertex_id_by_stop_name.at(stop_from), 
                                                graph_.vertex_id_by_stop_name.at(stop_to));
    if (!route_info) {
        return std::nullopt;   
    } else {
        std::vector<EdgeInfo> items;
        for (graph::EdgeId edge_id : route_info->edges) {
            items.push_back(graph_.edge_info_by_edge_id.at(edge_id));
        }
        return PathInfo{items, routing_settings_.bus_wait_time, route_info->weight};
    }
}       
    
} // namespace transport
