#pragma once
#include "../N1_util/util_include.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>
#include <emscripten.h>
#include "../../dependencies/glm/glm/glm.hpp"
#include "../../dependencies/glm/glm/gtc/matrix_transform.hpp"
#include "../../dependencies/glm/glm/gtx/transform.hpp"
#include "../../dependencies/glm/glm/gtx/string_cast.hpp"
#include "../../dependencies/glm/glm/ext/matrix_transform.hpp" // perspective, translate, rotate


static void GLClearError() {
    //START_TRACE_FUNCTION();
    bool error_detected = false;
    while(GLenum error = glGetError()) {
        std::cout << "unhandled [OpenGl Error] (" << error << ")" << std::endl;
        error_detected = true;
    }
    if(error_detected) {
        Debug("Throw exception");
        throw std::invalid_argument("opengl threw exception!");
    }
    //END_TRACE_FUNCTION();
    return;
}

static void GLCheckError() {
    //START_TRACE_FUNCTION();
    bool error_detected = false;
    while(GLenum error = glGetError()) {
        std::cout << "[OpenGl Error] (" << error << ")" << std::endl;
        error_detected = true;
    }
    if(error_detected) {
        Debug("Throw exception");
        throw std::invalid_argument("opengl threw exception!");
    }
    //END_TRACE_FUNCTION();
}

template <class T>
static void expect_value(T val, T expected, std::string err_message) {
    if(val != expected) {
        std::cout << err_message << ", expected " << expected << "got: " << val << std::endl;
        throw;
    }
}

template<class T>
void expect_vec_length(std::vector<T>& vec, int size, std::string err_message) {
    if (vec.size() != size) {
        std::cout << err_message << "expected size " << size << " got " << vec.size() << std::endl;
        std::cout << str_vec_to_str(vec);
        throw;
    }
}

template<class T>
bool in_vector(T val, std::vector<T>& vec) {
    for(int i = 0; i < vec.size(); i++) {
        if(val == vec[i]) {
            return true;
        }
    }
    return false;
}

#define GLCall(x) try{GLClearError(); x; GLCheckError();} catch (const std::exception& e) {ThrowError("Opengl threw error!: " << e.what());}

class Timer {
public:
    Uint32 start_ms;
    Timer() {
        start_ms = SDL_GetTicks();
    }
    Uint32 get_time_since_start_ms() {
        return SDL_GetTicks() - start_ms;
    }
    double get_time_since_start_s() {
        return get_time_since_start_ms() / 1000.0;
    }
    void reset() {
        start_ms = SDL_GetTicks();
    }
};

class FrameStabilizer {
public:
    unsigned int _frames_per_second;
    unsigned int _update_count;
    Timer timer;
    FrameStabilizer() {}
    FrameStabilizer(unsigned int frames_per_second_) {
        _frames_per_second = frames_per_second_;
        reset();
    }
    void reset() {
        _update_count = 0;
        timer = Timer();
    }
    double get_time_to_next_frame_s() {
        double next_frame_time_s = ((double)_update_count) / (double)_frames_per_second;
        double time_to_next_frame_s = next_frame_time_s - timer.get_time_since_start_s();
        return time_to_next_frame_s;
    }
    void record_frame() {
        _update_count += 1;
    }
    double get_expected_frame_count() {
        return timer.get_time_since_start_s() * _frames_per_second;
    }
};