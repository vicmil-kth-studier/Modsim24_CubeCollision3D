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

/**
 * Fully resolve cube-cube collision
 * The collision is resolved by finding the contact point, contact normal, and then applying the correct impulse there
 * @return the impulse magnitude, it will be 0 if there was no collision
*/
double handle_cube_cube_collision(Cube& cube1, Cube& cube2, double restitution_constant = 0.8) {
    //1: determine if the cube1 is colliding with cube2
    Overlap overlap_faces1 = get_cube_cube_overlap_along_faces(cube1, cube2);
    Overlap overlap_faces2 = get_cube_cube_overlap_along_faces(cube2, cube1);
    Overlap overlap_edge_pairs = get_cube_cube_overlap_along_edge_pairs(cube1, cube2);

    Overlap min_overlap = overlap_faces1;
    int overlap_case = 1;

    if(min_overlap.overlap > overlap_faces2.overlap) {
        min_overlap = overlap_faces2;
        overlap_case = 2;
    }

    if(min_overlap.overlap > overlap_edge_pairs.overlap) {
        min_overlap = overlap_edge_pairs;
        overlap_case = 3;
    }

    if(min_overlap.overlap < 0) {
        return 0; // No overlap
    } 

    //2: separate the cube1 and cube2(since due to the time steps they might intersect)
    //     do this with the shortest distance
    vicmil::Line line;
    line.point = glm::dvec3(0, 0, 0);
    line.vector = min_overlap.axis;
    double p1 = vicmil::project_point_to_line(cube1.trajectory.orientation.center_of_mass, line);
    double p2 = vicmil::project_point_to_line(cube2.trajectory.orientation.center_of_mass, line);
    if(p1 > p2) {
        min_overlap.axis = -min_overlap.axis;
    }

    // Separate the objects depending on their mass
    double tot_inv_mass = 1.0/cube1.mass_kg + 1.0/cube2.mass_kg;
    cube1.trajectory.orientation.center_of_mass += min_overlap.axis * (1.0/cube1.mass_kg) / tot_inv_mass;
    cube2.trajectory.orientation.center_of_mass -= min_overlap.axis * (1.0/cube2.mass_kg) / tot_inv_mass;

    ContactPointInfo contact;
    contact.contact_normal = glm::normalize(min_overlap.axis);

    //3: calculate the impact position
    if(overlap_case == 1) {
        // In the face1 case, pick the point closest to the middle in cube2
        std::vector<glm::dvec3> corners = cube2.get_corner_positions();
        int lowest_point = vicmil::get_lowest_point_along_axis(corners, -min_overlap.axis);
        contact.contact_position = corners[lowest_point];
    }

    else if(overlap_case == 2) {
        // In the face2 case, pick the point closest to the middle in cube1
        std::vector<glm::dvec3> corners = cube1.get_corner_positions();
        int lowest_point = vicmil::get_lowest_point_along_axis(corners, min_overlap.axis);
        contact.contact_position = corners[lowest_point];
    }
    else {
        // In the edge case, pick the points closest and then calculate where they intersect
        std::vector<glm::dvec3> cube1_corners = cube1.get_corner_positions();
        int cube1_corner1;
        int cube1_corner2;
        get_two_lowest_points_along_axis(cube1_corners, min_overlap.axis, &cube1_corner1, &cube1_corner2);

        std::vector<glm::dvec3> cube2_corners = cube2.get_corner_positions();
        int cube2_corner1;
        int cube2_corner2;
        get_two_lowest_points_along_axis(cube2_corners, -min_overlap.axis, &cube2_corner1, &cube2_corner2);

        // Now we can determine where they intersect
        vicmil::Line edge1;
        edge1.point = cube1_corners[cube1_corner1];
        edge1.vector = cube1_corners[cube1_corner1] - cube1_corners[cube1_corner2];
        vicmil::Line edge2;
        edge2.point = cube2_corners[cube2_corner1];
        edge2.vector = cube2_corners[cube2_corner1] - cube1_corners[cube2_corner2];
        glm::dvec3 point1;
        glm::dvec3 point2;
        vicmil::get_closest_points_between_two_lines(edge1, edge2, &point1, &point2);
        contact.contact_position = (point1 + point2) / 2.0; // Get the point in between points
    }

    //4: apply the correct impulse magnitude at the contact
    ObjectShapeProperty cube1_shape = cube1.get_shape_property();
    ObjectShapeProperty cube2_shape = cube1.get_shape_property();
    CollisionImpulseResolver impulse_resolver;
    impulse_resolver.obj1_trajectory = cube1.trajectory;
    impulse_resolver.obj1_shape_property = cube1_shape;
    impulse_resolver.obj2_trajectory = cube2.trajectory;
    impulse_resolver.obj2_shape_property = cube2_shape;
    impulse_resolver.contact_point = contact;
    impulse_resolver.restitution_constant = restitution_constant;
    double impulse_magnitude = impulse_resolver.get_impulse_magnitude();
    DebugExpr(impulse_magnitude);
    ContactImpulse impulse;
    impulse.impulse.impulse_newton_s = glm::normalize(contact.contact_normal) * impulse_magnitude;
    impulse.position = contact.contact_position;
    apply_impulse(impulse, cube1.trajectory, cube1_shape);
    impulse.impulse.impulse_newton_s = -impulse.impulse.impulse_newton_s;
    apply_impulse(impulse, cube2.trajectory, cube2_shape);

    return impulse_magnitude;
}