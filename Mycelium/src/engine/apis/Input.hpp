#pragma once

#include <engine/data/Vector2.hpp>
#include <engine/objects/Window.hpp>
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_map>
#include <vector>

enum class Key
{
    // Printable keys
    Space = GLFW_KEY_SPACE,
    Apostrophe = GLFW_KEY_APOSTROPHE,
    Comma = GLFW_KEY_COMMA,
    Minus = GLFW_KEY_MINUS,
    Period = GLFW_KEY_PERIOD,
    Slash = GLFW_KEY_SLASH,

    // Numbers
    Num0 = GLFW_KEY_0,
    Num1 = GLFW_KEY_1,
    Num2 = GLFW_KEY_2,
    Num3 = GLFW_KEY_3,
    Num4 = GLFW_KEY_4,
    Num5 = GLFW_KEY_5,
    Num6 = GLFW_KEY_6,
    Num7 = GLFW_KEY_7,
    Num8 = GLFW_KEY_8,
    Num9 = GLFW_KEY_9,

    // Letters
    A = GLFW_KEY_A,
    B = GLFW_KEY_B,
    C = GLFW_KEY_C,
    D = GLFW_KEY_D,
    E = GLFW_KEY_E,
    F = GLFW_KEY_F,
    G = GLFW_KEY_G,
    H = GLFW_KEY_H,
    I = GLFW_KEY_I,
    J = GLFW_KEY_J,
    K = GLFW_KEY_K,
    L = GLFW_KEY_L,
    M = GLFW_KEY_M,
    N = GLFW_KEY_N,
    O = GLFW_KEY_O,
    P = GLFW_KEY_P,
    Q = GLFW_KEY_Q,
    R = GLFW_KEY_R,
    S = GLFW_KEY_S,
    T = GLFW_KEY_T,
    U = GLFW_KEY_U,
    V = GLFW_KEY_V,
    W = GLFW_KEY_W,
    X = GLFW_KEY_X,
    Y = GLFW_KEY_Y,
    Z = GLFW_KEY_Z,

    // Function keys
    F1 = GLFW_KEY_F1,
    F2 = GLFW_KEY_F2,
    F3 = GLFW_KEY_F3,
    F4 = GLFW_KEY_F4,
    F5 = GLFW_KEY_F5,
    F6 = GLFW_KEY_F6,
    F7 = GLFW_KEY_F7,
    F8 = GLFW_KEY_F8,
    F9 = GLFW_KEY_F9,
    F10 = GLFW_KEY_F10,
    F11 = GLFW_KEY_F11,
    F12 = GLFW_KEY_F12,

    // Special keys
    Escape = GLFW_KEY_ESCAPE,
    Enter = GLFW_KEY_ENTER,
    Tab = GLFW_KEY_TAB,
    Backspace = GLFW_KEY_BACKSPACE,
    Insert = GLFW_KEY_INSERT,
    Delete = GLFW_KEY_DELETE,
    Right = GLFW_KEY_RIGHT,
    Left = GLFW_KEY_LEFT,
    Down = GLFW_KEY_DOWN,
    Up = GLFW_KEY_UP,
    PageUp = GLFW_KEY_PAGE_UP,
    PageDown = GLFW_KEY_PAGE_DOWN,
    Home = GLFW_KEY_HOME,
    End = GLFW_KEY_END,
    CapsLock = GLFW_KEY_CAPS_LOCK,
    ScrollLock = GLFW_KEY_SCROLL_LOCK,
    NumLock = GLFW_KEY_NUM_LOCK,
    PrintScreen = GLFW_KEY_PRINT_SCREEN,
    Pause = GLFW_KEY_PAUSE,

    // Modifier keys
    LeftShift = GLFW_KEY_LEFT_SHIFT,
    LeftControl = GLFW_KEY_LEFT_CONTROL,
    LeftAlt = GLFW_KEY_LEFT_ALT,
    LeftSuper = GLFW_KEY_LEFT_SUPER,
    RightShift = GLFW_KEY_RIGHT_SHIFT,
    RightControl = GLFW_KEY_RIGHT_CONTROL,
    RightAlt = GLFW_KEY_RIGHT_ALT,
    RightSuper = GLFW_KEY_RIGHT_SUPER,
    Menu = GLFW_KEY_MENU
};

enum class MouseButton
{
    Left = GLFW_MOUSE_BUTTON_LEFT,
    Right = GLFW_MOUSE_BUTTON_RIGHT,
    Middle = GLFW_MOUSE_BUTTON_MIDDLE
};

// Define callback types outside the class
using KeyCallbackFn = std::function<void(Key key, int scancode, int action, int mods)>;
using MouseButtonCallbackFn = std::function<void(int button, int action, int mods)>;
using MouseMoveCallbackFn = std::function<void(float xpos, float ypos)>;
using MouseScrollCallbackFn = std::function<void(float xoffset, float yoffset)>;

class Input
{
    public:
        Input(std::shared_ptr<GameWindow> window);
        ~Input();

        // Key state queries
        static bool is_key_pressed(Key key);
        static bool was_key_pressed(Key key);
        static bool was_key_released(Key key);

