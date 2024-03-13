#include "N3_forces.h"

struct Sphere {
    double radious = 1.0;
    double mass_kg = 1.0;
    ObjectTrajectory trajectory;
};


struct Cube {
    double side_length_m = 1.0;
    double mass_kg = 1.0; // The weight
    ObjectTrajectory trajectory;

    ObjectShapeProperty get_shape_property() {
        return ObjectShapeProperty::from_cube(side_length_m, mass_kg);
    }

    std::vector<glm::dvec3> get_corner_positions() {
        double s = side_length_m/2;
    
        std::vector<glm::dvec3> corner_positions = {
        trajectory.orientation.apply_orientation(glm::dvec3(s, s, s)),
        trajectory.orientation.apply_orientation(glm::dvec3(s, s, -s)),
        trajectory.orientation.apply_orientation(glm::dvec3(s, -s, s)),
        trajectory.orientation.apply_orientation(glm::dvec3(s, -s, -s)),
        trajectory.orientation.apply_orientation(glm::dvec3(-s, s, s)),
        trajectory.orientation.apply_orientation(glm::dvec3(-s, s, -s)),
        trajectory.orientation.apply_orientation(glm::dvec3(-s, -s, s)),
        trajectory.orientation.apply_orientation(glm::dvec3(-s, -s, -s))
        };
        return corner_positions;
    }
};


struct Overlap {
    glm::dvec3 axis;
    double overlap;
};

// This can probably be optimized. TODO
std::vector<double> project_cube_to_line(Cube& cube, vicmil::Line& line) {
    // Let us first calculate the cube corner positions
    std::vector<glm::dvec3> corners = cube.get_corner_positions();
    
    // Let us project these corners onto line
    std::vector<double> projected_corners = {
        vicmil::project_point_to_line(corners[0], line),
        vicmil::project_point_to_line(corners[1], line),
        vicmil::project_point_to_line(corners[2], line),
        vicmil::project_point_to_line(corners[3], line),
        vicmil::project_point_to_line(corners[4], line),
        vicmil::project_point_to_line(corners[5], line),
        vicmil::project_point_to_line(corners[6], line),
        vicmil::project_point_to_line(corners[7], line),
    };
    
    Debug("projected_corner_pos: " + vicmil::vec_to_string(corners));
    Debug("projected corners" << vicmil::vec_to_string(projected_corners));

    return projected_corners;
}


/** Get the overlap between plane and cube
 * TODO: Now only handles ground plane
 * @return The overlap between the cube and plane, the axis is the direction the cube should move in to resolve the overlap
*/
Overlap get_plane_cube_overlap(Cube& cube, vicmil::Plane& plane) {
    Overlap cube_overlap;
    vicmil::Line collision_line;
    collision_line.point = plane.point;
    collision_line.vector = plane.normal;
    
    std::vector<double> cube_projected = project_cube_to_line(cube, collision_line);
    double min_cube = vicmil::get_min_in_vector(cube_projected);

    cube_overlap.axis = plane.normal;
    cube_overlap.overlap = -min_cube; // Everything below plane is overlap

    return cube_overlap;
}

ContactPointInfo find_cube_plane_contact_point(Cube cube, vicmil::Plane plane) {
    std::vector<glm::dvec3> corners = cube.get_corner_positions();
    int lowest_corner = vicmil::get_lowest_point_along_axis(corners, plane.normal);

    ContactPointInfo contact_point;
    contact_point.contact_normal = plane.normal; // Straigth up from the plane
    contact_point.contact_position = corners[lowest_corner]; // The corner colliding with plane
    return contact_point;
}

