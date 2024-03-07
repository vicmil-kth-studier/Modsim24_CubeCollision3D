#include "L5_time.h"

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