#pragma once

#include "json_reader.h"
#include "transport_catalogue.h"

#include <optional>

class RequestHandler {
public:
    RequestHandler(const transport::TransportCatalogue& catalogue, const JsonReader& requests, const MapRenderer& renderer)
        : catalogue_(catalogue), requests_(requests), renderer_(renderer) {
    }
    
    json::Dict GetRouteRequestResult(std::string_view bus_name, int request_id) const;
    
    json::Dict GetStopRequestResult(std::string_view stop_name, int request_id) const;
    
    json::Dict GetMapRequestResult(int request_id) const;
    
    void PrintRequestsResults(std::ostream& out) const;
    
    std::optional<transport::TransportCatalogue::RouteInfo> GetBusStat(const std::string_view& bus_name) const;

    const std::unordered_set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;
    
private:
    const transport::TransportCatalogue& catalogue_;
    const JsonReader& requests_;
    const MapRenderer& renderer_;
};
