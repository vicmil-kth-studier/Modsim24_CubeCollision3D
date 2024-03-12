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
}