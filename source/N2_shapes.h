#include "N1_quantities.h"

struct ObjectOrientation {
    Rotation rotational_orientation; // Stores the rotation
    glm::dvec3 center_of_mass; // Where the center of mass of the object is
    static ObjectOrientation centered_at_0() {
        ObjectOrientation new_orientation;
        new_orientation.rotational_orientation = Rotation::from_axis_rotation(0, glm::dvec3(0, 1, 0));
        new_orientation.center_of_mass = glm::dvec3(0, 0, 0);
        return new_orientation;
    }
    ObjectOrientation inverse() {
        ObjectOrientation new_obj_orientation;
        new_obj_orientation.rotational_orientation.quaternion = glm::inverse(rotational_orientation.quaternion);
        new_obj_orientation.center_of_mass = -center_of_mass;
        return new_obj_orientation;
    }
    ObjectOrientation add(const ObjectOrientation& other) const {
        ObjectOrientation new_obj_orientation;
        new_obj_orientation.rotational_orientation.quaternion = rotational_orientation.quaternion + other.rotational_orientation.quaternion;
        new_obj_orientation.center_of_mass = center_of_mass + (glm::dvec3)(other.center_of_mass * glm::dmat3x3(rotational_orientation.quaternion));
        return new_obj_orientation;
    }
    // Suppose the object was at (0,0,0) and had no rotation, and we had a point on the object
    //  if we then we applied the orientation. Where would the new point be? (if it were in the same place relative the object)
    glm::dvec3 apply_orientation(glm::dvec3 point) {
        return center_of_mass + rotational_orientation.rotate_vector(point);
    }
};

struct ObjectShapeProperty {
    InertiaTensor inertia_tensor; // Describes how difficult it is to rotate in different directions
    double inverse_mass_kg; // It is better to store inverse mass, this means we can have objects with infinite mass

    inline static ObjectShapeProperty from_cube(double side_len_m, double mass_kg) {
        ObjectShapeProperty shape_property;
        shape_property.inverse_mass_kg = 1.0 / mass_kg;
        shape_property.inertia_tensor = InertiaTensor::from_cube(side_len_m, mass_kg);
        return shape_property;
    }
    inline static ObjectShapeProperty from_immovable_object() {
        ObjectShapeProperty shape_property;
        shape_property.inverse_mass_kg = 0;
        shape_property.inertia_tensor = InertiaTensor::zero_inertia_tensor();
        return shape_property;
    }
};