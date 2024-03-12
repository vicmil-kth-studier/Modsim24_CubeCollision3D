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

// TODO: make it able to handle other than ground plane
ContactPointInfo find_cube_plane_contact_point(Cube cube, vicmil::Plane plane) {
    std::vector<glm::dvec3> corners = cube.get_corner_positions();
    glm::dvec3 lowest_corner = corners[0];
    for(int i = 1; i < corners.size(); i++) {
        if(corners[i].y < lowest_corner.y) { // Grab the lowest point
            lowest_corner = corners[i];
        }
    }

    ContactPointInfo contact_point;
    contact_point.contact_normal = glm::dvec3(0, 1, 0); // Straigth up from the plane
    contact_point.contact_position = lowest_corner; // The corner colliding with plane
    return contact_point;
}