#include "TowerGameScene.h"
#include "Core/GameObject.h"
#include "Input/InputManager.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Components/Camera3DComponent.h"
#include "Components/CameraBaseComponent.h"
#include "Core/FontRenderer.h"
#include "Core/AssetManager.h"
#include "Core/Shader.h"
#include "Core/Texture.h"
#include "Core/Mesh.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

TowerGameScene::TowerGameScene(const std::string& name)
    : Scene(name),
    m_towerGameObject(nullptr),
    m_currentTowerHeight(0.0f)
{
    std::cout << "TowerGameScene '" << m_name << "' created." << std::endl;
}

TowerGameScene::~TowerGameScene() {
    std::cout << "TowerGameScene '" << m_name << "' destroyed." << std::endl;
}

void TowerGameScene::Init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    Scene::Init();
    std::cout << "Initializing TowerGameScene '" << m_name << "'..." << std::endl;

    SetupTowerGameObjects();

    std::cout << "TowerGameScene '" << m_name << "' initialized with renderable objects." << std::endl;
}

void TowerGameScene::Update(float deltaTime) {
    Scene::Update(deltaTime);


    if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_W)) {
        if (m_towerGameObject) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<> scale_dist(0.3, 1.6);
            static std::uniform_real_distribution<> color_dist(0.2, 1.0);

            float newCubeScale = static_cast<float>(scale_dist(gen));
            glm::vec4 newCubeColor = glm::vec4(color_dist(gen), color_dist(gen), color_dist(gen), 1.0f);

            auto newCube = std::make_unique<GameObject>("TowerCube");
            newCube->getTransform()->setLocalScale(glm::vec3(newCubeScale));
            newCube->getTransform()->setLocalPosition(glm::vec3(0.0f, m_currentTowerHeight + (newCubeScale / 2.0f), 0.0f));

            MeshComponent* meshComp = newCube->addComponent<MeshComponent>();
            std::shared_ptr<Shader> cubeShader = AssetManager::getInstance().getShader("res/shaders/basic.vert", "res/shaders/basic.frag");
            RenderComponent* renderComp = newCube->addComponent<RenderComponent>(cubeShader);

            if (meshComp && meshComp->getMesh()) {
                renderComp->setMesh(meshComp->getMesh());
            }
            else {
                std::cerr << "ERROR: Failed to add MeshComponent or get mesh from newCube! RenderComponent might not have a mesh." << std::endl;
            }

            std::shared_ptr<Texture> cubeTexture = AssetManager::getInstance().getTexture("res/textures/wall.png", "diffuse");
            renderComp->setTexture(cubeTexture);
            renderComp->setObjectColor(newCubeColor);

            m_towerGameObject->addChild(std::move(newCube));
            m_currentTowerHeight += newCubeScale;
            std::cout << "Added cube. New tower height: " << m_towerGameObject->getChildren().size() << " cubes, " << m_currentTowerHeight << " m" << std::endl;
        }
    }
  

    if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_S)) {
        if (m_towerGameObject && !m_towerGameObject->getChildren().empty()) {
            const std::unique_ptr<GameObject>& topCube = m_towerGameObject->getChildren().back();
            float topCubeScale = topCube->getTransform()->getLocalScale().y;

            m_towerGameObject->removeLastChild();
            m_currentTowerHeight -= topCubeScale;
            if (m_currentTowerHeight < 0) m_currentTowerHeight = 0;
            std::cout << "Removed cube. New tower height: " << m_towerGameObject->getChildren().size() << " cubes, " << m_currentTowerHeight << " m" << std::endl;
        }
        else {
            std::cout << "Tower is empty! Cannot remove cube." << std::endl;
        }
    }

    if (m_towerGameObject && m_towerGameObject->getTransform()) {
        TransformComponent* towerTransform = m_towerGameObject->getTransform();
        float rotationSpeed = glm::radians(90.0f) * deltaTime;

        if (InputManager::getInstance().isKeyPressed(GLFW_KEY_Q)) {
            towerTransform->rotate(glm::vec3(0.0f, rotationSpeed, 0.0f));
        }
        if (InputManager::getInstance().isKeyPressed(GLFW_KEY_E)) {
            towerTransform->rotate(glm::vec3(0.0f, -rotationSpeed, 0.0f));
        }
    }

    if (m_activeCamera && m_towerGameObject) {
        Camera3DComponent* active3DCamera = dynamic_cast<Camera3DComponent*>(m_activeCamera);

        if (active3DCamera) {
            glm::vec3 towerTopPosition = glm::vec3(0.0f, m_currentTowerHeight + 1.0f, 0.0f);
            glm::vec3 cameraEyePosition = towerTopPosition + glm::vec3(0.0f, 7.0f, 15.0f);

            active3DCamera->getOwner()->getTransform()->setLocalPosition(cameraEyePosition);

            active3DCamera->setLookAtTarget(towerTopPosition);
            float scrollY = InputManager::getInstance().getScrollYOffset();
            active3DCamera->applyFovZoom(scrollY);
        }
        else {
            std::cerr << "WARNING: Active camera is not a 3D CameraComponent in TowerGameScene. Update cannot apply 3D-specific camera logic." << std::endl;
        }
    }
}