double get_cube_cube_overlap_along_axis(Cube cube1, Cube cube2, glm::dvec3 axis) {
    vicmil::Line line;
    line.point = glm::dvec3(0, 0, 0);
    line.vector = axis;
    std::vector<double> cube1_proj = project_cube_to_line(cube1, line);
    std::vector<double> cube2_proj = project_cube_to_line(cube2, line);
    double cube1_min = vicmil::get_min_in_vector(cube1_proj);
    double cube1_max = vicmil::get_max_in_vector(cube1_proj);
    double cube2_min = vicmil::get_min_in_vector(cube2_proj);
    double cube2_max = vicmil::get_max_in_vector(cube2_proj);
    double overlap = vicmil::get_overlap(cube1_min, cube1_max, cube2_min, cube2_max);
    return overlap;
}

Overlap get_cube_cube_overlap_along_faces(Cube cube_with_faces, Cube other_cube) {
    glm::dmat3x3 cube_rotation_matrix = cube_with_faces.trajectory.orientation.rotational_orientation.to_matrix3x3();
    std::vector<glm::dvec3> cube_face_axis = {
        glm::dvec3(1, 0, 0) * cube_rotation_matrix,
        glm::dvec3(0, 1, 0) * cube_rotation_matrix,
        glm::dvec3(0, 0, 1) * cube_rotation_matrix
    };
    
    glm::dvec3 min_overlap_axis = glm::dvec3(0, 1, 0);
    double min_overlap = 1000000000; // Some large number
    for(int i = 0; i < 3; i++) {
        double overlap = get_cube_cube_overlap_along_axis(cube_with_faces, other_cube, cube_face_axis[i]);
        if(overlap < min_overlap) {
            min_overlap = overlap;
            min_overlap_axis = cube_face_axis[i];
        }
    }
    Overlap overlap;
    overlap.axis = min_overlap_axis;
    overlap.overlap = min_overlap;
    return overlap;
}

Overlap get_cube_cube_overlap_along_edge_pairs(Cube cube1, Cube cube2) {
    glm::dmat3x3 cube1_rotation_matrix = cube1.trajectory.orientation.rotational_orientation.to_matrix3x3();
    glm::dmat3x3 cube2_rotation_matrix = cube2.trajectory.orientation.rotational_orientation.to_matrix3x3();
    std::vector<glm::dvec3> cube1_face_axis = {
        glm::dvec3(1, 0, 0) * cube1_rotation_matrix,
        glm::dvec3(0, 1, 0) * cube1_rotation_matrix,
        glm::dvec3(0, 0, 1) * cube1_rotation_matrix
    };
    std::vector<glm::dvec3> cube2_face_axis = {
        glm::dvec3(1, 0, 0) * cube2_rotation_matrix,
        glm::dvec3(0, 1, 0) * cube2_rotation_matrix,
        glm::dvec3(0, 0, 1) * cube2_rotation_matrix
    };
    std::vector<glm::dvec3> edge_pair_axis = {
        glm::cross(cube1_face_axis[0], cube2_face_axis[0]),
        glm::cross(cube1_face_axis[0], cube2_face_axis[1]),
        glm::cross(cube1_face_axis[0], cube2_face_axis[2]),
        glm::cross(cube1_face_axis[1], cube2_face_axis[0]),
        glm::cross(cube1_face_axis[1], cube2_face_axis[1]),
        glm::cross(cube1_face_axis[1], cube2_face_axis[2]),
        glm::cross(cube1_face_axis[2], cube2_face_axis[0]),
        glm::cross(cube1_face_axis[2], cube2_face_axis[1]),
        glm::cross(cube1_face_axis[2], cube2_face_axis[2]),
    };
    
    glm::dvec3 min_overlap_axis = glm::dvec3(0, 1, 0);
    double min_overlap = 1000000000; // Some large number
    for(int i = 0; i < 9; i++) {
        glm::dvec3 axis = edge_pair_axis[i];
        if(glm::length2(axis) > 0.00001) { // Only pick valid axis
            axis = glm::normalize(axis);
            double overlap = get_cube_cube_overlap_along_axis(cube1, cube2, axis);
            if(overlap < min_overlap) {
                min_overlap = overlap;
                min_overlap_axis = axis;
            }
        }
    }
    Overlap overlap;
    overlap.axis = min_overlap_axis;
    overlap.overlap = min_overlap;
    return overlap;
}