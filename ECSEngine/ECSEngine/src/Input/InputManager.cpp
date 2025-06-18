#include "Input/InputManager.h"
#include <iostream>

InputManager::InputManager()
    : m_mouseX(0.0), m_mouseY(0.0),
    m_lastMouseX(0.0), m_lastMouseY(0.0),
    m_mouseDeltaX(0.0), m_mouseDeltaY(0.0),
    m_scrollYOffset(0.0f),
    m_firstMouseMove(true) 
{

}

InputManager::~InputManager() {

}

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

void InputManager::initialize(GLFWwindow* window) {
    if (!window) {
        std::cerr << "ERROR: InputManager::initialize - Provided GLFWwindow* is null." << std::endl;
        return;
    }
    m_window = window;

    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    m_mouseX = x;
    m_mouseY = y;
    m_lastMouseX = x;
    m_lastMouseY = y;
    m_firstMouseMove = false;

    glfwSetKeyCallback(m_window, InputManager::glfw_key_callback);
    glfwSetMouseButtonCallback(m_window, InputManager::glfw_mouse_button_callback);
    glfwSetCursorPosCallback(m_window, InputManager::glfw_mouse_position_callback);
    glfwSetScrollCallback(m_window, InputManager::glfw_mouse_scroll_callback);

    std::cout << "InputManager initialized with GLFW callbacks." << std::endl;
}

void InputManager::update() {
    m_justPressedKeys.clear();
    m_justReleasedKeys.clear();
    m_justPressedMouseButtons.clear();
    m_justReleasedMouseButtons.clear();

    m_scrollYOffset = 0.0f;

 
    m_mouseDeltaX = m_mouseX - m_lastMouseX;
    m_mouseDeltaY = m_lastMouseY - m_mouseY;


    m_lastMouseX = m_mouseX;
    m_lastMouseY = m_mouseY;

}

bool InputManager::isKeyPressed(int key) const {
    auto it = m_keys.find(key);
    return (it != m_keys.end() && it->second);
}

bool InputManager::isKeyJustPressed(int key) const {
    return m_justPressedKeys.count(key) > 0;
}

bool InputManager::isKeyReleased(int key) const {
    return m_justReleasedKeys.count(key) > 0;
}

bool InputManager::isMouseButtonPressed(int button) const {
    auto it = m_mouseButtons.find(button);
    return (it != m_mouseButtons.end() && it->second);
}

bool InputManager::isMouseButtonJustPressed(int button) const {
    return m_justPressedMouseButtons.count(button) > 0;
}

bool InputManager::isMouseButtonReleased(int button) const {
    return m_justReleasedMouseButtons.count(button) > 0;
}

float InputManager::getScrollYOffset() {
    return m_scrollYOffset;
}


void InputManager::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputManager& instance = InputManager::getInstance();

    if (action == GLFW_PRESS) {
        if (!instance.m_keys.count(key) || !instance.m_keys[key]) {
            instance.m_justPressedKeys.insert(key);
        }
        instance.m_keys[key] = true;
        instance.m_justReleasedKeys.erase(key);
    }
    else if (action == GLFW_RELEASE) {
        if (instance.m_keys.count(key) && instance.m_keys[key]) {
            instance.m_justReleasedKeys.insert(key);
        }
        instance.m_keys[key] = false;
        instance.m_justPressedKeys.erase(key);
    }
}

void InputManager::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    InputManager& instance = InputManager::getInstance();

    if (action == GLFW_PRESS) {
        if (!instance.m_mouseButtons.count(button) || !instance.m_mouseButtons[button]) {
            instance.m_justPressedMouseButtons.insert(button);
        }
        instance.m_mouseButtons[button] = true;
        instance.m_justReleasedMouseButtons.erase(button);
    }
    else if (action == GLFW_RELEASE) {
        if (instance.m_mouseButtons.count(button) && instance.m_mouseButtons[button]) {
            instance.m_justReleasedMouseButtons.insert(button);
        }
        instance.m_mouseButtons[button] = false;
        instance.m_justPressedMouseButtons.erase(button);
    }
}

void InputManager::glfw_mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    InputManager& instance = InputManager::getInstance();

    instance.m_mouseX = xpos;
    instance.m_mouseY = ypos;
}

void InputManager::glfw_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    InputManager& instance = InputManager::getInstance();
    instance.m_scrollYOffset += static_cast<float>(yoffset); 
}