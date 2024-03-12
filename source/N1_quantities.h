#pragma once
#include "../vicmil_lib/N3_vicmil_opengl/vicmil_opengl.h"

class Rotation {
public:
    glm::dquat quaternion;
    inline static Rotation from_quaternion(glm::dquat quaternion_) {
        Rotation new_rotation = Rotation();
        new_rotation.quaternion = quaternion_;
        return new_rotation;
    }
    inline static Rotation from_axis_rotation(const double radians, const glm::dvec3 axis) {
        double sin_rad = std::sin(radians/2);
        double cos_rad = std::cos(radians/2);
        glm::dvec3 normalized_axis = glm::normalize(axis);

        glm::dquat new_quaternion = glm::dquat();
        new_quaternion.w = cos_rad;
        new_quaternion.x = normalized_axis[0] * sin_rad;
        new_quaternion.y = normalized_axis[1] * sin_rad;
        new_quaternion.z = normalized_axis[2] * sin_rad;
        return Rotation::from_quaternion(new_quaternion);
    }
    // The length of the axis is the radians, the direction is the axis of rotation
    inline static Rotation from_scaled_axis(const glm::dvec3 scaled_axis) {
        double radians = glm::length(scaled_axis);
        if(glm::length(scaled_axis) != 0) {
            return from_axis_rotation(radians, scaled_axis);
        }
        else {
            glm::dquat new_quaternion = glm::dquat();
            new_quaternion.w = 1.0;
            new_quaternion.x = 0;
            new_quaternion.y = 0;
            new_quaternion.z = 0;
            return Rotation::from_quaternion(new_quaternion);
        }
    }
    Rotation rotate(const Rotation& other) const {
        return Rotation::from_quaternion(this->quaternion * other.quaternion);
    }
    Rotation inverse() const {
        return Rotation::from_quaternion(glm::inverse(this->quaternion));
    }
    std::string to_axis_rotation_str() const {
        double radians = std::acos(this->quaternion.w);
        glm::dvec3 direction = glm::dvec3(this->quaternion.x, this->quaternion.y, this->quaternion.z);
        return std::to_string(vicmil::radians_to_degrees(radians)) + "deg, " + glm::to_string(glm::normalize(direction));
    }
    inline glm::dvec3 rotate_vector(const glm::dvec3& vec) const {
        // TODO: improve implementation
        glm::dvec4 temp = to_matrix()*glm::dvec4(vec.x, vec.y, vec.z, 1);
        return glm::dvec3(temp.x, temp.y, temp.z);
    }
    inline glm::dvec3 inverse_rotate_vector(const glm::dvec3& vec) const {
        return vec; // TODO
    }
    inline glm::dmat4x4 to_matrix() const {
        return glm::dmat4x4(quaternion);
    }
};
TestWrapper(TEST1_Rotation,
    void test() {
        glm::dvec3 axis = glm::normalize(glm::dvec3(1, 0, 0));
        double radians1 = 0.1;
        double radians2 = -0.1;
        Rotation rot1 = Rotation::from_axis_rotation(radians1, axis);
        Rotation rot2 = Rotation::from_scaled_axis(axis * radians2);
        Rotation rot_sum = rot1.rotate(rot2);
        glm::dvec3 vec1 = glm::dvec3(0, 1, 0);
        glm::dvec3 vec1_rot = rot_sum.rotate_vector(vec1);
        DebugExpr(glm::to_string(vec1_rot));
        assert(glm::length(vec1_rot - vec1) < 0.0001); // The rotations should cancel each other
    }
);
TestWrapper(TEST2_Rotation,
    void test() {
        glm::dvec3 axis = glm::normalize(glm::dvec3(1, 2, 3));
        double radians1 = 0.1;
        Rotation rot = Rotation::from_axis_rotation(radians1, axis);
        glm::dvec3 vec1 = glm::dvec3(0, 1, 0);
        glm::dvec3 vec1_rot = rot.rotate_vector(vec1);
        DebugExpr(glm::to_string(vec1_rot));

        // It should rotate a bit different in each axis
        assert(abs(vec1_rot.x) > 0.0001); 
        assert(abs(vec1_rot.y) > 0.0001); 
        assert(abs(vec1_rot.z) > 0.0001); 
        assert(abs(vec1_rot.x - vec1_rot.y) > 0.0001); 
        assert(abs(vec1_rot.x - vec1_rot.z) > 0.0001); 
        assert(abs(vec1_rot.y - vec1_rot.z) > 0.0001); 
    }
);
TestWrapper(TEST3_Rotation,
    void test() {
        glm::dvec3 axis = glm::dvec3(0, 1, 0);
        double radians1 = 2 * vicmil::PI / 2; // Half a rotation
        Rotation rot = Rotation::from_axis_rotation(radians1, axis);
        glm::dvec3 vec1 = glm::dvec3(0, 0, 1);
        glm::dvec3 vec1_rot = rot.rotate_vector(vec1);
        DebugExpr(glm::to_string(vec1_rot));

        // Make sure it rotated half a rotation
        assert(glm::length(vec1_rot - glm::dvec3(0, 0, -1)) < 0.0001); 
    }
);

