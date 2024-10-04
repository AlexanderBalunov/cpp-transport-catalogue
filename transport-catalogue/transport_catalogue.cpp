#include "transport_catalogue.h"

#include <algorithm>

namespace transport {

void TransportCatalogue::AddStop(const std::string& stop_name, const geo::Coordinates& stop_coordinates) {
    stops_.push_back({stop_name, stop_coordinates});
    stop_info_by_stop_name_[stops_.back().name] = &stops_.back();
    routes_through_stop_by_stop_name_[stops_.back().name];
}

void TransportCatalogue::AddRoute(const std::string& route_name, const std::vector<std::string>& route_stops) {
    routes_.push_back({route_name, route_stops});
    route_info_by_route_name_[routes_.back().name] = &routes_.back();       
    for (const std::string& stop_name : route_stops) {
        routes_through_stop_by_stop_name_.at(stop_name).insert(routes_.back().name);
    }    
}

const Stop* TransportCatalogue::GetStop(const std::string_view& stop_name) const {
    if (!stop_info_by_stop_name_.count(stop_name)) {
        return nullptr;
    }
    return stop_info_by_stop_name_.at(stop_name);
}

const Route* TransportCatalogue::GetRoute(const std::string_view& route_name) const {
    if (!route_info_by_route_name_.count(route_name)) {
        return nullptr;
    }
    return route_info_by_route_name_.at(route_name);
}

RouteInfo TransportCatalogue::GetRouteInfo(const std::string_view& route_name) const {
    if (!route_info_by_route_name_.count(route_name)) {
        return {0, 0, 0.0};
    }
    return {GetRoute(route_name)->stops.size(),
           std::unordered_set(GetRoute(route_name)->stops.begin(), GetRoute(route_name)->stops.end()).size(), 
           CalculateRouteLength(*route_info_by_route_name_.at(route_name))};
}

std::unordered_set<std::string_view> TransportCatalogue::GetRoutesThroughStop(const std::string& stop_name) const {
    if (!routes_through_stop_by_stop_name_.count(stop_name)) {
        return {};
    }
    return routes_through_stop_by_stop_name_.at(stop_name);
}

double TransportCatalogue::CalculateRouteLength(const Route& route) const {
    std::vector<geo::Coordinates> stops_coordinates;
    stops_coordinates.reserve(route.stops.size());
    for (const std::string& stop_name : route.stops) {
        stops_coordinates.push_back(GetStop(stop_name)->coordinates);
    }
    double route_length = 0.0;
    for (size_t i = 0; i < stops_coordinates.size() - 1; ++i) {
        route_length += geo::ComputeDistance(stops_coordinates[i], stops_coordinates[i + 1]);
    }
    return route_length;
}
    
}
