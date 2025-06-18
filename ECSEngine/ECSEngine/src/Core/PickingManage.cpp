#include "PickingManager.h"
#include "Input/InputManager.h"            
#include "Components/ClickableComponent.h" 
#include "Components/TransformComponent.h" 
#include "Components/MeshComponent.h"      
#include "Components/CameraBaseComponent.h"    
#include "Components/Camera2DComponent.h" 
#include "Components/Camera3DComponent.h"  
#include "Core/GameObject.h"               
#include "Core/Scene.h"                    

#include <iostream>
#include <algorithm>
#include <limits>    
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/matrix_inverse.hpp>   
#include <glm/gtx/component_wise.hpp>   
#include <glm/gtx/string_cast.hpp> 

PickingManager& PickingManager::getInstance() {
    static PickingManager instance;
    return instance;
}

PickingManager::PickingManager() {
}

PickingManager::~PickingManager() {
    m_clickableComponents.clear(); 
}

void PickingManager::Init(int windowWidth, int windowHeight) {
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    std::cout << "PickingManager initialized with window size " << windowWidth << "x" << windowHeight << std::endl;
}

void PickingManager::Update(float deltaTime, Scene* activeScene) {
    if (!activeScene) {
        std::cerr << "PickingManager: No active scene to pick from." << std::endl;
        return;
    }

    CameraBaseComponent* activeCamera = activeScene->getActiveCamera();
    if (!activeCamera) {
        return;
    }

    glm::vec2 mouseScreenPos = glm::vec2(InputManager::getInstance().getMouseX(), InputManager::getInstance().getMouseY());

    ClickableComponent* newHoveredComponent = nullptr;
    float closest3D_T = std::numeric_limits<float>::max();

    
    for (auto it = m_clickableComponents.rbegin(); it != m_clickableComponents.rend(); ++it) {
        ClickableComponent* clickable = *it;
        if (!clickable || !clickable->getOwner() || !clickable->getOwner()->getTransform()) {
            continue;
        }

        if (clickable->getPickingMethod() == PickingMethod::Method2D) {
            Camera2DComponent* cam2d = dynamic_cast<Camera2DComponent*>(activeCamera);
            if (cam2d) { 
                glm::vec2 mouseWorldPos2D = screenToWorld2D(mouseScreenPos, cam2d);
                if (isMouseOver2D(clickable->getOwner(), mouseWorldPos2D)) {
                    newHoveredComponent = clickable; 
                    break; 
                }
            }
        }
    }

    if (newHoveredComponent == nullptr) {
        Camera3DComponent* cam3d = dynamic_cast<Camera3DComponent*>(activeCamera);
        if (cam3d) {
            Ray pickRay = screenToWorldRay3D(mouseScreenPos, cam3d);

            for (ClickableComponent* clickable : m_clickableComponents) {
                if (!clickable || !clickable->getOwner() || !clickable->getOwner()->getTransform()) {
                    continue;
                }

                if (clickable->getPickingMethod() == PickingMethod::Method3D) {
                    float current_t;
                    if (rayIntersectsAABB(pickRay, clickable->getOwner(), current_t)) {
                        if (current_t < closest3D_T) {
                            closest3D_T = current_t;
                            newHoveredComponent = clickable; 
                        }
                    }
                }
            }
        }
    }


    if (m_hoveredComponent != newHoveredComponent) {
        if (m_hoveredComponent) {
            m_hoveredComponent->onHoverExit();
        }
        if (newHoveredComponent) {
            newHoveredComponent->onHoverEnter();
        }
        m_hoveredComponent = newHoveredComponent;
    }

    if (InputManager::getInstance().isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        if (m_hoveredComponent) { 
            m_hoveredComponent->onClick();

        }
    }
}

void PickingManager::AddClickable(ClickableComponent* clickable) {
    if (clickable) {
        if (std::find(m_clickableComponents.begin(), m_clickableComponents.end(), clickable) == m_clickableComponents.end()) {
            m_clickableComponents.push_back(clickable);
        }
        else {
        }
    }
}

void PickingManager::RemoveClickable(ClickableComponent* clickable) {
    if (clickable) {
        auto it = std::remove(m_clickableComponents.begin(), m_clickableComponents.end(), clickable);
        if (it != m_clickableComponents.end()) {
            m_clickableComponents.erase(it, m_clickableComponents.end());
            if (m_hoveredComponent == clickable) {
                m_hoveredComponent->onHoverExit(); 
                m_hoveredComponent = nullptr;
            }
        }
        else {
        }
    }
}


