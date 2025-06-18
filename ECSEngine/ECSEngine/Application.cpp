#include "Application.h"
#include <iostream>
#include <Core/TowerGameScene.h>
#include <Core/MicrowaveGameScene.h>
#include "Input/InputManager.h"
#include <glm/ext/matrix_clip_space.hpp>


void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

Application* Application::s_instance = nullptr;

Application::Application()
    : m_window(nullptr),
    m_windowWidth(0),
    m_windowHeight(0),
    m_windowTitle(""),
    m_deltaTime(0.0f),
    m_lastFrame(0.0f),
    m_gameScene(nullptr)
{
}

Application::~Application() {
    if (m_gameScene) {
        m_gameScene->Shutdown();
    }
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
    glfwSetErrorCallback(nullptr);
}

Application& Application::getInstance() {
    if (s_instance == nullptr) {
        s_instance = new Application();
        atexit([]() { delete s_instance; s_instance = nullptr; });
    }
    return *s_instance;
}

bool Application::initializeGLFW() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    return true;
}

bool Application::createWindow(unsigned int width, unsigned int height, const std::string& title) {
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    return true;
}

bool Application::initializeGLEW() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return false;
    }
    std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    int major_version = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MAJOR);
    int minor_version = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MINOR);
    int profile = glfwGetWindowAttrib(m_window, GLFW_OPENGL_PROFILE);
    std::cout << "Actual OpenGL Version: " << major_version << "." << minor_version << std::endl;
    std::cout << "Actual OpenGL Profile: " << (profile == GLFW_OPENGL_CORE_PROFILE ? "Core Profile" : "Compatibility Profile") << std::endl;
    return true;
}

bool Application::init(unsigned int windowWidth, unsigned int windowHeight, const std::string& title) {
    this->m_windowWidth = windowWidth;
    this->m_windowHeight = windowHeight;
    this->m_windowTitle = title;

    int choice = 0;
    bool validChoice = false;
    while (!validChoice) {
        std::cout << "\n--- Select a Scene ---" << std::endl;
        std::cout << "1. Tower Game Scene" << std::endl;
        std::cout << "2. Microwave Game Scene" << std::endl;
        std::cout << "Enter your choice (1 or 2): ";

        std::cin >> choice;

        if (std::cin.fail() || (choice != 1 && choice != 2)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter 1 or 2." << std::endl;
        }
        else {
            validChoice = true;
        }
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice) {
    case 1:
        m_gameScene = std::make_unique<TowerGameScene>("Tower Game Scene");
        std::cout << "Loading Tower Game Scene..." << std::endl;
        break;
    case 2:
        m_gameScene = std::make_unique<MicrowaveGameScene>("Microwave Game Scene");
        std::cout << "Loading Microwave Game Scene..." << std::endl;
        break;
    default:
        std::cerr << "Unexpected choice, exiting." << std::endl;
        return false;
    }

    if (!m_gameScene) {
        std::cerr << "ERROR: Failed to create game scene." << std::endl;
        return false;
    }


    if (!initializeGLFW()) return false;
    if (!createWindow(m_windowWidth, m_windowHeight, m_windowTitle)) return false;
    if (!initializeGLEW()) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    InputManager::getInstance().initialize(m_window);
    PickingManager::getInstance().Init(m_windowWidth, m_windowHeight);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(0, 0, m_windowWidth, m_windowHeight);

    

    m_gameScene->setWindowDimensions(m_windowWidth, m_windowHeight);
    m_gameScene->Init();

    std::cout << "Application initialized successfully. GameObjects and transforms are ready." << std::endl;
    return true;
}

void Application::run() {
    while (!glfwWindowShouldClose(m_window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        m_deltaTime = currentFrame - m_lastFrame;
        m_lastFrame = currentFrame;

        InputManager::getInstance().update();
        PickingManager::getInstance().Update(m_deltaTime, m_gameScene.get());
        glfwPollEvents();

        if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            std::cout << "ESCAPE key was pressed. Closing window." << std::endl;
            glfwSetWindowShouldClose(m_window, true);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_gameScene) {
            m_gameScene->Update(m_deltaTime);
            m_gameScene->Render();
        }

        glfwSwapBuffers(m_window);
    }
}

void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) app->onFramebufferSize(width, height);
}

void Application::onFramebufferSize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    glViewport(0, 0, m_windowWidth, m_windowHeight);
    std::cout << "Framebuffer Resized to: " << width << "x" << height << std::endl;
}