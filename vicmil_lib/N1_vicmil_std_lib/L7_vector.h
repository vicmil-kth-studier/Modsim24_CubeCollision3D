#pragma once
#include "L6_basic_math.h"

namespace vicmil {

/**
 * Determine if a value exists somewhere in a vector
 * @param val The value to look for
 * @param vec The vector to look in
 * @return Returns true if value is somewhere in vector, otherwise returns false
*/
template<class T>
bool in_vector(T val, std::vector<T>& vec) {
    for(int i = 0; i < vec.size(); i++) {
        if(val == vec[i]) {
            return true;
        }
    }
    return false;
}
}