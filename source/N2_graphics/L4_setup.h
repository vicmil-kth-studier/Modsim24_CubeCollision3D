#include "L3_buffer.h"

class Setup {
public:
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;
    SDL_GLContext gl_context;

    Setup() {}
    static Setup create_setup() {
        Setup setup = Setup();
        setup.create_window_and_renderer();
        //setup.setup_frag_shader_blending();
        return setup;
    }
    static void setup_frag_shader_blending() {
        // Enable blending, which makes textures look less blocky
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }
    void create_window_and_renderer() {
        /* Create a windowed mode window and its OpenGL context */
        SDL_CreateWindowAndRenderer(1024, 1024, SDL_WINDOW_OPENGL, &window, &renderer);

        Debug("OpenGL version " << glGetString(GL_VERSION));
    }
    ~Setup() {
    }
};