glm::vec2 PickingManager::screenToWorld2D(const glm::vec2& screenCoords, Camera2DComponent* camera) {
    if (!camera || !camera->getOwner() || !camera->getOwner()->getTransform()) {
        std::cerr << "PickingManager ERROR: Camera2DComponent or its transform is null in screenToWorld2D." << std::endl;
        return screenCoords; 
    }

    glm::mat4 viewMatrix = camera->getViewMatrix();
    glm::mat4 projectionMatrix = camera->getProjectionMatrix(); 

    glm::mat4 inverseVP = glm::inverse(projectionMatrix * viewMatrix);

    float ndcX = (screenCoords.x / static_cast<float>(m_windowWidth)) * 2.0f - 1.0f;
    float ndcY = 1.0f - (screenCoords.y / static_cast<float>(m_windowHeight)) * 2.0f;

    glm::vec4 worldPos = inverseVP * glm::vec4(ndcX, ndcY, 0.0f, 1.0f); 
    return glm::vec2(worldPos.x / worldPos.w, worldPos.y / worldPos.w);
}

bool PickingManager::isMouseOver2D(GameObject* gameObject, const glm::vec2& mouseWorldPos2D) {
    if (!gameObject || !gameObject->getTransform()) {
        return false;
    }

    glm::mat4 worldMatrix = gameObject->getTransform()->getWorldMatrix();

    glm::vec2 objectWorldPos = glm::vec2(worldMatrix[3].x, worldMatrix[3].y);

    float objectWorldWidth = glm::length(glm::vec2(worldMatrix[0])); 
    float objectWorldHeight = glm::length(glm::vec2(worldMatrix[1])); 

    float halfWidth = objectWorldWidth * 0.5f;
    float halfHeight = objectWorldHeight * 0.5f;

    float minX = objectWorldPos.x - halfWidth;
    float maxX = objectWorldPos.x + halfWidth;
    float minY = objectWorldPos.y - halfHeight;
    float maxY = objectWorldPos.y + halfHeight;

    return (mouseWorldPos2D.x >= minX && mouseWorldPos2D.x <= maxX &&
        mouseWorldPos2D.y >= minY && mouseWorldPos2D.y <= maxY);
}


PickingManager::Ray PickingManager::screenToWorldRay3D(const glm::vec2& screenCoords, Camera3DComponent* camera) {
    if (!camera || !camera->getOwner() || !camera->getOwner()->getTransform()) {
        std::cerr << "PickingManager ERROR: CameraComponent or its transform is null in screenToWorldRay3D." << std::endl;
        return { glm::vec3(0), glm::vec3(0) }; 
    }

    float ndcX = (2.0f * screenCoords.x) / static_cast<float>(m_windowWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenCoords.y) / static_cast<float>(m_windowHeight);

    glm::vec4 rayClipNear(ndcX, ndcY, -1.0f, 1.0f); 
    glm::vec4 rayClipFar(ndcX, ndcY, 1.0f, 1.0f);   

    glm::mat4 inverseProjection = glm::inverse(camera->getProjectionMatrix());
    glm::vec4 rayEyeNear = inverseProjection * rayClipNear;
    glm::vec4 rayEyeFar = inverseProjection * rayClipFar;

    if (rayEyeNear.w != 0.0f) rayEyeNear /= rayEyeNear.w;
    if (rayEyeFar.w != 0.0f) rayEyeFar /= rayEyeFar.w;

    glm::mat4 cameraWorldMatrix = camera->getOwner()->getTransform()->getWorldMatrix();
    glm::vec3 rayWorldNear = glm::vec3(cameraWorldMatrix * rayEyeNear);
    glm::vec3 rayWorldFar = glm::vec3(cameraWorldMatrix * rayEyeFar);

    glm::vec3 rayOrigin = rayWorldNear;
    glm::vec3 rayDirection = glm::normalize(rayWorldFar - rayWorldNear);

    return { rayOrigin, rayDirection };
}

void PickingManager::getGameObjectWorldAABB(GameObject* obj, glm::vec3& outMin, glm::vec3& outMax) {
    if (!obj || !obj->getTransform()) {
        outMin = glm::vec3(0.0f);
        outMax = glm::vec3(0.0f);
        std::cerr << "WARNING: GameObject " << (obj ? obj->getName() : "nullptr") << " has no TransformComponent for AABB calculation." << std::endl;
        return;
    }
    obj->getTransform()->calculateWorldAABB(outMin, outMax);
}

bool PickingManager::rayIntersectsAABB(const Ray& ray, GameObject* gameObject, float& outT) {
    if (!gameObject || !gameObject->getTransform()) {
        return false;
    }

    glm::vec3 aabbMin, aabbMax;
    getGameObjectWorldAABB(gameObject, aabbMin, aabbMax); 

    glm::vec3 invDir = 1.0f / ray.direction;
    glm::vec3 tMin = (aabbMin - ray.origin) * invDir;
    glm::vec3 tMax = (aabbMax - ray.origin) * invDir;

    glm::vec3 realTMin = glm::min(tMin, tMax);
    glm::vec3 realTMax = glm::max(tMin, tMax);

    float tEnter = glm::compMax(realTMin); 
    float tExit = glm::compMin(realTMax);   

    if (tEnter > tExit || tExit < 0.0f) {
        return false; 
    }

    outT = tEnter; 
    return true;
}