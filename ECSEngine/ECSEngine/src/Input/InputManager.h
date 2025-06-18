#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <map>
#include <set>

class InputManager {
public:
    static InputManager& getInstance();
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void initialize(GLFWwindow* window);
    void update();

    bool isKeyPressed(int key) const;
    bool isKeyJustPressed(int key) const;
    bool isKeyReleased(int key) const;

    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonJustPressed(int button) const;
    bool isMouseButtonReleased(int button) const;
    double getMouseX() const { return m_mouseX; }
    double getMouseY() const { return m_mouseY; }
    double getMouseDeltaX() const { return m_mouseDeltaX; }
    double getMouseDeltaY() const { return m_mouseDeltaY; }
    float getScrollYOffset();

private:
    InputManager();
    ~InputManager();

    GLFWwindow* m_window = nullptr;

    std::map<int, bool> m_keys;
    std::map<int, bool> m_mouseButtons;


    std::set<int> m_justPressedKeys;
    std::set<int> m_justReleasedKeys; 
    std::set<int> m_justPressedMouseButtons;
    std::set<int> m_justReleasedMouseButtons;

    double m_mouseX, m_mouseY;
    double m_lastMouseX, m_lastMouseY;
    double m_mouseDeltaX, m_mouseDeltaY;
    float m_scrollYOffset;
    bool m_firstMouseMove = true;

    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void glfw_mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void glfw_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};