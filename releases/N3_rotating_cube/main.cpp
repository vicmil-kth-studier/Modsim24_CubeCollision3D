#include "../../source/cubecollision_include.h"

static bool quitting = false;
static Setup setup;
static Program program;
static float offset;
Shared3DModelsBuffer shared_models_buffer;
Camera camera;
CameraMover camera_mover;
float obj_rotation_rad = 0.0;

void render() {
    //START_TRACE_FUNCTION();
    SDL_GL_MakeCurrent(setup.window, setup.gl_context);
    clear_screen();

    // Update camera
    camera_mover.update();

    // Update object
    Orientation obj_orientation;
    obj_rotation_rad += 0.05;
    obj_orientation.position.z = -15.0;
    obj_orientation.position.y = -2;
    obj_orientation.rotation = glm::rotate(obj_rotation_rad, glm::vec3(0.0, 1.0, 0.0));

    // Draw object
    for(int i = 0; i < 3; i++) {
        obj_orientation.position.x = -3 + 2.5*i;
        PerspectiveMatrixGen mvp_gen;
        mvp_gen.load_camera_state(camera);
        mvp_gen.load_object_orientation(obj_orientation);
        glm::mat4 mvp = mvp_gen.get_perspective_matrix_MVP();
        shared_models_buffer.draw_object(graphics_help::BLUE_CUBE_INDEX, mvp, &program);
    }

    glm::vec3 view_vector = camera.get_camera_view_vector();
    obj_orientation.position = view_vector + camera.position;
    obj_orientation.rotation = glm::mat4(1.0);

    PerspectiveMatrixGen mvp_gen;
    mvp_gen.obj_scale = 0.01;
    mvp_gen.load_camera_state(camera);
    mvp_gen.load_object_orientation(obj_orientation);
    glm::mat4 mvp = mvp_gen.get_perspective_matrix_MVP();
    shared_models_buffer.draw_object(2, mvp, &program);
    

    SDL_GL_SwapWindow(setup.window);
    //END_TRACE_FUNCTION();
}

void update(){
    SDL_Event event;
    while( SDL_PollEvent(&event) ) {
        if(event.type == SDL_QUIT) {
            quitting = true;
        }
    }

    render();
};

extern "C" {
    int set_screen_size(double width, double height) {
        camera.screen_aspect_ratio = width / height;
        return 0;
    }
}

int main(int argc, char *argv[]) {
    Debug("Start!");
    setup = Setup::create_setup();

    // Create Vertex Array Object
    create_vertex_array_object();

    Debug(glGetString (GL_SHADING_LANGUAGE_VERSION));

    // Load gpu program for how to interpret the data
    program = Program::from_strings(graphics_help::vert_shader, graphics_help::frag_shader);
    program.bind_program();

    // Load models
    shared_models_buffer = Shared3DModelsBuffer::from_models(graphics_help::get_models_vector());

    set_depth_testing_enabled(true);

    camera_mover.camera = &camera;

    // register update as callback
    emscripten_set_main_loop(update, 0, 1);
}