#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,init(),main(),handle_cube_cube_collision" 
#include "../../source/cubecollision_include.h"

using namespace vicmil;

FPSCounter fps_counter;

// Used when applying force
std::vector<Cube> cubes;
Plane ground_plane;
glm::dvec3 gravity_m_s2 = glm::dvec3(0, -1, 0);

const int FPS = 30;
bool start_pressed = false;

void render() {
    clear_screen();

    // Update camera
    int screen_width_pixels;
    int screen_height_pixels;
    vicmil::app_help::app->graphics_setup.get_window_size(&screen_width_pixels, &screen_height_pixels);
    double screen_aspect_ratio = vicmil::app_help::globals::screen_width / vicmil::app_help::globals::screen_height;
    vicmil::app_help::app->camera.screen_aspect_ratio = screen_aspect_ratio;

    // Draw cubes
    for(int i = 0; i < cubes.size(); i++) {
        ModelOrientation cube_orientation = get_model_orientation_from_obj_trajectory(cubes[i].trajectory);
        vicmil::app_help::draw_3d_model(graphics_help::BLUE_CUBE_INDEX, cube_orientation, 0.5);
    }

    // Draw ground plane
    ModelOrientation ground_orientation = ModelOrientation();
    ground_orientation.position = ground_plane.point;
    vicmil::app_help::draw_3d_model(graphics_help::RED_PLANE_INDEX, ground_orientation, 100);


    fps_counter.record_frame();
    double fps = fps_counter.get_fps();
    std::string info_str = "fps: " + std::to_string(fps);

    MouseState mouse_state = MouseState();
    info_str += "   x: " + std::to_string(vicmil::x_pixel_to_opengl(mouse_state.x(), screen_width_pixels));
    info_str += "   y: " + std::to_string(vicmil::y_pixel_to_opengl(mouse_state.y(), screen_height_pixels));

    vicmil::app_help::draw2d_text(info_str, -1.0, 1.0, 0.02, screen_aspect_ratio);

    // Create buttons for moving around
    // Create buttons for moving around
    vicmil::app_help::TextButton text_button;
    text_button.center_x = 0.5;
    text_button.center_y = 0.2;
    text_button.letter_width = 0.03;
    text_button.screen_width_pixels = screen_width_pixels;
    text_button.screen_height_pixels = screen_height_pixels;

    text_button.center_y -= 0.1;
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
        for(int i = 0; i < cubes.size(); i++) {
            apply_acceleration(gravity_m_s2, 1.0 / FPS, cubes[i].trajectory);
            cubes[i].trajectory.move_time_step_s(1.0 / FPS);
        }

        for(int i = 0; i < cubes.size(); i++) {
            for(int i2 = 0; i2 < i; i2++) {
                handle_cube_cube_collision(cubes[i], cubes[i2], 1.0);
            }
            handle_cube_plane_collision(cubes[i], ground_plane, 0.8);
        }
    }
}

void init() {
    Debug("C++ init!");
    vicmil::app_help::set_render_func(app_help::VoidFuncRef(render));
    vicmil::app_help::set_game_update_func(app_help::VoidFuncRef(game_loop));
    vicmil::app_help::set_game_updates_per_second(FPS);
    fps_counter = FPSCounter();

    cubes.resize(10);

    srand(time(0));

    for(int i = 0; i < cubes.size(); i++) {
        cubes[i] = Cube();
        cubes[i].trajectory.orientation.center_of_mass.x = -2.0;
        cubes[i].trajectory.orientation.center_of_mass.y = 4.0;
        cubes[i].trajectory.orientation.center_of_mass.z = -15.0;
        cubes[i].trajectory.orientation.center_of_mass += glm::dvec3((rand()%120)/40.0, (rand()%120)/20.0, (rand()%120)/40.0);

        // Setup random rotation
        double rad = 2 * vicmil::PI * (rand()%100) / 100.0;
        glm::dvec3 axis = glm::dvec3((rand()%10000) / 10000.0, (rand()%10000) / 10000.0, (rand()%10000) / 10000.0);
        cubes[i].trajectory.orientation.rotational_orientation = Rotation::from_axis_rotation(rad, glm::normalize(axis));
        
        // Setup other properties
        cubes[i].side_length_m = 1;
        cubes[i].mass_kg = 10;
    }

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

