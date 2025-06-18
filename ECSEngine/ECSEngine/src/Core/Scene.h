#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "PickingManager.h"

class GameObject;
class Shader;
class CameraBaseComponent;
class FontRenderer;

class Scene {
public:
    Scene(const std::string& name);
    virtual ~Scene();

    virtual void Init();
    virtual void Update(float deltaTime);
    virtual void Render();
    virtual void Shutdown();

    const std::string& getName() const { return m_name; }

    GameObject* AddGameObject(std::unique_ptr<GameObject> gameObject);
    void RemoveGameObject(GameObject* gameObject);
    void RemoveGameObjectByName(const std::string& name);

    void setActiveCamera(CameraBaseComponent* camera);
    CameraBaseComponent* getActiveCamera() const { return m_activeCamera; }

    int getWindowWidth() const { return m_windowWidth; }
    int getWindowHeight() const { return m_windowHeight; }
    void setWindowDimensions(int width, int height);


protected:
    std::string m_name;
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    CameraBaseComponent* m_activeCamera;

    int m_windowWidth;
    int m_windowHeight;

    std::shared_ptr<FontRenderer> m_fontRenderer;


};