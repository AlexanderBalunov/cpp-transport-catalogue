#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <iterator>
    
using namespace std::literals;

namespace transport {

namespace detail {    
    
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}
 
struct SeparationFormat {
    std::string_view coordinates;
    std::string_view distances;
};

SeparationFormat Separate(std::string_view description) {
    auto comma_between_coordinates = description.find(',');    
    auto comma_after_coordinates = description.find(',', comma_between_coordinates + 1);
    if (comma_after_coordinates == description.npos) {
        return {description, ""sv};
    }
    return {description.substr(0, comma_after_coordinates),
            description.substr(comma_after_coordinates + 1)};
}    
    
std::pair<std::string_view, int> ParseDistance(std::string_view distance_info) {
    return {distance_info.substr(distance_info.find_first_not_of(' ', distance_info.find("to"sv) + 2)), 
                              std::stoi(std::string(distance_info.substr(0, distance_info.find('m'))))};
}

std::unordered_map<std::string_view, int> ParseDistances(std::string_view distances) {
    if (distances.empty()) {
        return {};
    }
    
    std::unordered_map<std::string_view, int> distances_to_stops;    
    for (auto distance_info : Split(distances, ',')) {
        distances_to_stops.insert(ParseDistance(distance_info));
    }
    return distances_to_stops;
}   

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

}
    
void InputReader::ParseLine(std::string_view line) {
    auto command_description = detail::ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    for (const detail::CommandDescription& command_description : commands_) {
        if (command_description.command == "Stop") {
            catalogue.AddStop(command_description.id, detail::ParseCoordinates(detail::Separate(command_description.description).coordinates));
        } else if (command_description.command != "Bus") {
            throw std::invalid_argument("Unknown request in the InputReader class.");
        }
    }
    for (const detail::CommandDescription& command_description : commands_) {
        if (command_description.command == "Stop") {
            for (auto [stop_to, distance] : detail::ParseDistances(detail::Separate(command_description.description).distances)) {
                catalogue.AddDistance(command_description.id, stop_to, distance);   
            }
        }
    }    
    for (const detail::CommandDescription& command_description : commands_) {
        if (command_description.command == "Bus") {
            std::vector<std::string> stops;
            for (auto stop : detail::ParseRoute(command_description.description)) {
                stops.push_back(std::string(stop));
            }
            catalogue.AddRoute(command_description.id, stops);
        }    
    }   
}
    
}
