#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include "Core/Scene.h"

class Application {
public:
    static Application& getInstance();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool init(unsigned int windowWidth, unsigned int windowHeight, const std::string& title);
    void run();

private:
    Application();
    ~Application();

    bool initializeGLFW();
    bool createWindow(unsigned int width, unsigned int height, const std::string& title);
    bool initializeGLEW();

    GLFWwindow* m_window;
    unsigned int m_windowWidth, m_windowHeight;
    std::string m_windowTitle;

    float m_deltaTime;
    float m_lastFrame;

    std::unique_ptr<Scene> m_gameScene;

    static Application* s_instance;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void onFramebufferSize(int width, int height);
};