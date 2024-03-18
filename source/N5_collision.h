#include "N4_geometry_representation.h"

/** Get how fast two objects are going towards each other along some axis
 * @param obj1_vel The velocity of the first object
 * @param obj2_vel The velocity of the second object
 * @param axis The axis to check their relative velocity in
 * @return The relative velocity of obj1 and obj2 along axis
*/ 
inline double get_closing_velocity(glm::dvec3 obj1_vel, glm::dvec3 obj2_vel, glm::dvec3 axis) {
    Assert(abs(glm::length(axis) - 1) < 0.0001); // Make sure the axis is normalized!

    // The closing velocity is the relative speed of the objects along contact normal at contact point
    double closing_vel_m_s = glm::dot(obj1_vel, axis) - glm::dot(obj2_vel, axis);

    DebugExpr(closing_vel_m_s);
    return closing_vel_m_s;
}

/** Determine the impulse magnitude to resolve the collision of two objects */
class CollisionImpulseResolver {
public:
    ContactPointInfo contact_point; // Make sure contact normal is normalized! e.g. has length 1
    ObjectTrajectory obj1_trajectory;
    ObjectTrajectory obj2_trajectory;
    ObjectShapeProperty obj1_shape_property;
    ObjectShapeProperty obj2_shape_property;

    // 1 means no loss of energy(perfect bounce)
    // 0 means maximum energy loss(They stick together)
    double restitution_constant = 1.0;

    inline double get_obj_closing_velocity() const {
        glm::dvec3 obj1_vel = obj1_trajectory.get_point_velocity_m_per_s(contact_point.contact_position);
        glm::dvec3 obj2_vel = obj2_trajectory.get_point_velocity_m_per_s(contact_point.contact_position);
        double closing_vel = get_closing_velocity(obj1_vel, obj2_vel, contact_point.contact_normal);
        return closing_vel;
    }

    // This is the impulse that should be applied on obj1(and negativly on obj2)
    double get_impulse_magnitude() const {
        Assert(abs(glm::length(contact_point.contact_normal) - 1) < 0.0001); // Make sure the normal is normalized!
        START_TRACE_FUNCTION();

        double prev_closing_vel_m_s = get_obj_closing_velocity(); // How fast the objects are approaching along contact normal(at contact point)
        DebugExpr(prev_closing_vel_m_s);

        double target_closing_vel = - restitution_constant * prev_closing_vel_m_s;
        DebugExpr(target_closing_vel);

        glm::dvec3 r1 = obj1_trajectory.orientation.center_of_mass - contact_point.contact_position;
        glm::dvec3 r2 = obj2_trajectory.orientation.center_of_mass - contact_point.contact_position;
        glm::dvec3 n = contact_point.contact_normal;
        glm::dmat3 I1_inv = obj1_shape_property.inertia_tensor._matrix_inverse;
        glm::dmat3 I2_inv = obj2_shape_property.inertia_tensor._matrix_inverse;

        // We can calculate the impulse magnitude such as
        // See https://en.wikipedia.org/wiki/Collision_response
        double divider = obj1_shape_property.inverse_mass_kg + obj2_shape_property.inverse_mass_kg;
        divider += glm::dot(I1_inv * glm::cross(glm::cross(r1, n), r1) + I2_inv * glm::cross(glm::cross(r2, n), r2), n);

        double impulse_magnitude = target_closing_vel / divider;
        return impulse_magnitude;
    }
};

