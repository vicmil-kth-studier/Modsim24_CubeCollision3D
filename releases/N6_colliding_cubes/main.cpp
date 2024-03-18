#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib" 
#include "../../source/cubecollision_include.h"

using namespace vicmil;

FPSCounter fps_counter;

// Used when applying force
Cube cube1;
Cube cube2;
glm::dvec3 gravity_m_s2 = glm::dvec3(0, -1, 0);

const int FPS = 30;
bool start_pressed = false;
ContactImpulse cube_impulse = ContactImpulse::zero();

void render() {
    clear_screen();

    // Update camera
    int screen_width_pixels;
    int screen_height_pixels;
    vicmil::app::globals::main_app->graphics_setup.get_window_size(&screen_width_pixels, &screen_height_pixels);
    double screen_aspect_ratio = vicmil::app::globals::screen_width / vicmil::app::globals::screen_height;
    vicmil::app::globals::main_app->camera.screen_aspect_ratio = screen_aspect_ratio;

    // Draw cube1
    ModelOrientation cube_orientation1 = get_model_orientation_from_obj_trajectory(cube1.trajectory);
    vicmil::app::draw_3d_model(graphics_help::BLUE_CUBE_INDEX, cube_orientation1, 0.5);

    // Draw cube2
    ModelOrientation cube_orientation2 = get_model_orientation_from_obj_trajectory(cube2.trajectory);
    vicmil::app::draw_3d_model(graphics_help::BLUE_CUBE_INDEX, cube_orientation2, 0.5);

    // Draw contact position(where the impulse is applied)
    ModelOrientation impulse_orientation;
    impulse_orientation.position = cube_impulse.position;
    vicmil::app::draw_3d_model(graphics_help::RED_SPHERE_INDEX, impulse_orientation, 0.1);

    // Draw plane to show collision direction
    if(glm::length(cube_impulse.impulse.impulse_newton_s) > 0.01) {
        glm::dvec3 normalized_impulse = glm::normalize(cube_impulse.impulse.impulse_newton_s);
        Rotation impulse_rotation = Rotation::from_rotation_of_vectors(glm::dvec3(0, 1, 0), normalized_impulse);
        glm::mat4 rotation_matrix = impulse_rotation.to_matrix();
        
        Assert(glm::length(impulse_rotation.to_matrix3x3() * glm::dvec3(0, 1, 0) - normalized_impulse) < 0.001);

        impulse_orientation.rotation = rotation_matrix;
        vicmil::app::draw_3d_model(graphics_help::RED_PLANE_INDEX, impulse_orientation, 2.0);
    }


    fps_counter.record_frame();
    double fps = fps_counter.get_fps();
    std::string info_str = "fps: " + std::to_string(fps);

    MouseState mouse_state = MouseState();
    info_str += "   x: " + std::to_string(vicmil::x_pixel_to_opengl(mouse_state.x(), screen_width_pixels));
    info_str += "   y: " + std::to_string(vicmil::y_pixel_to_opengl(mouse_state.y(), screen_height_pixels));
    info_str += "   impulse: " + std::to_string(glm::length(cube_impulse.impulse.impulse_newton_s));

    vicmil::app::draw2d_text(info_str, -1.0, 1.0, 0.02, screen_aspect_ratio);

    // Create buttons for moving around
    vicmil::app::TextButton text_button;
    text_button.text = "CUBE1 ROTATE1";
    text_button.center_x = 0.5;
    text_button.center_y = 0.2;
    text_button.letter_width = 0.03;
    text_button.screen_width_pixels = screen_width_pixels;
    text_button.screen_height_pixels = screen_height_pixels;

    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube1.trajectory.orientation.rotational_orientation = cube1.trajectory.orientation.rotational_orientation.rotate(
            Rotation::from_scaled_axis({0.1, 0, 0}));
    }

    text_button.center_y -= 0.1;
    text_button.text = "CUBE1 ROTATE2";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube1.trajectory.orientation.rotational_orientation = cube1.trajectory.orientation.rotational_orientation.rotate(
            Rotation::from_scaled_axis({0, 0.1, 0}));
    }

    text_button.center_y -= 0.1;
    text_button.text = "CUBE1 ROTATE3";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube1.trajectory.orientation.rotational_orientation = cube1.trajectory.orientation.rotational_orientation.rotate(
            Rotation::from_scaled_axis({0, 0, 0.1}));
    }

    text_button.center_y -= 0.1;
    text_button.text = "CUBE2 ROTATE1";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube2.trajectory.orientation.rotational_orientation = cube2.trajectory.orientation.rotational_orientation.rotate(
            Rotation::from_scaled_axis({0.1, 0, 0}));
    }

    text_button.center_y -= 0.1;
    text_button.text = "CUBE2 ROTATE2";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube2.trajectory.orientation.rotational_orientation = cube2.trajectory.orientation.rotational_orientation.rotate(
            Rotation::from_scaled_axis({0, 0.1, 0}));
    }

    text_button.center_y -= 0.1;
    text_button.text = "CUBE2 ROTATE3";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube2.trajectory.orientation.rotational_orientation = cube2.trajectory.orientation.rotational_orientation.rotate(
            Rotation::from_scaled_axis({0, 0, 0.1}));
    }

    text_button.center_y -= 0.1;
    text_button.text = "START";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        start_pressed = true;
    }
    text_button.center_y -= 0.1;
    text_button.text = "PAUSE";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == true) {
        start_pressed = false;
    }

    vicmil::app::draw2d_text(
        "Interact with the simulation using the buttons on the right. \n"
        "This simulation is about simulating two cubes colliding with each other\n"
        "\n"
        "The simulation will start when \n"
        "START \n"
        "is pressed. You can move the cubes using the buttons to the right to change\n"
        "the initial position before the simulation starts", 
        -0.9, 0.8, 0.02, screen_aspect_ratio);
}