        // Mouse button state queries
        static bool is_mouse_button_pressed(MouseButton button);
        static bool was_mouse_button_pressed(MouseButton button);
        static bool was_mouse_button_released(MouseButton button);

        // Mouse position and movement
        static Vector2 get_mouse_position();
        static Vector2 get_mouse_delta();
        static Vector2 get_mouse_scroll_delta();

        static void set_mouse_position(float x, float y);
        static void reset_mouse_delta();
        static void capture_mouse(bool capture);
        static void capture_mouse_reset_delta(bool capture);
        static bool is_mouse_captured();

        // Callback registration - need instance access
        void register_key_callback(KeyCallbackFn callback);
        void register_mouse_button_callback(MouseButtonCallbackFn callback);
        void register_mouse_move_callback(MouseMoveCallbackFn callback);
        void register_mouse_scroll_callback(MouseScrollCallbackFn callback);

        // Update state (call once per frame) - static since it updates global state
        static void update();

    private:
        // Static callbacks for GLFW
        static void key_callback(GLFWwindow *glfwWindow, Key key, int scancode, int action, int mods);
        static void mouse_button_callback(GLFWwindow *glfwWindow, int button, int action, int mods);
        static void mouse_move_callback(GLFWwindow *glfwWindow, float xpos, float ypos);
        static void scroll_callback(GLFWwindow *glfwWindow, float xoffset, float yoffset);
        static GLFWwindow *get_glfw_window() { return instance->glfwWindow; }

        static std::shared_ptr<GameWindow> get_window() { return instance->window; }

        // Static state storage
        static Input *instance;
        static std::unordered_map<Key, bool> currentKeyStates;
        static std::unordered_map<Key, bool> previousKeyStates;
        static std::unordered_map<int, bool> currentMouseButtonStates;
        static std::unordered_map<int, bool> previousMouseButtonStates;
        static float mouseX, mouseY;
        static float previousMouseX, previousMouseY;
        static float scrollDeltaX, scrollDeltaY;
        static bool mouseCaptured;

        // Instance members
        GLFWwindow *glfwWindow;
        std::shared_ptr<GameWindow> window;
        std::vector<KeyCallbackFn> keyCallbacks;
        std::vector<MouseButtonCallbackFn> mouseButtonCallbacks;
        std::vector<MouseMoveCallbackFn> mouseMoveCallbacks;
        std::vector<MouseScrollCallbackFn> mouseScrollCallbacks;
};



// Static member initialization
Input *Input::instance = nullptr;
std::unordered_map<Key, bool> Input::currentKeyStates;
std::unordered_map<Key, bool> Input::previousKeyStates;
std::unordered_map<int, bool> Input::currentMouseButtonStates;
std::unordered_map<int, bool> Input::previousMouseButtonStates;
float Input::mouseX = 0.0;
float Input::mouseY = 0.0;
float Input::previousMouseX = 0.0;
float Input::previousMouseY = 0.0;
float Input::scrollDeltaX = 0.0;
float Input::scrollDeltaY = 0.0;
bool Input::mouseCaptured = false;

Input::Input(std::shared_ptr<GameWindow> window) : glfwWindow(window->get_glfw_window()), window(window) // Update call
{
    if (instance != nullptr)
    {
        throw std::runtime_error("Multiple InputManager instances not allowed");
    }

    instance = this;

    glfwSetKeyCallback(glfwWindow, [](GLFWwindow * glfwWindow, int key, int scancode, int action, int mods) { key_callback(glfwWindow, static_cast<Key>(key), scancode, action, mods); });

    glfwSetMouseButtonCallback(glfwWindow, mouse_button_callback);
    glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow * glfwWindow, double xpos, double ypos) { mouse_move_callback(glfwWindow, static_cast<float>(xpos), static_cast<float>(ypos)); });

    glfwSetScrollCallback(glfwWindow, [](GLFWwindow * glfwWindow, double xoffset, double yoffset) { scroll_callback(glfwWindow, static_cast<float>(xoffset), static_cast<float>(yoffset)); });

    // Initialize mouse position
    double tempX, tempY;
    glfwGetCursorPos(glfwWindow, &tempX, &tempY);
    mouseX = tempX;
    mouseY = tempY;
    previousMouseX = mouseX;
    previousMouseY = mouseY;
}

Input::~Input()
{
    if (instance == this)
    {
        glfwSetKeyCallback(glfwWindow, nullptr);
        glfwSetMouseButtonCallback(glfwWindow, nullptr);
        glfwSetCursorPosCallback(glfwWindow, nullptr);
        glfwSetScrollCallback(glfwWindow, nullptr);
        instance = nullptr;
    }
}

void Input::update()
{
    if (!instance)
    {
        return;
    }

    // Update key states
    previousKeyStates = currentKeyStates;
    previousMouseButtonStates = currentMouseButtonStates;

    // Update mouse delta
    previousMouseX = mouseX;
    previousMouseY = mouseY;

    double tempX, tempY;
    glfwGetCursorPos(instance->glfwWindow, &tempX, &tempY);
    mouseX = tempX;
    mouseY = tempY;

    // Reset scroll delta
    scrollDeltaX = 0.0;
    scrollDeltaY = 0.0;
}

