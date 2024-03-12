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
double handle_cube_plane_collision(Cube& cube, vicmil::Plane& plane, double restitution_constant = 0.8) {
    //1: determine if the cube is colliding with the plane
    Overlap overlap = get_plane_cube_overlap(cube, plane);
    if(overlap.overlap <= 0) {
        return 0.0; // No contact
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
    Impulse impulse;
    impulse.impulse_newton_s = glm::normalize(contact_point.contact_normal) * impulse_magnitude;
    apply_impulse(impulse, contact_point, cube.trajectory, cube_shape);

    return impulse_magnitude;
}
