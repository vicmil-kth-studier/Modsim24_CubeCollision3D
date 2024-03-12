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
        DisableLogging
        // Update position
        glm::dvec3 d_position = linear_velocity.speed_m_per_s * time_step_s;
        orientation.center_of_mass = orientation.center_of_mass + d_position;

        // Update rotation
        glm::dvec3 d_rotation_vec = rotational_velocity.rotation * time_step_s;
        Debug("d_rotation_vec: " << glm::to_string(d_rotation_vec));
        Rotation d_rotation = Rotation::from_scaled_axis(d_rotation_vec);
        Debug("d_quaternion: " << glm::to_string(d_rotation.quaternion));
        Debug("old quaternion: " << glm::to_string(orientation.rotational_orientation.quaternion));
        orientation.rotational_orientation = orientation.rotational_orientation.rotate(d_rotation);
        Debug("new quaternion: " << glm::to_string(orientation.rotational_orientation.quaternion));
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


LinearVelocity get_change_in_linear_velocity(Impulse& impulse, ObjectOrientation& orientation, const ObjectShapeProperty& shape_property) {
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


RotationVelocity get_change_in_rotational_velocity(ContactPointInfo contact, Impulse& impulse, ObjectOrientation& orientation, const ObjectShapeProperty& shape_property) {
    DisableLogging
    START_TRACE_FUNCTION();
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
    rel_pos = contact.contact_position - orientation.center_of_mass;

    // Get the angular momentum stored in the impulse
    glm::dvec3 d_momentum = glm::cross(rel_pos, impulse.impulse_newton_s);

    Debug("d_momentum" << glm::to_string(d_momentum));

    Debug("get the rotated inertia tensor");
    Debug("inverse inertia tensor" << glm::to_string(shape_property.inertia_tensor._matrix_inverse));

    // Get the rotated inertia tensor
    InertiaTensor rotated_inertia_tensor = shape_property.inertia_tensor.rotate(orientation.rotational_orientation);

    Debug("inverse rotated inertia tensor" << glm::to_string(rotated_inertia_tensor._matrix_inverse));

    Debug("get the change in angular velocity");
    // Get the change in angular velocity
    glm::dvec3 d_velocity = rotated_inertia_tensor._matrix_inverse * d_momentum;

    Debug("d_velocity " << glm::to_string(d_velocity));

    END_TRACE_FUNCTION();
    return RotationVelocity::from_vec3(d_velocity);
}


void apply_impulse(Impulse impulse, ContactPointInfo contact, ObjectTrajectory& trajectory, const ObjectShapeProperty& shape_property) {
    Debug("Get change in velocities");
    // Get change in velocities
    LinearVelocity d_linear_velocity = get_change_in_linear_velocity(impulse, trajectory.orientation, shape_property);
    RotationVelocity d_rotational_velocity = get_change_in_rotational_velocity(contact, impulse, trajectory.orientation, shape_property);

    Debug("Add to old velocities");
    // Add change in velocities to old velocity
    trajectory.linear_velocity = trajectory.linear_velocity.add(d_linear_velocity);
    trajectory.rotational_velocity = trajectory.rotational_velocity.add(d_rotational_velocity);
}


double get_linear_kinetic_energy_of_object(double mass_kg, LinearVelocity velocity) {
    return mass_kg * glm::length2(velocity.speed_m_per_s) / 2.0;
}


double get_rotational_kinetic_energy_of_object(ObjectShapeProperty shape_property, RotationVelocity velocity) {
    return glm::dot((shape_property.inertia_tensor._matrix * velocity.rotation), velocity.rotation) / 2.0;
}


double get_kinetic_energy_of_object(const ObjectShapeProperty& shape_property, const ObjectTrajectory& trajectory) {
    double lin_E = get_linear_kinetic_energy_of_object(1.0 / shape_property.inverse_mass_kg, trajectory.linear_velocity);
    double rot_E = get_rotational_kinetic_energy_of_object(shape_property, trajectory.rotational_velocity);
    DebugExpr(lin_E);
    DebugExpr(rot_E);
    return lin_E + rot_E;
}


double get_height_potential_energy_of_object(double mass_kg, double height_m, double gravitational_contant_m_s2) {
    return mass_kg * gravitational_contant_m_s2 * height_m;
}


void apply_acceleration(glm::dvec3 acceleration_m_per_s2, double time_s, ObjectTrajectory& trajectory) {
    trajectory.linear_velocity.speed_m_per_s = trajectory.linear_velocity.speed_m_per_s + (acceleration_m_per_s2 * time_s);
}