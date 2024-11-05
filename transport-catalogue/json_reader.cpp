#include "json_reader.h"

using namespace std::literals;

void JsonReader::FillCatalogue(transport::TransportCatalogue& catalogue) const {
    const auto& base_requests_array = requests_doc_.GetRoot().AsMap().at("base_requests"s).AsArray();
    for (const auto& base_request : base_requests_array) {
        const auto& base_request_map = base_request.AsMap();
        if (base_request_map.at("type"s).AsString() == "Stop"s) {
            catalogue.AddStop(base_request_map.at("name"s).AsString(),
                             {base_request_map.at("latitude"s).AsDouble(), 
                              base_request_map.at("longitude"s).AsDouble()});
        }
    }
    for (const auto& base_request : base_requests_array) {
        const auto& base_request_map = base_request.AsMap();
        if (base_request_map.at("type"s).AsString() == "Stop"s) {
            for (const auto& [stop_to, distance] : base_request_map.at("road_distances"s).AsMap()) {
                catalogue.AddDistance(base_request_map.at("name"s).AsString(), stop_to, distance.AsInt());    
            }
        }
    }
    for (const auto& base_request : base_requests_array) {        
        const auto& base_request_map = base_request.AsMap();                
        if (base_request_map.at("type"s).AsString() == "Bus"s) {
            std::vector<std::string> stops_in_route;
            for (const auto& stop : base_request_map.at("stops"s).AsArray()) {
                stops_in_route.push_back(stop.AsString());
            }        
            catalogue.AddRoute(base_request_map.at("name"s).AsString(), stops_in_route, 
                               base_request_map.at("is_roundtrip"s).AsBool());    
        }            
    }
}

void JsonReader::FillRenderer(MapRenderer& renderer) const {
    const auto& render_settings_map = requests_doc_.GetRoot().AsMap().at("render_settings"s).AsMap();
    renderer.SetSettings({render_settings_map.at("width"s).AsDouble(),
                          render_settings_map.at("height"s).AsDouble(),
                          render_settings_map.at("padding"s).AsDouble(),
                          render_settings_map.at("line_width"s).AsDouble(),
                          render_settings_map.at("stop_radius"s).AsDouble(),
                          render_settings_map.at("bus_label_font_size"s).AsInt(),
                          {render_settings_map.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                           render_settings_map.at("bus_label_offset"s).AsArray()[1].AsDouble()},
                          render_settings_map.at("stop_label_font_size"s).AsInt(),
                          {render_settings_map.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                           render_settings_map.at("stop_label_offset"s).AsArray()[1].AsDouble()},
                          ReadColorFromJson(render_settings_map.at("underlayer_color"s)),
                          render_settings_map.at("underlayer_width"s).AsDouble(),
                          ReadArrayColorFromJson(render_settings_map.at("color_palette"s).AsArray())});
}

const json::Document& JsonReader::GetDocument() const {
    return requests_doc_;
}

svg::Color JsonReader::ReadColorFromJson(json::Node color_node) const {
    svg::Color color;
    if (color_node.IsString()) {
        color = std::move(color_node.AsString());
    } else if (color_node.IsArray()) {
        const json::Array& color_numeric_format = std::move(color_node.AsArray());
        if (color_numeric_format.size() == 3) {
            color = svg::Rgb{static_cast<uint8_t>(color_numeric_format[0].AsInt()),
                             static_cast<uint8_t>(color_numeric_format[1].AsInt()),
                             static_cast<uint8_t>(color_numeric_format[2].AsInt())};
        } else if (color_numeric_format.size() == 4) {
            color = svg::Rgba{static_cast<uint8_t>(color_numeric_format[0].AsInt()),
                              static_cast<uint8_t>(color_numeric_format[1].AsInt()),
                              static_cast<uint8_t>(color_numeric_format[2].AsInt()),
                              color_numeric_format[3].AsDouble()};            
        }
    }
    return color;
}

std::vector<svg::Color> JsonReader::ReadArrayColorFromJson(std::vector<json::Node> color_nodes) const {
    std::vector<svg::Color> colors;
    for (const json::Node& color_node : color_nodes) {
        colors.emplace_back(JsonReader::ReadColorFromJson(color_node));
    }
    return colors;
}
