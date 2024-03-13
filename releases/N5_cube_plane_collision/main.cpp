#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,init(),main()" 
#include "../../source/cubecollision_include.h"

using namespace vicmil;

FPSCounter fps_counter;

// Used when applying force
Cube cube;
Plane ground_plane;
glm::dvec3 gravity_m_s2 = glm::dvec3(0, -1, 0);

const int FPS = 30;
bool start_pressed = false;
ContactImpulse cube_impulse = ContactImpulse::zero();

void render() {
    clear_screen();

    // Update camera
    int screen_width_pixels;
    int screen_height_pixels;
    vicmil::app_help::app->graphics_setup.get_window_size(&screen_width_pixels, &screen_height_pixels);
    double screen_aspect_ratio = vicmil::app_help::globals::screen_width / vicmil::app_help::globals::screen_height;
    vicmil::app_help::app->camera.screen_aspect_ratio = screen_aspect_ratio;

    // Draw cube
    ModelOrientation cube_orientation = get_model_orientation_from_obj_trajectory(cube.trajectory);
    vicmil::app_help::draw_3d_model(graphics_help::BLUE_CUBE_INDEX, cube_orientation, 0.5);

    // Draw ground plane
    ModelOrientation sphere_orientation = ModelOrientation();
    sphere_orientation.position = ground_plane.point;
    vicmil::app_help::draw_3d_model(graphics_help::RED_PLANE_INDEX, sphere_orientation, 100);


    fps_counter.record_frame();
    double fps = fps_counter.get_fps();
    std::string info_str = "fps: " + std::to_string(fps);

    MouseState mouse_state = MouseState();
    info_str += "   x: " + std::to_string(vicmil::x_pixel_to_opengl(mouse_state.x(), screen_width_pixels));
    info_str += "   y: " + std::to_string(vicmil::y_pixel_to_opengl(mouse_state.y(), screen_height_pixels));
    info_str += "   impulse: " + std::to_string(glm::length(cube_impulse.impulse.impulse_newton_s));

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
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube.trajectory.orientation.rotational_orientation =
            cube.trajectory.orientation.rotational_orientation.rotate(Rotation::from_axis_rotation(0.02, glm::dvec3(1, 0, 0)));
    }

    text_button.center_y = 0.1;
    text_button.text = "DOWN";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube.trajectory.orientation.rotational_orientation =
            cube.trajectory.orientation.rotational_orientation.rotate(Rotation::from_axis_rotation(-0.02, glm::dvec3(1, 0, 0)));
    }

    text_button.center_y = 0.15;
    text_button.center_x = 0.35;
    text_button.text = "LEFT";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube.trajectory.orientation.rotational_orientation =
            cube.trajectory.orientation.rotational_orientation.rotate(Rotation::from_axis_rotation(0.02, glm::dvec3(0, 1, 0)));
    }

    text_button.center_y = 0.15;
    text_button.center_x = 0.65;
    text_button.text = "RIGHT";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        cube.trajectory.orientation.rotational_orientation =
            cube.trajectory.orientation.rotational_orientation.rotate(Rotation::from_axis_rotation(-0.02, glm::dvec3(0, 1, 0)));
    }


    text_button.center_y = -0.6;
    text_button.center_x = 0.5;
    text_button.text = "START";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == false) {
        start_pressed = true;
    }

    vicmil::app_help::draw2d_text(
        "Interact with the simulation using the buttons on the right. \n"
        "This simulation is about simulating the cube being dropped on the plane\n"
        "\n"
        "The simulation will start when \n"
        "START \n"
        "is pressed. You can move the cube using the buttons to the right to change\n"
        "the initial position of the cube before it is dropped", 
        -0.9, 0.8, 0.02, screen_aspect_ratio);
}

// Runs at a fixed framerate
void game_loop() {
    if(start_pressed) {
        apply_acceleration(gravity_m_s2, 1.0 / FPS, cube.trajectory);
        cube.trajectory.move_time_step_s(1.0 / FPS);
        cube_impulse = handle_cube_plane_collision(cube, ground_plane, 0.8);
    }
}

void init() {
    Debug("C++ init!");
    vicmil::app_help::set_render_func(app_help::VoidFuncRef(render));
    vicmil::app_help::set_game_update_func(app_help::VoidFuncRef(game_loop));
    vicmil::app_help::set_game_updates_per_second(FPS);
    fps_counter = FPSCounter();



    cube = Cube();
    cube.trajectory.orientation.center_of_mass.x = 0.0;
    cube.trajectory.orientation.center_of_mass.y = 4.0;
    cube.trajectory.orientation.center_of_mass.z = -15.0;
    
    cube.side_length_m = 1;
    cube.mass_kg = 10;

    ground_plane.point = glm::dvec3(0, 0, 0);
    ground_plane.normal = glm::dvec3(0, 1, 0);

    vicmil::app_help::app->camera.position.y = 6;
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

