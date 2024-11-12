#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

#include <optional>

class RequestHandler {
public:
    RequestHandler(const transport::TransportCatalogue& catalogue, const MapRenderer& renderer)
        : catalogue_(catalogue), renderer_(renderer) {
    }
    
    std::optional<transport::TransportCatalogue::RouteInfo> GetBusStat(const std::string_view& bus_name) const;

    const std::unordered_set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;
    
private:
    const transport::TransportCatalogue& catalogue_;
    const MapRenderer& renderer_;
};