/**
 * Fully resolve cube plane collision
 *  The collision is resolved by finding the contact point, contact normal, and then applying the correct impulse there
 * @return the impulse magnitude, it will be 0 if there was no collision
*/
ContactImpulse handle_cube_plane_collision(Cube& cube, vicmil::Plane& plane, double restitution_constant = 0.8) {
    //1: determine if the cube is colliding with the plane
    Overlap overlap = get_plane_cube_overlap(cube, plane);
    if(overlap.overlap <= 0) {
        return ContactImpulse::zero(); // No contact
    }

    //2: separate the cube and the plane(since due to the time steps they might intersect)
    cube.trajectory.orientation.center_of_mass += glm::normalize(overlap.axis) * overlap.overlap;

    //3: calculate the impact position and direction
    ContactPointInfo contact_point = find_cube_plane_contact_point(cube, plane);
    contact_point.contact_normal = glm::normalize(contact_point.contact_normal);

    //4: apply the correct impulse magnitude at the contact
    ObjectShapeProperty cube_shape = cube.get_shape_property();
    CollisionImpulseResolver impulse_resolver;
    impulse_resolver.obj1_trajectory = cube.trajectory;
    impulse_resolver.obj1_shape_property = cube_shape;
    impulse_resolver.obj2_shape_property = ObjectShapeProperty::from_immovable_object();
    impulse_resolver.obj2_trajectory = ObjectTrajectory::zero();
    impulse_resolver.contact_point = contact_point;
    impulse_resolver.restitution_constant = restitution_constant;
    double impulse_magnitude = impulse_resolver.get_impulse_magnitude();
    DebugExpr(impulse_magnitude);
    ContactImpulse impulse;
    impulse.impulse.impulse_newton_s = glm::normalize(contact_point.contact_normal) * impulse_magnitude;
    impulse.position = contact_point.contact_position;
    apply_impulse(impulse, cube.trajectory, cube_shape);

    return impulse;
}

/**
 * Get the two lowest points if all points was projected to axis
*/
void get_two_lowest_points_along_axis(std::vector<glm::dvec3> points, glm::dvec3 axis, int* index1, int* index2) {
    *index1 = vicmil::get_lowest_point_along_axis(points, axis);
    vicmil::vec_remove(points, *index1);
    *index2 = vicmil::get_lowest_point_along_axis(points, axis);
    if(*index1 <= *index2) {
        *index2 += 1;
    }
}

struct IntersectionResolution {
    glm::dvec3 new_obj1_pos;
    glm::dvec3 new_obj2_pos;
    glm::dvec3 collision_position;
    glm::dvec3 collision_axis; // The collision axis will signal the direction to apply force to obj1
    bool is_collision = true;
};

