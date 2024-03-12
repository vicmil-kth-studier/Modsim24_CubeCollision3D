#include "../../source/cubecollision_include.h"

using namespace vicmil;

FPSCounter fps_counter;

// Used when applying force
ObjectTrajectory cube_trajectory;
ObjectShapeProperty cube_shape;

Impulse impulse;
ContactPointInfo impulse_contact_point;
bool impulse_applied = false;

const int FPS = 30;

Orientation get_orientation_from_obj_trajectory(ObjectTrajectory trajectory) {
    Orientation orientation;
    orientation.position = trajectory.orientation.center_of_mass;
    orientation.rotation = trajectory.orientation.rotational_orientation.to_matrix();
    return orientation;
}

void render() {
    clear_screen();

    // Update camera
    int screen_width_pixels;
    int screen_height_pixels;
    vicmil::app_help::app->graphics_setup.get_window_size(&screen_width_pixels, &screen_height_pixels);
    double screen_aspect_ratio = vicmil::app_help::globals::screen_width / vicmil::app_help::globals::screen_height;
    vicmil::app_help::app->camera.screen_aspect_ratio = screen_aspect_ratio;

    // Draw cube
    Orientation cube_orientation = get_orientation_from_obj_trajectory(cube_trajectory);
    vicmil::app_help::draw_3d_model(graphics_help::BLUE_CUBE_INDEX, cube_orientation, 1.0);


    // Draw sphere at contact point
    Orientation sphere_orientation = Orientation();
    sphere_orientation.position = impulse_contact_point.contact_position;
    vicmil::app_help::draw_3d_model(graphics_help::RED_SPHERE_INDEX, sphere_orientation, 0.1);


    fps_counter.record_frame();
    double fps = fps_counter.get_fps();
    std::string info_str = "fps: " + std::to_string(fps);

    MouseState mouse_state = MouseState();
    info_str += "   x: " + std::to_string(vicmil::x_pixel_to_opengl(mouse_state.x(), screen_width_pixels));
    info_str += "   y: " + std::to_string(vicmil::y_pixel_to_opengl(mouse_state.y(), screen_height_pixels));

    vicmil::app_help::draw2d_text(info_str, -1.0, 1.0, 0.02, screen_aspect_ratio);

    // Create buttons for moving around
    vicmil::app_help::TextButton text_button;
    text_button.text = "UP";
    text_button.center_x = 0.5;
    text_button.center_y = 0.2;
    text_button.letter_width = 0.03;
    text_button.screen_width_pixels = screen_width_pixels;
    text_button.screen_height_pixels = screen_height_pixels;

    text_button.draw();
    if(text_button.is_pressed(mouse_state)) {
        cube_trajectory.orientation.center_of_mass.y += 0.1;
    }

    text_button.center_y = 0.1;
    text_button.text = "DOWN";
    text_button.draw();
    if(text_button.is_pressed(mouse_state)) {
        cube_trajectory.orientation.center_of_mass.y -= 0.1;
    }

    text_button.center_y = 0.15;
    text_button.center_x = 0.35;
    text_button.text = "LEFT";
    text_button.draw();
    if(text_button.is_pressed(mouse_state)) {
        cube_trajectory.orientation.center_of_mass.x -= 0.1;
    }

    text_button.center_y = 0.15;
    text_button.center_x = 0.65;
    text_button.text = "RIGHT";
    text_button.draw();
    if(text_button.is_pressed(mouse_state)) {
        cube_trajectory.orientation.center_of_mass.x += 0.1;
    }


    text_button.center_y = -0.6;
    text_button.center_x = 0.5;
    text_button.text = "APPLY IMPULSE";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && impulse_applied == false) {
        apply_impulse(impulse, impulse_contact_point, cube_trajectory, cube_shape);
        impulse_applied = true;
    }

    vicmil::app_help::draw2d_text(
        "Interact with the simulation using the buttons on the right. \n"
        "This simulation is about simulating an impact at the red sphere\n"
        "\n"
        "An impulse will be applied to the cube at the red sphere when \n"
        "APPLY IMPULSE \n"
        "is pressed. You can move the cube using the buttons to the right to change\n"
        "where the impulse is applied", 
        -0.9, 0.8, 0.02, screen_aspect_ratio);
}

// Runs at a fixed framerate
void game_loop() {
    cube_trajectory.move_time_step_s(1.0 / FPS);
}

void init() {
    Debug("C++ init!");
    vicmil::app_help::set_render_func(app_help::VoidFuncRef(render));
    vicmil::app_help::set_game_update_func(app_help::VoidFuncRef(game_loop));
    vicmil::app_help::set_game_updates_per_second(FPS);
    fps_counter = FPSCounter();



    cube_trajectory = ObjectTrajectory();
    cube_trajectory.orientation.center_of_mass.x = 0.0;
    cube_trajectory.orientation.center_of_mass.y = -2.0;
    cube_trajectory.orientation.center_of_mass.z = -15.0;
    
    double cube_side_length_m = 1;
    double cube_mass_kg = 10;
    cube_shape = ObjectShapeProperty::from_cube(cube_side_length_m, cube_mass_kg);

    glm::dvec3 force_newton = glm::dvec3(0, 0, -5);
    double time_step_s = 1;
    impulse = Impulse::from_force(force_newton, time_step_s);

    impulse_contact_point = ContactPointInfo();
    impulse_contact_point.contact_normal = glm::dvec3(0, 0, 1.0);
    impulse_contact_point.contact_position = glm::dvec3(0, 0, -14);
}


// Handle emscripten
void emscripten_update() {
    vicmil::app_help::emscripten_loop_handler(vicmil::app_help::VoidFuncRef(init));
}
int main(int argc, char *argv[]) {
    Debug("Main!");
    emscripten_set_main_loop(emscripten_update, 0, 1);
    return 0;
};

