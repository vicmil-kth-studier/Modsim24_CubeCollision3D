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
            GPUProgram texture_program;
            IndexVertexBufferPair index_vertex_buffer;
            Texture text_texture;
            Shared3DModelsBuffer shared_models_buffer;
            Camera camera;
            App() : 
            graphics_setup(GraphicsSetup::create_setup()) {
                frame_stabilizer = FrameStabilizer(30);
                
                // Create Vertex Array Object
                create_vertex_array_object();

                Debug(glGetString (GL_SHADING_LANGUAGE_VERSION));

                // Load gpu program for how to interpret the data
                texture_program = GPUProgram::from_strings(vicmil::shader_example::texture_vert_shader, vicmil::shader_example::texture_frag_shader);
                program = GPUProgram::from_strings(vicmil::shader_example::vert_shader, vicmil::shader_example::frag_shader);
                std::vector<float> vertices = {0.0f, 0.5f, 3.0f,
                         0.5f, -0.5f, 3.0f,
                         -0.5f, -0.5f, 3.0f};
                std::vector<unsigned int> faces = {1, 2, 3};

                index_vertex_buffer = vicmil::IndexVertexBufferPair::from_raw_data(
                    &faces[0], 
                    3*sizeof(unsigned int), 
                    &vertices[0],
                    9 * sizeof(float));

                RawImageRGB raw_image_alphabet = graphics_help::get_raw_image_of_alphabet();
                text_texture = Texture::from_raw_image_rgb(raw_image_alphabet);

                // Load models
                shared_models_buffer = Shared3DModelsBuffer::from_models(graphics_help::get_models_vector());
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
        void draw2d_text(std::string text, double x = -1.0, double y = 1.0, double letter_width = 0.1, double screen_aspect_ratio = 1.0) {
            // Make sure the right shader is loaded
            app->texture_program.bind_program();
            
            // Add character rectangles to vertex and index buffer
            TextureTraingles text_trig = graphics_help::get_texture_triangles_from_text(text, x, y, letter_width, screen_aspect_ratio);

            // Load the vertex buffer
            text_trig.overwrite_index_vertex_buffer_pair(app->index_vertex_buffer);
            app->index_vertex_buffer.bind();
            app->index_vertex_buffer.set_texture_vertex_buffer_layout();

            // Make sure the right texture is loaded
            app->text_texture.bind();

            // Ensure that depth testing is disabled
            set_depth_testing_enabled(false);

            // Create a mvp matrix that does nothing
            glm::mat4 matrix_mvp = glm::mat4(1.0);
            //UniformBuffer::set_mat4f(matrix_mvp, app->texture_program, "u_MVP");

            // Perform the drawing
            app->index_vertex_buffer.draw();
        }
        void draw3d_text() {
            ThrowNotImplemented();
            // Make sure the right shader is loaded

            // Add rectangles to vertex and index buffer

            // Create a mvp matrix that transforms the position according to camera

            // Ensure that depth testing is enabled

            // Perform the drawing
        }
        
        void draw_3d_model(unsigned int model_index, vicmil::ModelOrientation obj_orientation, double scale = 1.0) {
            // Make sure the right shader is loaded
            app->program.bind_program();
            
            // Load the correct model:
            app->shared_models_buffer.bind();

            // Create a mvp matrix that transforms the position according to camera
            PerspectiveMatrixGen mvp_gen;
            mvp_gen.obj_scale = scale;
            mvp_gen.load_camera_state(vicmil::app_help::app->camera);
            mvp_gen.load_object_orientation(obj_orientation);
            glm::mat4 mvp = mvp_gen.get_perspective_matrix_MVP();
            
            // Ensure that depth testing is enabled:
            set_depth_testing_enabled(true);

            // Perform the drawing
            vicmil::app_help::app->shared_models_buffer.draw_object(model_index, mvp, &vicmil::app_help::app->program);
        }

        class TextButton {
        public:
            std::string text;
            double letter_width = 0.04;
            double center_x;
            double center_y;
            int screen_width_pixels;
            int screen_height_pixels;
            bool is_pressed(vicmil::MouseState mouse_state) {
                double width = vicmil::graphics_help::get_letter_width_with_spacing(letter_width) * text.size();;
                double height = vicmil::graphics_help::get_letter_height_with_spacing(letter_width, screen_width_pixels / screen_height_pixels);
                double x = center_x - width / 2;
                double y = center_y + height / 2 - height;

                Button button = Button();
                button.screen_width = screen_width_pixels;
                button.screen_height = screen_height_pixels;
                button.width = width;
                button.height = height;
                button.start_x = x;
                button.start_y = y;
                return button.is_pressed(mouse_state);
            }
            void draw() {
                double width = vicmil::graphics_help::get_letter_width_with_spacing(letter_width) * text.size();
                double height = vicmil::graphics_help::get_letter_height_with_spacing(letter_width, screen_width_pixels / screen_height_pixels);
                double x = center_x - width / 2;
                double y = center_y + height / 2;
                draw2d_text(text, x, y, letter_width, screen_width_pixels / screen_height_pixels);
            }
        };
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