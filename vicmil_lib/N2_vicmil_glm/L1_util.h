#pragma once
#include "../N1_vicmil_std_lib/vicmil_std_lib.h"
#include "../../dependencies/glm/glm/glm.hpp"
#include "../../dependencies/glm/glm/gtc/matrix_transform.hpp"
#include "../../dependencies/glm/glm/gtx/transform.hpp"
#include "../../dependencies/glm/glm/ext/matrix_transform.hpp" // perspective, translate, rotate
#include "../../dependencies/glm/glm/gtc/quaternion.hpp"
#include "../../dependencies/glm/glm/gtx/quaternion.hpp"
#include "../../dependencies/glm/glm/gtx/string_cast.hpp"

namespace vicmil {
glm::dvec3 project_vector_onto_vector(glm::dvec3 vector, glm::dvec3 on_vector) {
    double len = glm::dot(vector, on_vector);
    return glm::normalize(on_vector) * len;
}
TestWrapper(TEST_project_vector_onto_vector,
    void test() {
        assert(glm::length(project_vector_onto_vector(glm::dvec3(1, 0, 0), glm::dvec3(0, 1, 0)) - glm::dvec3(0, 0, 0)) < 0.00001);
        assert(glm::length(project_vector_onto_vector(glm::dvec3(1, 0, 0), glm::dvec3(0, 0, 1)) - glm::dvec3(0, 0, 0)) < 0.00001);
        assert(glm::length(project_vector_onto_vector(glm::dvec3(1, 1, 0), glm::dvec3(1, 0, 0)) - glm::dvec3(1, 0, 0)) < 0.00001);
        assert(glm::length(project_vector_onto_vector(glm::dvec3(1, 0, 0), glm::dvec3(1, 1, 0))) < 1.0);
    }
);

glm::dvec3 project_vector_onto_plane(glm::dvec3 vector, glm::dvec3 plane_normal) {
    return vector - project_vector_onto_vector(vector, plane_normal);
}

inline std::string vec_to_string(const std::vector<glm::dvec3>& vec) {
    std::string out_str;
    out_str += "{ \n";
    for(int i = 0; i < vec.size(); i++) {
        if(i != 0) {
            out_str += ", \n";
        }
        out_str += glm::to_string(vec[i]);
    }
    out_str += " }";
    return out_str;
}

/**
 * A line, represented by a point on the line and the line vector
*/
struct Line {
    glm::dvec3 point;
    glm::dvec3 vector;
};

/**
 * A plane, represented by a point on the plane and the plane normal
*/
struct Plane {
    // The plane normal and a point on the plane is sufficient to describe it
    glm::dvec3 point = glm::dvec3(0, 0, 0);
    glm::dvec3 normal = glm::dvec3(0, 1, 0);
};

/**
 * Project a point to a line, you can find the projected position by using
 *  "line.point + return_value * line.vector"
 * @param point The point to project
 * @param line The line to project the point to
*/
double project_point_to_line(glm::dvec3 point, Line& line) {
    glm::dvec3 relative_pos = point - line.point;
    float len = glm::dot(relative_pos, line.vector);
    return len; // Get a value where it is one the line if it was projected
}
}