// If the axis is not aligned along vector, flip it
glm::dvec3 align_axis_along_vector(glm::dvec3 axis, glm::dvec3 vec) {
    vicmil::Line line;
    line.point = glm::dvec3(0, 0, 0);
    line.vector = axis;

    double p = project_point_to_line(vec, line);
    if(p < 0) {
        return -axis;
    }
    return axis;
}
void handle_cube_separation_along_axis(
    const Cube& cube1, 
    const Cube& cube2, 
    double overlap, 
    glm::dvec3 move_cube1_axis, 
    glm::dvec3* new_cube1_pos, 
    glm::dvec3* new_cube2_pos) {
    glm::dvec3 axis = glm::normalize(move_cube1_axis);
    double tot_inv_mass = 1.0/cube1.mass_kg + 1.0/cube2.mass_kg;
    *new_cube1_pos = cube1.trajectory.orientation.center_of_mass + (overlap+0.01) * axis * (1.0/cube1.mass_kg) / tot_inv_mass;
    *new_cube2_pos = cube2.trajectory.orientation.center_of_mass - (overlap+0.01) * axis * (1.0/cube2.mass_kg) / tot_inv_mass;
    return;
}
IntersectionResolution handle_face1_collision(const Cube& cube1, const Cube& cube2, const Overlap& overlap_) {
    START_TRACE_FUNCTION();
    IntersectionResolution intersection_resolution;

    // Align axis in direction that cube1 should move
    glm::dvec3 axis = align_axis_along_vector(overlap_.axis, cube1.trajectory.orientation.center_of_mass - cube2.trajectory.orientation.center_of_mass);
    intersection_resolution.collision_axis = axis;

    // Determine new cube positions to separate cubes
    handle_cube_separation_along_axis(cube1, cube2, overlap_.overlap, axis, &intersection_resolution.new_obj1_pos, &intersection_resolution.new_obj2_pos);

    // Determine where the objects are colliding
    Cube cube_copy = cube2;
    cube_copy.trajectory.orientation.center_of_mass = intersection_resolution.new_obj2_pos;
    std::vector<glm::dvec3> corners = cube_copy.get_corner_positions();
    int lowest_point = vicmil::get_lowest_point_along_axis(corners, -axis);
    intersection_resolution.collision_position = corners[lowest_point];

    return intersection_resolution;
}
IntersectionResolution handle_face2_collision(const Cube& cube1, const Cube& cube2, const Overlap& overlap_) {
    START_TRACE_FUNCTION();
    IntersectionResolution intersection_resolution;

    // Align axis in direction that cube1 should move
    glm::dvec3 axis = align_axis_along_vector(overlap_.axis, cube1.trajectory.orientation.center_of_mass - cube2.trajectory.orientation.center_of_mass);
    intersection_resolution.collision_axis = axis;

    // Determine new cube positions to separate cubes
    handle_cube_separation_along_axis(cube1, cube2, overlap_.overlap, axis, &intersection_resolution.new_obj1_pos, &intersection_resolution.new_obj2_pos);

    // Determine where the objects are colliding
    Cube cube_copy = cube1;
    cube_copy.trajectory.orientation.center_of_mass = intersection_resolution.new_obj1_pos;
    std::vector<glm::dvec3> corners = cube_copy.get_corner_positions();
    int lowest_point = vicmil::get_lowest_point_along_axis(corners, axis);
    intersection_resolution.collision_position = corners[lowest_point];

    return intersection_resolution;
}   
IntersectionResolution handle_edge_collision(const Cube& cube1, const Cube& cube2, const Overlap& overlap_) {
    START_TRACE_FUNCTION();
    IntersectionResolution intersection_resolution;

    // Align axis in direction that cube1 should move
    glm::dvec3 axis = align_axis_along_vector(overlap_.axis, cube1.trajectory.orientation.center_of_mass - cube2.trajectory.orientation.center_of_mass);
    intersection_resolution.collision_axis = axis;

    // Determine new cube positions to separate cubes
    handle_cube_separation_along_axis(cube1, cube2, overlap_.overlap, axis, &intersection_resolution.new_obj1_pos, &intersection_resolution.new_obj2_pos);

    // Determine where the objects are colliding
    // In the edge case, pick the points closest and then calculate where they intersect
    Cube cube1_copy = cube1;
    cube1_copy.trajectory.orientation.center_of_mass = intersection_resolution.new_obj1_pos;
    std::vector<glm::dvec3> cube1_corners = cube1_copy.get_corner_positions();
    int cube1_corner1;
    int cube1_corner2;
    get_two_lowest_points_along_axis(cube1_corners, axis, &cube1_corner1, &cube1_corner2);

    Cube cube2_copy = cube2;
    cube2_copy.trajectory.orientation.center_of_mass = intersection_resolution.new_obj2_pos;
    std::vector<glm::dvec3> cube2_corners = cube2_copy.get_corner_positions();
    int cube2_corner1;
    int cube2_corner2;
    get_two_lowest_points_along_axis(cube2_corners, -axis, &cube2_corner1, &cube2_corner2);

    // Get where on the edge they are colliding
    vicmil::Line line;
    line.point = cube2_corners[cube2_corner1];
    line.vector = cube2_corners[cube2_corner1] - cube2_corners[cube2_corner2];
    vicmil::get_closest_point_on_line_segment_to_line(cube1_corners[cube1_corner1], cube1_corners[cube1_corner2], line);

    return intersection_resolution;
}
IntersectionResolution get_cube_cube_intersection_resolution(const Cube& cube1, const Cube& cube2) {
    //1: determine if the cube1 is colliding with cube2
    Overlap overlap_faces1 = get_cube_cube_overlap_along_faces(cube1, cube2);
    Overlap overlap_faces2 = get_cube_cube_overlap_along_faces(cube2, cube1);
    Overlap overlap_edge_pairs = get_cube_cube_overlap_along_edge_pairs(cube1, cube2);

    DebugExpr(overlap_faces1.overlap);
    DebugExpr(overlap_faces2.overlap);
    DebugExpr(overlap_edge_pairs.overlap);

    // The there is no overlap along one of the axis, then there is no collision!
    if(overlap_faces1.overlap <= 0 || overlap_faces2.overlap <= 0 || overlap_edge_pairs.overlap <= 0) {
        IntersectionResolution intersection_resolution;
        intersection_resolution.is_collision = false;
        return intersection_resolution;
    }
    if(overlap_faces1.overlap <= overlap_faces2.overlap && overlap_faces1.overlap <= overlap_edge_pairs.overlap) {
        return handle_face1_collision(cube1, cube2, overlap_faces1);
    }
    if(overlap_faces2.overlap <= overlap_faces1.overlap && overlap_faces2.overlap <= overlap_edge_pairs.overlap) {
        return handle_face2_collision(cube1, cube2, overlap_faces2);
    }
    if(overlap_edge_pairs.overlap <= overlap_faces1.overlap && overlap_edge_pairs.overlap <= overlap_faces2.overlap) {
        return handle_edge_collision(cube1, cube2, overlap_edge_pairs);
    }

    ThrowError("Should be unreachable!");
}

