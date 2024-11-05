#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {
        
bool Coordinates::operator==(const Coordinates& other) const {
    return lat == other.lat && lng == other.lng;
}
    
bool Coordinates::operator!=(const Coordinates& other) const {
    return !(*this == other);
}  
    
double ComputeDistance(Coordinates from, Coordinates to) {
    if (from == to) {
        return 0;
    }    
    using namespace std;
    const double dr = M_PI / 180.0;
    return acos(sin(from.lat * dr) * sin(to.lat * dr) + 
                cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
                * 6371000;
}
    
}  // namespace geo
