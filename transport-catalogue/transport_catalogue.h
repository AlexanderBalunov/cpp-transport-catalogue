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
    int length;
    double curvature;
};

class NearbyStopsHasher {
public:
    size_t operator()(std::pair<const Stop*, const Stop*> nearby_stops) const;
};    
    
class TransportCatalogue {
public:
    void AddStop(const std::string& stop_name, const geo::Coordinates& stop_coorditanes);
    void AddDistance(std::string_view stop_name, const std::unordered_map<std::string_view, int>& distances);
    int GetDistance(std::string_view stop_from, std::string_view stop_to) const;
    void AddRoute(const std::string& route_name, const std::vector<std::string>& route_stops);  
    const Stop* GetStop(std::string_view stop_name) const;
    const Route* GetRoute(std::string_view route_name) const;
    RouteInfo GetRouteInfo(std::string_view route_name) const; 
    std::unordered_set<std::string_view> GetRoutesThroughStop(std::string_view stop_name) const;
     
private:
    double CalculateGeoRouteLength(const Route& route) const;
    int CalculateRealRouteLength(const Route& route) const;
    
    std::deque<Stop> stops_;
    std::deque<Route> routes_;
    std::unordered_map<std::string_view, const Stop*> stop_info_by_stop_name_;
    std::unordered_map<std::string_view, const Route*> route_info_by_route_name_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> routes_through_stop_by_stop_name_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, NearbyStopsHasher> distances_between_stops_;    
};
    
}
