#include "N2_shapes.h"


struct ContactPointInfo {
    glm::dvec3 contact_normal;
    glm::dvec3 contact_position;
};


struct ObjectTrajectory {
    ObjectOrientation orientation = ObjectOrientation::centered_at_0();
    LinearVelocity linear_velocity = LinearVelocity::from_vec3(glm::dvec3(0, 0, 0));
    RotationVelocity rotational_velocity = RotationVelocity::from_vec3(glm::dvec3(0, 0, 0));
    static ObjectTrajectory zero() {
        ObjectTrajectory new_trejectory = ObjectTrajectory();
        return new_trejectory;
    }
    ObjectTrajectory offset_center_of_mass(glm::dvec3 pos_offset) const {
        ObjectTrajectory new_trejectory = *this;
        new_trejectory.orientation.center_of_mass = orientation.center_of_mass + pos_offset;
        return new_trejectory;
    }
    glm::dvec3 get_point_velocity_m_per_s(glm::dvec3 position) const {
        // Add the velocity caused by rotation at that point
        glm::dvec3 relative_position = position - orientation.center_of_mass;
        glm::dvec3 velocity_caused_by_rotation = glm::cross(rotational_velocity.rotation, relative_position);

        // The total velocity is the linear velocity + the velocity caused by rotation
        glm::dvec3 point_velocity = linear_velocity.speed_m_per_s + velocity_caused_by_rotation;

        return point_velocity;
    }
    // Move according to trajectory
    void move_time_step_s(double time_step_s) {
        // Update position
        glm::dvec3 d_position = linear_velocity.speed_m_per_s * time_step_s;
        orientation.center_of_mass = orientation.center_of_mass + d_position;

        // Update rotation
        glm::dvec3 d_rotation_vec = rotational_velocity.rotation * time_step_s;

        DebugExpr(glm::to_string(d_rotation_vec));
        Rotation d_rotation = Rotation::from_scaled_axis(d_rotation_vec);
        orientation.rotational_orientation = orientation.rotational_orientation.rotate(d_rotation);
        orientation.rotational_orientation.quaternion = 
            glm::normalize(orientation.rotational_orientation.quaternion);
    }
    static ObjectTrajectory diff(const ObjectTrajectory& t1, const ObjectTrajectory& t2) {
        ObjectTrajectory new_t_ = t1;
        new_t_.linear_velocity.speed_m_per_s -= t2.linear_velocity.speed_m_per_s;
        new_t_.rotational_velocity.rotation -= t2.rotational_velocity.rotation;
        new_t_.orientation.center_of_mass -= t2.orientation.center_of_mass;
        Rotation r2_inv = t2.orientation.rotational_orientation.inverse();
        new_t_.orientation.rotational_orientation = t1.orientation.rotational_orientation.rotate(r2_inv);
        return new_t_;
    }
    static ObjectTrajectory add(const ObjectTrajectory& t1, const ObjectTrajectory& t2) {
        ObjectTrajectory new_t_ = t1;
        new_t_.linear_velocity.speed_m_per_s += t2.linear_velocity.speed_m_per_s;
        new_t_.rotational_velocity.rotation += t2.rotational_velocity.rotation;
        new_t_.orientation.center_of_mass += t2.orientation.center_of_mass;
        Rotation r2_rot = t2.orientation.rotational_orientation;
        new_t_.orientation.rotational_orientation = t1.orientation.rotational_orientation.rotate(r2_rot);
        return new_t_;
    }
};

vicmil::ModelOrientation get_model_orientation_from_obj_trajectory(ObjectTrajectory trajectory) {
    vicmil::ModelOrientation orientation;
    orientation.position = trajectory.orientation.center_of_mass;
    orientation.rotation = trajectory.orientation.rotational_orientation.to_matrix();
    return orientation;
}