// Runs at a fixed framerate
void game_loop() {
    cube_impulse = ContactImpulse::zero();
    cube_impulse.position.x = 100000000; // Far far away
    if(start_pressed) {
        cube1.trajectory.move_time_step_s(1.0 / FPS);
        cube2.trajectory.move_time_step_s(1.0 / FPS);
        cube_impulse = handle_cube_cube_collision(cube1, cube2, 1.0);
        //cube_impulse = handle_cube_plane_collision(cube, ground_plane, 0.8);
    }
}

void init() {
    Debug("C++ init!");
    vicmil::app::set_render_func(VoidFuncRef(render));
    vicmil::app::set_game_update_func(VoidFuncRef(game_loop));
    vicmil::app::set_game_updates_per_second(FPS);
    fps_counter = FPSCounter();



    cube1 = Cube();
    cube1.trajectory.orientation.center_of_mass.x = -2.0;
    cube1.trajectory.orientation.center_of_mass.y = 4.0;
    cube1.trajectory.orientation.center_of_mass.z = -15.0;
    
    cube1.side_length_m = 1;
    cube1.mass_kg = 10;
    cube1.trajectory.linear_velocity.speed_m_per_s.x = 1.0;

    cube2 = Cube();
    cube2.trajectory.orientation.center_of_mass.x = 2.0;
    cube2.trajectory.orientation.center_of_mass.y = 4.0;
    cube2.trajectory.orientation.center_of_mass.z = -15.0;

    cube2.side_length_m = 1;
    cube2.mass_kg = 10;

    vicmil::app::globals::main_app->camera.position.y = 6;
}


// Handle emscripten
void emscripten_update() {
    vicmil::app::app_loop_handler(vicmil::VoidFuncRef(init));
}
int main(int argc, char *argv[]) {
    Debug("Main!");
    emscripten_set_main_loop(emscripten_update, 0, 1);
    return 0;
};

