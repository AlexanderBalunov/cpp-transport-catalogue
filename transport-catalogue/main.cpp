#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <sstream>

int main () {
    transport::TransportCatalogue ctlg;
    MapRenderer renderer;
    
    JsonReader reader(std::cin);
    
    reader.FillCatalogue(ctlg);
    reader.FillRenderer(renderer);
    
    RequestHandler handler(ctlg, renderer);
    
    reader.PrintRequestsResults(handler, std::cout);
}
