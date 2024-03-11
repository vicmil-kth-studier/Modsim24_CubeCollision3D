#include "L10_obj_mtl_text.h"

namespace vicmil {
    /**
    A wrapper to make it easier to write emscripten programs
    */
   namespace app_help {
        class VoidFuncRef {
            em_callback_func _func;
            bool _callable = false;
        public:
            VoidFuncRef() :
                _callable(false) {}
            VoidFuncRef(em_callback_func func) :
                _func(func) {
                    _callable = true;
            }
            int call() {
                if(_callable) {
                    _func();
                    return 0;
                }

                Debug("Function not set!");
                return 1;
            }
        };
        namespace globals {
            static double screen_width = 1000;
            static double screen_height = 1000;
            static VoidFuncRef game_update_func = VoidFuncRef(); // A reference to the function that will handle the game updates, void function without any args
            static VoidFuncRef render_func = VoidFuncRef(); // A reference to the function that will handle the rendering, void function without any args
            static bool init_called = false;
        }
        
        class App {
        public:
            // Add graphics setup, programs and other stuff here
            vicmil::FrameStabilizer frame_stabilizer;
            GraphicsSetup graphics_setup;
            GPUProgram program;
            Shared3DModelsBuffer shared_models_buffer;
            Camera camera;
            App() : 
            graphics_setup(GraphicsSetup::create_setup()) {
                frame_stabilizer = FrameStabilizer(30);
                
                // Create Vertex Array Object
                create_vertex_array_object();

                Debug(glGetString (GL_SHADING_LANGUAGE_VERSION));

                // Load gpu program for how to interpret the data
                program = GPUProgram::from_strings(vicmil::shader_example::vert_shader, vicmil::shader_example::frag_shader);
                program.bind_program();

                // Load models
                shared_models_buffer = Shared3DModelsBuffer::from_models(graphics_help::get_models_vector());

                set_depth_testing_enabled(true);
            }
        };
        App* app = nullptr;

        void emscripten_loop_handler(VoidFuncRef init_func) {
            DisableLogging;
            if(globals::init_called == false) {
                if(app == nullptr) {
                    app = new App();
                }   
                init_func.call();
                globals::init_called = true;
            }
            Debug("emscripten_loop_handler");
            Debug("screen width:  " << globals::screen_width);
            if(app != nullptr) {
                if(app->frame_stabilizer.get_time_to_next_frame_s() < 0) {
                    if(globals::game_update_func.call() != 0) {
                        Debug("Game update func not set!");
                    }
                    app->frame_stabilizer.record_frame();
                }
            }
 
            // Render
            SDL_GL_MakeCurrent(vicmil::app_help::app->graphics_setup.window, vicmil::app_help::app->graphics_setup.gl_context);
            if(globals::render_func.call() != 0) {
                Debug("Render func not set!");
            }
            SDL_GL_SwapWindow(vicmil::app_help::app->graphics_setup.window);
        }
        void set_game_update_func(VoidFuncRef func) {
            globals::game_update_func = func;
        }
        void set_render_func(VoidFuncRef func) {
            globals::render_func = func;
        }
        void set_game_updates_per_second(unsigned int updates_per_s) {
            if(app != nullptr) {
                app->frame_stabilizer = FrameStabilizer(updates_per_s);
            }
        }
        void draw2d_rect() {
            ThrowNotImplemented();
            // Make sure the right shader is loaded

            // Add rectangle to vertex and index buffer

            // Create a mvp matrix that does nothing

            // Ensure that depth testing is disabled

            // Perform the drawing

            // glm::mat4 mvp = glm::mat4(1.0);
            
            // Ensure that depth testing is enabled: TODO

            // Perform the drawing
            // vicmil::app_help::app->shared_models_buffer.draw_object(2, mvp, &vicmil::app_help::app->program);
        }
        void draw2d_text() {
            ThrowNotImplemented();
            // Make sure the right shader is loaded

            // Add character rectangles to vertex and index buffer

            // Create a mvp matrix that does nothing

            // Make sure the right texture is loaded

            // Ensure that depth testing is disabled

            // Perform the drawing
        }
        void draw3d_text() {
            ThrowNotImplemented();
            // Make sure the right shader is loaded

            // Add rectangles to vertex and index buffer

            // Create a mvp matrix that transforms the position according to camera

            // Ensure that depth testing is enabled

            // Perform the drawing
        }
        
        void draw_3d_model(unsigned int model_index, vicmil::Orientation obj_orientation, double scale = 1.0) {
            // Make sure the right shader is loaded: TODO

            // Load the correct model: TODO

            // Create a mvp matrix that transforms the position according to camera
            PerspectiveMatrixGen mvp_gen;
            mvp_gen.obj_scale = scale;
            mvp_gen.load_camera_state(vicmil::app_help::app->camera);
            mvp_gen.load_object_orientation(obj_orientation);
            glm::mat4 mvp = mvp_gen.get_perspective_matrix_MVP();
            
            // Ensure that depth testing is enabled: TODO

            // Perform the drawing
            vicmil::app_help::app->shared_models_buffer.draw_object(model_index, mvp, &vicmil::app_help::app->program);
        }
    }
}

// Make sure javascript can send updates when screen size changes
extern "C" {
    int set_screen_size(double width, double height) {
        if(vicmil::app_help::app != nullptr)
        vicmil::app_help::globals::screen_width = width;
        vicmil::app_help::globals::screen_height = height;
        return 0;
    }
}