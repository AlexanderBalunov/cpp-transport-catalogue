#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Route {
    std::string name;
    std::vector<std::string> stops;
};

struct RouteInfo {
    size_t number_of_stops;
    size_t number_of_unique_stops;
    double length;
};

class TransportCatalogue {
public:
    void AddStop(const std::string& stop_name, const geo::Coordinates& stop_coorditanes);    
    void AddRoute(const std::string& route_name, const std::vector<std::string>& route_stops);   
    const Stop* GetStop(const std::string_view& stop_name) const;
    const Route* GetRoute(const std::string_view& route_name) const;
    RouteInfo GetRouteInfo(const std::string_view& route_name) const; 
    std::unordered_set<std::string_view> GetRoutesThroughStop(const std::string& stop_name) const;
     
private:
    double CalculateRouteLength(const Route& route) const;
    
    std::deque<Stop> stops_;
    std::deque<Route> routes_;
    std::unordered_map<std::string_view, const Stop*> stop_info_by_stop_name_;
    std::unordered_map<std::string_view, const Route*> route_info_by_route_name_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> routes_through_stop_by_stop_name_;
};
    
}
