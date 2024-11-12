#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

class JsonReader {
public:
    JsonReader(std::istream& input)
        : requests_doc_(json::Load(input)) {
    }
    
    void FillCatalogue(transport::TransportCatalogue& catalogue) const;

    void FillRenderer(MapRenderer& renderer) const;
    
    void PrintRequestsResults(const RequestHandler& handler, std::ostream& out) const;
    
    const json::Document& GetDocument() const;

private:
    void FillCatalogueWithStops(const json::Array& base_requests, 
                                      transport::TransportCatalogue& catalogue) const;
    void FillCatalogueWithDistances(const json::Array& base_requests, 
                                          transport::TransportCatalogue& catalogue) const;
    void FillCatalogueWithRoutes(const json::Array& base_requests, 
                                       transport::TransportCatalogue& catalogue) const;
        
    svg::Color ReadColorFromJson(json::Node color) const;
    std::vector<svg::Color> ReadArrayColorFromJson(std::vector<json::Node> colors) const;
    
    json::Dict GetRouteRequestResult(std::string_view bus_name, int request_id, 
                                     const RequestHandler& handler) const;
    json::Dict GetStopRequestResult(std::string_view stop_name, int request_id, 
                                    const RequestHandler& handler) const;
    json::Dict GetMapRequestResult(int request_id, const RequestHandler& handler) const;    
    
    json::Document requests_doc_;
};
