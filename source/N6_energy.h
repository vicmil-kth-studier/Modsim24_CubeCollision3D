/* Calculate the energy of the objects, can be used to ensure that the calculations are correct
*/
#include "N5_collision.h"

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

struct ObjectEnergyInfo {
    double potential_energy;
    double rotational_kin_energy;
    double linear_kin_energy;
    std::string to_string() {
        return "tot_E: " + std::to_string(potential_energy + rotational_kin_energy + linear_kin_energy) +
            "  kin_E: " + std::to_string(rotational_kin_energy + linear_kin_energy) +
            "  pot_E: " + std::to_string(potential_energy) + 
            "  rot_E: " + std::to_string(rotational_kin_energy) +
            "  lin_E: " + std::to_string(linear_kin_energy);
    }
    static ObjectEnergyInfo diff(const ObjectEnergyInfo& info1, const ObjectEnergyInfo& info2) {
        ObjectEnergyInfo new_info = info1;
        new_info.potential_energy -= info2.potential_energy;
        new_info.rotational_kin_energy -= info2.rotational_kin_energy;
        new_info.linear_kin_energy -= info2.linear_kin_energy;
        return new_info;
    }
};

ObjectEnergyInfo get_cube_energy_information(Cube& cube, double gravitation_constant = 0.0) { // Mostly for debugging purposes
    ObjectEnergyInfo energy_info;
    double height_m = cube.trajectory.orientation.center_of_mass.y;
    energy_info.potential_energy = cube.mass_kg * gravitation_constant * height_m; // m*g*h

    ObjectShapeProperty cube_shape_property = cube.get_shape_property();
    energy_info.rotational_kin_energy = get_rotational_kinetic_energy_of_object(cube_shape_property, cube.trajectory.rotational_velocity);
    energy_info.linear_kin_energy = get_linear_kinetic_energy_of_object(cube.mass_kg, cube.trajectory.linear_velocity);
    return energy_info;
}