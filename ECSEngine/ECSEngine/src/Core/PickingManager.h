#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <limits> 

class ClickableComponent;
class CameraComponent;  
class Camera2DComponent; 
class Camera3DComponent;
class TransformComponent;
class MeshComponent;
class GameObject;
class Scene;             

class PickingManager {
public:
    static PickingManager& getInstance();

    PickingManager(const PickingManager&) = delete;
    PickingManager& operator=(const PickingManager&) = delete;

    void Init(int windowWidth, int windowHeight);

    void Update(float deltaTime, Scene* activeScene);

    void AddClickable(ClickableComponent* clickable);

    void RemoveClickable(ClickableComponent* clickable);

private:
    PickingManager();
    ~PickingManager();

    std::vector<ClickableComponent*> m_clickableComponents;

    int m_windowWidth = 0;
    int m_windowHeight = 0;

    ClickableComponent* m_hoveredComponent = nullptr;

    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    glm::vec2 screenToWorld2D(const glm::vec2& screenCoords, Camera2DComponent* camera);

    bool isMouseOver2D(GameObject* gameObject, const glm::vec2& mouseWorldPos2D);

    Ray screenToWorldRay3D(const glm::vec2& screenCoords, Camera3DComponent* camera);

    bool rayIntersectsAABB(const Ray& ray, GameObject* gameObject, float& outT);

    void getGameObjectWorldAABB(GameObject* obj, glm::vec3& outMin, glm::vec3& outMax);
};