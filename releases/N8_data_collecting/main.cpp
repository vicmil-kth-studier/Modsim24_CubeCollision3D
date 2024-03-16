#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,init(),main(),handle_cube_cube_collision" 
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

// Save the simulation data over time
std::vector<double> time_data_s = {};
std::vector<double> total_energy_J = {};
std::vector<double> potential_energy_J = {};
std::vector<double> kinetic_energy_J = {};
double simulated_time = 0;


void render() {
    clear_screen();

    // Update camera
    int screen_width_pixels;
    int screen_height_pixels;
    vicmil::app::globals::main_app->graphics_setup.get_window_size(&screen_width_pixels, &screen_height_pixels);
    double screen_aspect_ratio = vicmil::app::globals::screen_width / vicmil::app::globals::screen_height;
    vicmil::app::globals::main_app->camera.screen_aspect_ratio = screen_aspect_ratio;

    // Draw cube
    ModelOrientation cube_orientation = get_model_orientation_from_obj_trajectory(cube.trajectory);
    vicmil::app::draw_3d_model(graphics_help::BLUE_CUBE_INDEX, cube_orientation, 0.5);

    // Draw ground plane
    ModelOrientation sphere_orientation = ModelOrientation();
    sphere_orientation.position = ground_plane.point;
    vicmil::app::draw_3d_model(graphics_help::RED_PLANE_INDEX, sphere_orientation, 100);


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

    text_button.center_y -= 0.1;
    text_button.text = "DOWNLOAD DATA";
    text_button.draw();
    if(text_button.is_pressed(mouse_state) && start_pressed == true) {
        vicmil::browser::alert("Hello from c++");
        vicmil::json::Json j = vicmil::json::Json();
        j["description"] = "This is some energy data over time!";
        j["time_data_s"] = time_data_s;
        j["total_energy_data_J"] = total_energy_J;
        j["kinetic_energy_J"] = kinetic_energy_J;
        j["potential_energy_J"] = potential_energy_J;
        
        std::string json_str = j.to_string();
        vicmil::browser::download_text_file("falling_cube.json", json_str);
    }

    vicmil::app::draw2d_text(
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

        // Record the simulation data
        time_data_s.push_back(simulated_time);
        ObjectEnergyInfo cube_energy = get_cube_energy_information(cube, 1);
        total_energy_J.push_back(cube_energy.potential_energy + cube_energy.linear_kin_energy + cube_energy.rotational_kin_energy);
        kinetic_energy_J.push_back(cube_energy.linear_kin_energy + cube_energy.rotational_kin_energy);
        potential_energy_J.push_back(cube_energy.potential_energy);
        simulated_time += 1.0 / FPS;
    }
}

void init() {
    Debug("C++ init!");
    vicmil::app::set_render_func(VoidFuncRef(render));
    vicmil::app::set_game_update_func(VoidFuncRef(game_loop));
    vicmil::app::set_game_updates_per_second(FPS);
    fps_counter = FPSCounter();

    cube = Cube();
    cube.trajectory.orientation.center_of_mass.x = 0.0;
    cube.trajectory.orientation.center_of_mass.y = 4.0;
    cube.trajectory.orientation.center_of_mass.z = -15.0;
    
    cube.side_length_m = 1;
    cube.mass_kg = 10;

    ground_plane.point = glm::dvec3(0, 0, 0);
    ground_plane.normal = glm::dvec3(0, 1, 0);

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

