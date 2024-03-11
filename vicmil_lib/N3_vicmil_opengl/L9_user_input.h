#include "L8_drawing.h"

namespace vicmil {
class KeyboardState {
    const Uint8* state = SDL_GetKeyboardState(nullptr);
public:
    bool key_is_pressed(unsigned int sdl_key) {
        return (bool)state[sdl_key];
    }
    bool escape_key_is_pressed() {
        return key_is_pressed(SDL_SCANCODE_ESCAPE);
    }
};

/**
 * Create a screenshot of the current mouse state, with mouse position and button presses
*/
class MouseState {
public:
    int _x;
    int _y;
    Uint32 _button_state;
    MouseState() {
        _button_state = SDL_GetMouseState(&_x, &_y);
    }
    int x() {
        return _x;
    }
    int y() {
        return _y;
    }
    bool left_button_is_pressed() {
        return (bool)(_button_state & SDL_BUTTON(1));
    }
    bool middle_button_is_pressed() {
        return (bool)(_button_state & SDL_BUTTON(2));
    }
    bool right_button_is_pressed() {
        return (bool)(_button_state & SDL_BUTTON(3));
    }
};

/**
 * Create a way to move the camera around as you press left click and drag
*/
class CameraMover {
public:
    Camera* camera = nullptr;
    double rotation_speed = 0.01;
    double movement_speed = 0.1;
    int last_mouse_pos[2];
    bool enabled = false;
    CameraMover() {}
    CameraMover(Camera* camera_) {
        camera = camera_;
    }
    void update_rotation() {
        MouseState mouse_state = MouseState();
        if(enabled) {
            int mouse_diff_x = last_mouse_pos[0] - mouse_state.x();
            int mouse_diff_y = last_mouse_pos[1] - mouse_state.y();
            camera->rotate(mouse_diff_y*rotation_speed, mouse_diff_x*rotation_speed);
        }
        enabled = mouse_state.left_button_is_pressed();
        last_mouse_pos[0] = mouse_state.x();
        last_mouse_pos[1] = mouse_state.y();
    }
    void update_movement() {
        KeyboardState keyboard_state = KeyboardState();
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_A)) {
            glm::vec4 movement_vec = camera->get_left_right_rotation_matrix_inv() * glm::vec4(1.0, 0.0, 0.0, 1.0);
            camera->position[0] += movement_speed * movement_vec[0];
            camera->position[2] += movement_speed * movement_vec[2];
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_D)) {
            glm::vec4 movement_vec = camera->get_left_right_rotation_matrix_inv() * glm::vec4(-1.0, 0.0, 0.0, 1.0);
            camera->position[0] += movement_speed * movement_vec[0];
            camera->position[2] += movement_speed * movement_vec[2];
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_W)) {
            glm::vec4 movement_vec = camera->get_left_right_rotation_matrix_inv() * glm::vec4(0.0, 0.0, 1.0, 1.0);
            camera->position[0] += movement_speed * movement_vec[0];
            camera->position[2] += movement_speed * movement_vec[2];
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_S)) {
            glm::vec4 movement_vec = camera->get_left_right_rotation_matrix_inv() * glm::vec4(0.0f, 0.0, -1.0, 1.0);
            camera->position[0] += movement_speed * movement_vec[0];
            camera->position[2] += movement_speed * movement_vec[2];
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_SPACE)) {
            camera->position[1] += movement_speed; // This means we should move up, so y value should increase
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_LSHIFT)) {
            camera->position[1] -= movement_speed; // This means we want to move down, so y value should decrease
        }
    }
    void update() {
        update_rotation();
        update_movement();
    }
};
}