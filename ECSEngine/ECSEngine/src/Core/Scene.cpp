#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Components/RenderComponent.h"
#include "Components/CameraBaseComponent.h"
#include "Core/FontRenderer.h"
#include "Core/Shader.h"
#include "Core/AssetManager.h"
#include <iostream>
#include <algorithm>
#include <filesystem> 
#include <fstream>

Scene::Scene(const std::string& name)
    : m_name(name),
    m_activeCamera(nullptr),
    m_windowWidth(800),
    m_windowHeight(600)
{
    std::cout << "Scene '" << m_name << "' created." << std::endl;
}

Scene::~Scene() {
    Shutdown();
    std::cout << "Scene '" << m_name << "' destroyed." << std::endl;
}

void Scene::Init() {
    std::cout << "Initializing Scene '" << m_name << "'..." << std::endl;

    m_fontRenderer = std::make_shared<FontRenderer>();
    if (!m_fontRenderer->init(m_windowWidth, m_windowHeight)) { 
        std::cerr << "ERROR: Failed to initialize FontRenderer for scene '" << m_name << "'!" << std::endl;
    }
    if (!m_fontRenderer->loadFont("res/fonts/Roboto-Regular.ttf", 48)) {
        std::cerr << "ERROR: Failed to load font 'res/fonts/Roboto-Regular.ttf' for scene '" << m_name << "'!" << std::endl;
    }

}

void Scene::Update(float deltaTime) {
    for (const auto& gameObject : m_gameObjects) {
        if (gameObject) {
            gameObject->Update(deltaTime);
        }
    }

    PickingManager::getInstance().Update(deltaTime, this);
}

void Scene::Render() {
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);

    if (m_activeCamera) {
        projectionMatrix = m_activeCamera->getProjectionMatrix();
        viewMatrix = m_activeCamera->getViewMatrix();
    }
    else {
        std::cerr << "WARNING: No active camera set for scene '" << m_name << "'. Rendering with identity matrices." << std::endl;
    }

    for (const auto& gameObject : m_gameObjects) {
        if (gameObject) {
            gameObject->Render(viewMatrix, projectionMatrix); 
        }
    }
}

void Scene::Shutdown() {
    std::cout << "Shutting down Scene '" << m_name << "'..." << std::endl;
    m_gameObjects.clear();
    m_activeCamera = nullptr;
}

GameObject* Scene::AddGameObject(std::unique_ptr<GameObject> gameObject) {
    if (!gameObject) {
        std::cerr << "ERROR: Attempted to add a null GameObject to scene '" << m_name << "'." << std::endl;
        return nullptr;
    }
    GameObject* rawPtr = gameObject.get();
    m_gameObjects.push_back(std::move(gameObject));
    return rawPtr;
}

void Scene::RemoveGameObject(GameObject* gameObject) {
    auto it = std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
        [gameObject](const std::unique_ptr<GameObject>& ptr) {
            return ptr.get() == gameObject;
        });
    if (it != m_gameObjects.end()) {
        m_gameObjects.erase(it, m_gameObjects.end());
        std::cout << "Removed GameObject by pointer from scene '" << m_name << "'." << std::endl;
    }
}

void Scene::RemoveGameObjectByName(const std::string& name) {
    auto it = std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
        [name](const std::unique_ptr<GameObject>& ptr) {
            return ptr->getName() == name;
        });
    if (it != m_gameObjects.end()) {
        m_gameObjects.erase(it, m_gameObjects.end());
        std::cout << "Removed GameObject '" << name << "' by name from scene '" << m_name << "'." << std::endl;
    }
}

void Scene::setActiveCamera(CameraBaseComponent* camera) {
    m_activeCamera = camera;
    std::cout << "Active camera set for scene '" << m_name << "'." << std::endl;
}

void Scene::setWindowDimensions(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
}