void TowerGameScene::Render() {
    Scene::Render();

    if (m_fontRenderer) {
        std::string nameText = "Milan Arežina, SV55/2021";
        float nameTextScale = 0.3f;
        float estimatedCharWidth = 48.0f * 0.6f; 
        float textPixelWidth = nameText.length() * estimatedCharWidth * nameTextScale;
        float padding = 10.0f;
        float nameX = 800.0f - textPixelWidth - padding; 
        float nameY = padding; 
        m_fontRenderer->renderText(nameText, nameX, nameY, nameTextScale, glm::vec3(1.0f, 0.5f, 0.2f));
        size_t numCubes = m_towerGameObject ? m_towerGameObject->getChildren().size() : 0;
        std::string heightText = "Tower height: " + std::to_string(numCubes) + " cubes, " +
            std::to_string(m_currentTowerHeight) + " m";

        m_fontRenderer->renderText(heightText, 20, getWindowHeight() - 20, nameTextScale, glm::vec3(0.2f, 0.8f, 1.0f));

        float controlTextScale = 0.3f;
        float controlPaddingX = 10.0f;
        float controlPaddingY = 10.0f;
        float lineHeight = 48.0f * controlTextScale * 1.2f; 

        float currentY = 10; 
        glm::vec3 controlsColor = glm::vec3(0.0f, 1.0f, 1.0f);

        m_fontRenderer->renderText("Rotate tower -> Q/E", controlPaddingX, currentY, controlTextScale, controlsColor);


        currentY += lineHeight;
        m_fontRenderer->renderText("Remove layer -> S", controlPaddingX, currentY, controlTextScale, controlsColor);


        currentY += lineHeight;

        m_fontRenderer->renderText("Add layer -> W", controlPaddingX, currentY, controlTextScale, controlsColor);

        currentY += lineHeight;
        m_fontRenderer->renderText("Keyboard controls", controlPaddingX, currentY, controlTextScale, controlsColor);




    }
}

void TowerGameScene::Shutdown() {
    std::cout << "Shutting down TowerGameScene '" << m_name << "'..." << std::endl;
    m_towerGameObject = nullptr;
    Scene::Shutdown();
    std::cout << "TowerGameScene '" << m_name << "' shutdown complete." << std::endl;
}

void TowerGameScene::SetupTowerGameObjects() {
    std::shared_ptr<Shader> basicShader = AssetManager::getInstance().getShader("res/shaders/basic.vert", "res/shaders/basic.frag");
    std::shared_ptr<Texture> wallTexture = AssetManager::getInstance().getTexture("res/textures/wall.png", "diffuse");
    std::shared_ptr<Texture> grassTexture = AssetManager::getInstance().getTexture("res/textures/grass.png", "diffuse");

    auto cameraObject = std::make_unique<GameObject>("MainCamera");
    CameraBaseComponent* cameraComp = cameraObject->addComponent<Camera3DComponent>(
        45.0f,
        static_cast<float>(getWindowWidth()) / getWindowHeight(),
        0.1f,
        100.0f
    );
    AddGameObject(std::move(cameraObject));
    setActiveCamera(cameraComp);

    auto towerObject = std::make_unique<GameObject>("Tower");
    towerObject->getTransform()->setLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    m_towerGameObject = AddGameObject(std::move(towerObject));

    auto groundPlane = std::make_unique<GameObject>("GroundPlane");
    MeshComponent* groundMeshComp = groundPlane->addComponent<MeshComponent>();

    if (groundMeshComp && groundMeshComp->getMesh()) {
        groundMeshComp->getMesh()->generatePlane(50.0f);
    }
    else {
        std::cerr << "ERROR: Failed to add MeshComponent or get mesh from GroundPlane!" << std::endl;
    }

    RenderComponent* groundRenderComp = groundPlane->addComponent<RenderComponent>(basicShader);

    if (groundRenderComp) {
        if (groundMeshComp) groundRenderComp->setMesh(groundMeshComp->getMesh());
        groundRenderComp->setTexture(grassTexture);
        groundRenderComp->setObjectColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    }
    else {
        std::cerr << "ERROR: Failed to setup GroundPlane render component." << std::endl;
    }

    groundPlane->getTransform()->setLocalPosition(glm::vec3(0.0f, -0.5f, 0.0f));
    groundPlane->getTransform()->setLocalScale(glm::vec3(100.0f, 1.0f, 100.0f));

    AddGameObject(std::move(groundPlane));
}