#include "../../source/cubecollision_include.h"

using namespace vicmil;
CameraMover camera_mover;
float obj_rotation_rad = 0.0;

void render() {
    clear_screen();

    // Update camera
    camera_mover.update();
    vicmil::app_help::app->camera.screen_aspect_ratio = vicmil::app_help::globals::screen_width / vicmil::app_help::globals::screen_height;

    // Update object
    Orientation obj_orientation;
    obj_orientation.position.z = -15.0;
    obj_orientation.position.y = -2;
    obj_orientation.rotation = glm::rotate(obj_rotation_rad, glm::vec3(0.0, 1.0, 0.0));

    // Draw object
    for(int i = 0; i < 3; i++) {
        obj_orientation.position.x = -3 + 2.5*i;
        
        vicmil::app_help::draw_3d_model(graphics_help::BLUE_CUBE_INDEX, obj_orientation, 1.0);
    }

    glm::vec3 view_vector = vicmil::app_help::app->camera.get_camera_view_vector();
    obj_orientation.position = view_vector + vicmil::app_help::app->camera.position;
    obj_orientation.rotation = glm::mat4(1.0);

    vicmil::app_help::draw_3d_model(2, obj_orientation, 0.01);
}

// Runs at a fixed frame rate
void game_loop() {
    obj_rotation_rad += 0.05;
}

void init() {
    Debug("C++ init!");
    vicmil::app_help::set_render_func(app_help::VoidFuncRef(render));
    vicmil::app_help::set_game_update_func(app_help::VoidFuncRef(game_loop));
    vicmil::app_help::set_game_updates_per_second(30);
    camera_mover.camera = &vicmil::app_help::app->camera;
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