LinearVelocity get_change_in_linear_velocity(const Impulse& impulse, ObjectOrientation& orientation, const ObjectShapeProperty& shape_property) {
    DisableLogging
    START_TRACE_FUNCTION();
    // The physics
    // dp = F * t
    // dp = m*dv => dv = dp / m

    // F - force [N]
    // p - momentum [Ns]
    // t - time [s]
    // v - velocity [m/s]

    // dp - change in momentum
    // dv - change in velocity

    // Get the momentum stored in the impulse
    glm::dvec3 d_momentum = impulse.impulse_newton_s;

    // Get the change in linear velocity caused by the impulse
    glm::dvec3 d_velocity = d_momentum * shape_property.inverse_mass_kg;

    END_TRACE_FUNCTION();
    return LinearVelocity::from_vec3(d_velocity);
}


RotationVelocity get_change_in_rotational_velocity(const ContactImpulse& impulse, ObjectOrientation& orientation, const ObjectShapeProperty& shape_property) {
    // r = p1 - p2
    // J = F * dt
    // dL = r x J
    // L = Iw -> dw = inv(I) * dL

    // So we get:
    // dw = inv(I) * (r x J)

    // r - position relative center of mass [m]
    // J - impulse [Ns]
    // L - angular momentum [Nms]
    // F - force [N]
    // w - angular velocity [rad / s]

    // dL - change in angular momentum
    // dt - time step
    // dw - change in angular velocity

    // Get the relative position of the impulse
    glm::dvec3 rel_pos;
    rel_pos = impulse.position - orientation.center_of_mass;

    // Get the angular momentum stored in the impulse
    glm::dvec3 d_momentum = glm::cross(rel_pos, impulse.impulse.impulse_newton_s);

    // TODO: Get the rotated inertia tensor
    InertiaTensor rotated_inertia_tensor = shape_property.inertia_tensor;

    // Get the change in angular velocity
    glm::dvec3 d_velocity = rotated_inertia_tensor._matrix_inverse * d_momentum;

    return RotationVelocity::from_vec3(d_velocity);
}
TestWrapper(TEST3_get_change_in_rotational_velocity,
    /** Ensure that the initial rotation of a cube does not affect the rotational velocity
    */
    void test() {
        ContactImpulse impulse;
        impulse.impulse.impulse_newton_s = glm::dvec3(0, 0, -1);
        impulse.position = glm::dvec3(10, 5, 2);

        ObjectShapeProperty shape = ObjectShapeProperty::from_cube(1, 1);
        
        ObjectOrientation orientation = ObjectOrientation();
        orientation.rotational_orientation = Rotation();
        orientation.center_of_mass = glm::dvec3(1, 2, 3);
        RotationVelocity vel1 = get_change_in_rotational_velocity(impulse, orientation, shape);

        orientation.rotational_orientation = Rotation::from_scaled_axis(glm::dvec3(0.2, 0.1, 0.14));
        RotationVelocity vel2 = get_change_in_rotational_velocity(impulse, orientation, shape);

        glm::dvec3 diff = vel1.rotation - vel2.rotation;
        Assert(glm::length(diff) < 0.00001);
    }
);


void apply_impulse(const ContactImpulse impulse, ObjectTrajectory& trajectory, const ObjectShapeProperty& shape_property) {
    Debug("Get change in velocities");
    // Get change in velocities
    LinearVelocity d_linear_velocity = get_change_in_linear_velocity(impulse.impulse, trajectory.orientation, shape_property);
    RotationVelocity d_rotational_velocity = get_change_in_rotational_velocity(impulse, trajectory.orientation, shape_property);

    Debug("Add to old velocities");
    // Add change in velocities to old velocity
    trajectory.linear_velocity = trajectory.linear_velocity.add(d_linear_velocity);
    trajectory.rotational_velocity = trajectory.rotational_velocity.add(d_rotational_velocity);
}

void apply_acceleration(glm::dvec3 acceleration_m_per_s2, double time_s, ObjectTrajectory& trajectory) {
    trajectory.linear_velocity.speed_m_per_s = trajectory.linear_velocity.speed_m_per_s + (acceleration_m_per_s2 * time_s);
}