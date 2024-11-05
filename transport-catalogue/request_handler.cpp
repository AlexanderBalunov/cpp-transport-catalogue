#include "request_handler.h"

#include <set>
#include <sstream>

using namespace std::literals;

json::Dict RequestHandler::GetRouteRequestResult(std::string_view bus_name, int request_id) const {
    if (!GetBusStat(bus_name)) {
        return {{"request_id"s, request_id}, {"error_message"s, "not found"s}};
    }
    const auto route_info = *GetBusStat(bus_name);
    return {{"request_id"s, request_id},
            {"stop_count"s, route_info.number_of_stops},
            {"unique_stop_count"s, route_info.number_of_unique_stops},
            {"route_length"s, route_info.length},
            {"curvature"s, route_info.curvature}};       
}

json::Dict RequestHandler::GetStopRequestResult(std::string_view stop_name, int request_id) const {
    const auto routes_un_set_ptr = GetBusesByStop(stop_name);
    if (!routes_un_set_ptr) {
        return {{"request_id"s, request_id}, {"error_message"s, "not found"s}};
    }
    std::set<std::string> routes_set(routes_un_set_ptr->begin(), routes_un_set_ptr->end());
    json::Array routes_vec;
    for (const auto& route : routes_set) {
        routes_vec.push_back(route);
    }
    return {{"request_id"s, request_id}, {"buses"s, routes_vec}};
}

json::Dict RequestHandler::GetMapRequestResult(int request_id) const {
    std::ostringstream oss;
    RenderMap().Render(oss);
    return {{"request_id"s, request_id}, {"map"s, oss.str()}};
}

void RequestHandler::PrintRequestsResults(std::ostream& out) const {
    json::Array result;
    const auto& stat_requests_array = requests_.GetDocument().GetRoot().AsMap().at("stat_requests"s).AsArray();
    for (const auto& stat_request : stat_requests_array) {
        const auto& stat_request_map = stat_request.AsMap();
        if (stat_request_map.at("type"s).AsString() == "Bus"s) {
            result.push_back(GetRouteRequestResult(stat_request_map.at("name"s).AsString(), 
                                                     stat_request_map.at("id"s).AsInt()));
        }
        if (stat_request_map.at("type"s).AsString() == "Stop"s) {
            result.push_back(GetStopRequestResult(stat_request_map.at("name"s).AsString(), 
                                                    stat_request_map.at("id"s).AsInt()));
        }
        if (stat_request_map.at("type"s).AsString() == "Map"s) {
            result.push_back(GetMapRequestResult(stat_request_map.at("id"s).AsInt()));
        }             
    }
    json::Print(json::Document{result}, out);
}

std::optional<transport::TransportCatalogue::RouteInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    const auto bus_stat = catalogue_.GetRouteInfo(bus_name);
    if (bus_stat.number_of_stops == 0) {
        return std::nullopt;
    }
    return bus_stat;
}

const std::unordered_set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return catalogue_.GetRoutesThroughStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.MakeSvgDocument(catalogue_.GetAllStops(), catalogue_.GetAllRoutes());
}
