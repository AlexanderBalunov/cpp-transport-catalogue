#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

class JsonReader {
public:
    JsonReader(std::istream& input)
        : requests_doc_(json::Load(input)) {
    }
    
    void FillCatalogue(transport::TransportCatalogue& catalogue) const;

    void FillRenderer(MapRenderer& renderer) const;
    
    const json::Document& GetDocument() const;

private:
    svg::Color ReadColorFromJson(json::Node color) const;
    std::vector<svg::Color> ReadArrayColorFromJson(std::vector<json::Node> colors) const;
    
    json::Document requests_doc_;
};
