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

/**
 * If we have two lines, get the point on each line that is closest to the other line
 * 
 * NOTE: In the case if parallell lines, there are infinately many pairs points that fullfill this criteria, 
 *  we pick one of the points to be line1.point and then calculate the other
*/
void get_closest_points_between_two_lines(Line line1, Line line2, glm::dvec3* point1, glm::dvec3* point2) {
    glm::dvec3 line_segment = glm::cross(line1.vector, line2.vector);
    if(glm::length2(line_segment) < 0.00001) {
        // This means the lines are parallell, TODO fix implementation
        *point1 = line1.point;
        *point2 = line2.point;
        return;
    }

    /* Now we want to find t1, t2, t3 that fullfills
    line1.point + line1.vector * t1 + line_segment * t2 = line2.point + line2.vector * t3

    cx1 + t1*lx1 + t2*lx2 = cx2 + t3*lx3
    cy1 + t1*ly1 + t2*ly2 = cy2 + t3*ly3
    cz1 + t1*lz1 + t2*lz2 = cz2 + t3*lz3

    t1*lx1 + t2*lx2 - t3*lx3 = cx2 - cx1
    t1*ly1 + t2*ly2 - t3*ly3 = cy2 - cy1
    t1*lz1 + t2*lz2 - t3*lz3 = cz2 - cz1

    // Now we just need to solve the linear equation system
    */

    glm::dvec4 row1 = glm::dvec4(line1.vector.x, line_segment.x, -line2.vector.x, line2.point.x - line1.point.x);
    glm::dvec4 row2 = glm::dvec4(line1.vector.y, line_segment.y, -line2.vector.y, line2.point.y - line1.point.y);
    glm::dvec4 row3 = glm::dvec4(line1.vector.z, line_segment.z, -line2.vector.z, line2.point.z - line1.point.z);

    // Solve first column
    if(abs(row1[0]) < 0.001 && abs(row2[0]) > 0.001) {
        glm::dvec4 tmp = row1;
        row1 = row2;
        row2 = tmp;
    }
    else if(abs(row1[0]) < 0.001 && abs(row3[0]) > 0.001) {
        glm::dvec4 tmp = row1;
        row1 = row3;
        row3 = tmp;
    }
    row1 = row1 / row1[0];
    row2 -= row1 * row2[0];
    row3 -= row1 * row3[0];

    // Solve second column
    if(abs(row2[1]) < 0.001 && abs(row3[1]) > 0.001) {
        glm::dvec4 tmp = row2;
        row2 = row3;
        row3 = tmp;
    }
    row2 = row2 / row2[1];
    row3 -= row2 * row3[1];

    // Solve third column
    row3 = row3 / row3[2];

    // We now have
    // row1 = (1, _, _, _)
    // row2 = (0, 1, _, _)
    // row3 = (0, 0, 1, _)
    Assert(abs(row1[0] - 1) < 0.00001);
    Assert(abs(row2[0] - 0) < 0.00001);
    Assert(abs(row3[0] - 0) < 0.00001);
    Assert(abs(row2[1] - 1) < 0.00001);
    Assert(abs(row3[1] - 0) < 0.00001);
    Assert(abs(row3[2] - 1) < 0.00001);

    // Time to do the final solving
    row2 -= row3 * row2[2];
    row1 -= row3 * row1[2];

    row1 -= row2 * row1[1];

    // We should now have:
    // row1 = (1, 0, 0, _)
    // row2 = (0, 1, 0, _)
    // row3 = (0, 0, 1, _)
    Assert(abs(row1[0] - 1) < 0.00001);
    Assert(abs(row2[0] - 0) < 0.00001);
    Assert(abs(row3[0] - 0) < 0.00001);
    Assert(abs(row1[1] - 0) < 0.00001);
    Assert(abs(row2[1] - 1) < 0.00001);
    Assert(abs(row3[1] - 0) < 0.00001);
    Assert(abs(row1[2] - 0) < 0.00001);
    Assert(abs(row2[2] - 0) < 0.00001);
    Assert(abs(row3[2] - 1) < 0.00001);

    double t1 = row1[3];
    double t2 = row2[3];
    double t3 = row3[3];

    *point1 = line1.point + line1.vector * t1;
    *point2 = line2.point + line2.vector * t3;

    Assert(glm::length(*point1 + line_segment*t2 - *point2) < 0.00001)

    return;
}

/**
 * If we were to project all points to some axis, which would be the lowest point?
 * @return The index if the lowest point
*/
int get_lowest_point_along_axis(std::vector<glm::dvec3> points, glm::dvec3 axis) {
    vicmil::Line line;
    line.point = glm::dvec3(0, 0, 0);
    line.vector = axis;
    double min_point_value = project_point_to_line(points[0], line);
    int min_point_index = 0;
    
    for(int i = 1; i < points.size(); i++) {
        double point_value = project_point_to_line(points[i], line);
        if(min_point_value > point_value) {
            min_point_value = point_value;
            min_point_index = i;
        }
    }
    return min_point_index;
}
}