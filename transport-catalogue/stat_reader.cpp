#include "stat_reader.h"

#include <iomanip>
#include <set>

namespace transport {

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output) {
    size_t command_begin_pos = request.find_first_not_of(' ');
    auto command_name = request.substr(command_begin_pos, request.find(' ', command_begin_pos));
    
    size_t id_name_begin_pos = request.find_first_not_of(' ', request.find(' ', command_begin_pos));
    auto id_name = request.substr(id_name_begin_pos, request.find_last_not_of(' ') - id_name_begin_pos + 1);
        
    if (command_name == "Bus") { 
        const RouteInfo result = tansport_catalogue.GetRouteInfo(std::string(id_name));
        if (result.number_of_stops == 0) {
            output << "Bus " << id_name << ": not found" << std::endl;
            return;
        }
        output << "Bus " << id_name << ": " << result.number_of_stops << " stops on route, "
                                              << result.number_of_unique_stops << " unique stops, "
                                              << std::setprecision(6) << result.length << " route length" << std::endl;
    } else if (command_name == "Stop") {
        if (tansport_catalogue.GetStop(std::string(id_name)) == nullptr) {
            output << "Stop " << id_name << ": not found" << std::endl;
            return;
        }
        const auto routes = tansport_catalogue.GetRoutesThroughStop(std::string(id_name));
        if (routes.empty()) {
            output << "Stop " << id_name << ": no buses" << std::endl;
        } else {
            output << "Stop " << id_name << ": buses";
            for (const std::string_view& stop : std::set(routes.begin(), routes.end())) {
                output << " " << stop;
            }
            output << std::endl;
        }        
    } else {
        throw std::invalid_argument("Unknown request in func ParseAndPrintStat.");
    }
}
    
}