/**
 * Fully resolve cube-cube collision
 * The collision is resolved by finding the contact point, contact normal, and then applying the correct impulse there
 * @return the impulse magnitude, it will be 0 if there was no collision
*/
ContactImpulse handle_cube_cube_collision(Cube& cube1, Cube& cube2, double restitution_constant = 0.8) {
    IntersectionResolution intersection_resolution = get_cube_cube_intersection_resolution(cube1, cube2);
    if(!intersection_resolution.is_collision) {
        return ContactImpulse::zero(); // No intersection
    }
    cube1.trajectory.orientation.center_of_mass = intersection_resolution.new_obj1_pos;
    cube2.trajectory.orientation.center_of_mass = intersection_resolution.new_obj2_pos;

    //4: apply the correct impulse magnitude at the contact
    ContactPointInfo contact;
    contact.contact_normal = intersection_resolution.collision_axis;
    contact.contact_position = intersection_resolution.collision_position;

    ObjectShapeProperty cube1_shape = cube1.get_shape_property();
    ObjectShapeProperty cube2_shape = cube2.get_shape_property();
    CollisionImpulseResolver impulse_resolver;
    impulse_resolver.obj1_trajectory = cube1.trajectory;
    impulse_resolver.obj1_shape_property = cube1_shape;
    impulse_resolver.obj2_trajectory = cube2.trajectory;
    impulse_resolver.obj2_shape_property = cube2_shape;
    impulse_resolver.contact_point = contact;
    impulse_resolver.restitution_constant = restitution_constant;
    double impulse_magnitude = impulse_resolver.get_impulse_magnitude();
    if(impulse_magnitude < 0) {
        return ContactImpulse::zero(); // Negative collision
    }

    ContactImpulse impulse;
    impulse.position = intersection_resolution.collision_position;
    impulse.impulse.impulse_newton_s = glm::normalize(contact.contact_normal) * impulse_magnitude;
    apply_impulse(impulse, cube1.trajectory, cube1_shape);
    impulse.impulse.impulse_newton_s = -impulse.impulse.impulse_newton_s;
    apply_impulse(impulse, cube2.trajectory, cube2_shape);

    return impulse;
}