bool Input::is_key_pressed(Key key)
{
    auto it = currentKeyStates.find(key);
    return it != currentKeyStates.end() && it->second;
}

bool Input::was_key_pressed(Key key)
{
    auto curr = currentKeyStates.find(key);
    auto prev = previousKeyStates.find(key);
    return curr != currentKeyStates.end() && curr->second &&
           (prev == previousKeyStates.end() || !prev->second);
}

bool Input::was_key_released(Key key)
{
    auto curr = currentKeyStates.find(key);
    auto prev = previousKeyStates.find(key);
    return (curr == currentKeyStates.end() || !curr->second) &&
           prev != previousKeyStates.end() && prev->second;
}

bool Input::is_mouse_button_pressed(MouseButton button)
{
    auto it = currentMouseButtonStates.find(static_cast<int>(button));
    return it != currentMouseButtonStates.end() && it->second;
}

bool Input::was_mouse_button_pressed(MouseButton button)
{
    auto curr = currentMouseButtonStates.find(static_cast<int>(button));
    auto prev = previousMouseButtonStates.find(static_cast<int>(button));
    return curr != currentMouseButtonStates.end() && curr->second &&
           (prev == previousMouseButtonStates.end() || !prev->second);
}

bool Input::was_mouse_button_released(MouseButton button)
{
    auto curr = currentMouseButtonStates.find(static_cast<int>(button));
    auto prev = previousMouseButtonStates.find(static_cast<int>(button));
    return (curr == currentMouseButtonStates.end() || !curr->second) &&
           prev != previousMouseButtonStates.end() && prev->second;
}

Vector2 Input::get_mouse_position()
{
    return Vector2(mouseX, mouseY);
}

Vector2 Input::get_mouse_delta()
{
    return Vector2(mouseX - previousMouseX, mouseY - previousMouseY);
}

Vector2 Input::get_mouse_scroll_delta()
{
    return Vector2(scrollDeltaX, scrollDeltaY);
}

void Input::set_mouse_position(float x, float y)
{
    mouseX = x;
    mouseY = y;
    previousMouseX = x;
    previousMouseY = y;
    glfwSetCursorPos(get_glfw_window(), x, y);
}

void Input::reset_mouse_delta()
{
    previousMouseX = mouseX;
    previousMouseY = mouseY;
}

void Input::capture_mouse(bool capture)
{
    set_mouse_position(get_window()->get_width() / 2.0f, get_window()->get_height() / 2.0f); // Update calls
    glfwSetInputMode(get_glfw_window(), GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetInputMode(get_glfw_window(), GLFW_RAW_MOUSE_MOTION, capture);

    mouseCaptured = capture;
}

/// @brief Sets the mouse capture and also resets the input mouse delta so that systems relying on delta values will not be affected by the sudden change in mouse position.
/// @param capture Whether to capture the mouse or not
void Input::capture_mouse_reset_delta(bool capture)
{
    capture_mouse(capture);
    Input::reset_mouse_delta();
}

bool Input::is_mouse_captured()
{
    return mouseCaptured;
}

void Input::register_key_callback(KeyCallbackFn callback)
{
    keyCallbacks.push_back(callback);
}

void Input::register_mouse_button_callback(MouseButtonCallbackFn callback)
{
    mouseButtonCallbacks.push_back(callback);
}

void Input::register_mouse_move_callback(MouseMoveCallbackFn callback)
{
    mouseMoveCallbacks.push_back(callback);
}

void Input::register_mouse_scroll_callback(MouseScrollCallbackFn callback)
{
    mouseScrollCallbacks.push_back(callback);
}

// Static callback implementations
void Input::key_callback(GLFWwindow *glfwWindow, Key key, int scancode, int action, int mods)
{
    if (instance)
    {
        currentKeyStates[key] = (action != GLFW_RELEASE);

        for (const auto &callback : instance->keyCallbacks)
        {
            callback(key, scancode, action, mods);
        }
    }
}

void Input::mouse_button_callback(GLFWwindow *glfwWindow, int button, int action, int mods)
{
    if (instance)
    {
        currentMouseButtonStates[button] = (action != GLFW_RELEASE);

        for (const auto &callback : instance->mouseButtonCallbacks)
        {
            callback(button, action, mods);
        }
    }
}

void Input::mouse_move_callback(GLFWwindow *glfwWindow, float xpos, float ypos)
{
    if (instance)
    {
        mouseX = xpos;
        mouseY = ypos;

        for (const auto &callback : instance->mouseMoveCallbacks)
        {
            callback(xpos, ypos);
        }
    }
}

void Input::scroll_callback(GLFWwindow *glfwWindow, float xoffset, float yoffset)
{
    if (instance)
    {
        scrollDeltaX += xoffset;
        scrollDeltaY += yoffset;

        for (const auto &callback : instance->mouseScrollCallbacks)
        {
            callback(xoffset, yoffset);
        }
    }
}
