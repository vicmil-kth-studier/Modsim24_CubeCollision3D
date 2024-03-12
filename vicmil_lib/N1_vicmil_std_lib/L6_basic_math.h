#pragma once
#include "L5_time.h"

namespace vicmil {
const double PI  = 3.141592653589793238463;

inline bool is_power_of_two(unsigned int x) {
    return !(x == 0) && !(x & (x - 1));
}
inline bool is_power_of_two(int x) {
    return !(x == 0) && !(x & (x - 1));
}
    
unsigned int upper_power_of_two(unsigned int x)
{
    int power = 1;
    while(power < x) {
        power*=2;
    }
    return power;

}

double modulo(double val, double mod) {
    if(val > 0) {
        return val - ((int)(val / mod)) * mod;
    }
    else {
        return val - ((int)((val-0.0000001) / mod) - 1) * mod;
    }
}

double degrees_to_radians(const double deg) {
    return deg * 2.0 * PI / 360.0;
}

double radians_to_degrees(const double rad) {
    return rad * 360.0 / (PI * 2.0);
}

/** Returns the amount of overlap between two ranges: (min1, max1) and (min2, max2)
 * @param min1 The minimum value in range1
 * @param max1 The maximum value in range1
 * @param min2 The minimum value in range2
 * @param max2 The maximum value in range2
 * @return the amount of overlap between ranges (min1, max1) and (min2, max2)
*/
double get_overlap(double min1, double max1, double min2, double max2) {
    // Scenarios
    // where (smaller --> greater)
    // (min1, max1, min2, max2)    -no overlap
    // (min1, min2, max1, max2)    - overlap
    // (min1, min2, max2, max1)    - overlap

    // (min2, max2, min1, max1)    -no overlap
    // (min2, min1, max2, max1)    - overlap
    // (min2, min1, max1, max2)    - overlap

    // How much do we have to move 1 left to avoid overlap?
    double left = max1 - min2;

    // How much do we have to move 1 right to avoid overlap?
    double right = max2 - min1;


    return std::min(left, right); // Negative result means there is no overlap!
}
}