class InertiaTensor {
    public:
    glm::dmat3x3 _matrix;
    glm::dmat3x3 _matrix_inverse;
    static InertiaTensor from_matrix(glm::dmat3x3 matrix_) {
        InertiaTensor new_tensor;
        new_tensor._matrix = matrix_;
        new_tensor._matrix_inverse = glm::inverse(matrix_);
        Debug(glm::to_string(new_tensor._matrix));
        Debug("n1 inertia tensor inv " << glm::to_string(new_tensor._matrix_inverse));
        return new_tensor;
    }
    static InertiaTensor from_cube(double side_len_m, double mass_kg) {
        glm::dmat3x3 tensor_matrix = glm::dmat3x3(1.0f);
        tensor_matrix = tensor_matrix * (mass_kg * side_len_m * side_len_m / 6);
        return InertiaTensor::from_matrix(tensor_matrix);
    }
    static InertiaTensor zero_inertia_tensor() {
        InertiaTensor new_tensor;
        // Make tensors just be zero, since the objects rotation is undefined
        new_tensor._matrix = glm::dmat3x3() * 0.0;
        new_tensor._matrix_inverse = glm::dmat3x3() * 0.0;
        return new_tensor;
    }
    InertiaTensor rotate(Rotation rotation) const {
        // TODO
        Debug("rotated inertia tensor not implemented yet!");
        return *this;
    }
    InertiaTensor move(glm::dvec3 direction) {
        Debug("not implemented yet!");
        return *this;
    }
};

class RotationVelocity {
public:
    glm::dvec3 rotation = glm::dvec3(0, 0, 0); // The direction is the rotation axis, the length is the rotation speed around that axis in rad/s
    Rotation get_change_in_rotation(double time_step_s) {
        return Rotation::from_axis_rotation(glm::length(rotation), rotation);
    }
    static inline RotationVelocity from_vec3(glm::dvec3 vec) {
        RotationVelocity vel;
        vel.rotation = vec;
        return vel;
    }
    RotationVelocity add(RotationVelocity vel) {
        return RotationVelocity::from_vec3(rotation + vel.rotation);
    }
};

class LinearVelocity {
public:
    glm::dvec3 speed_m_per_s = glm::dvec3(0, 0, 0); // The direction is the movement direction, the length is the speed in meter/sec
    glm::dvec3 get_change_in_position(double time_step_s) {
        return speed_m_per_s * time_step_s;
    }
    static inline LinearVelocity from_vec3(glm::dvec3 vec) {
        LinearVelocity vel;
        vel.speed_m_per_s = vec;
        return vel;
    }
    LinearVelocity add(LinearVelocity vel) {
        return LinearVelocity::from_vec3(speed_m_per_s + vel.speed_m_per_s);
    }
};

/**
 * An impulse is defined as a force over a time period
*/
class Impulse { 
public:
    glm::dvec3 impulse_newton_s;
    Impulse reversed() {
        Impulse new_impulse = *this;
        new_impulse.impulse_newton_s = impulse_newton_s * (-1.0);
        return new_impulse;
    }
    static Impulse from_force(glm::dvec3 force_newton, double time_s) {
        Impulse new_impulse = Impulse();
        new_impulse.impulse_newton_s = force_newton * time_s; // An impulse is just force times time
        return new_impulse;
    }
};