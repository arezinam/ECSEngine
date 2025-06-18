#pragma once

#include "Core/Scene.h"
#include <string>
#include <memory>

class GameObject;
class CameraComponent;

class TowerGameScene : public Scene {
public:
    TowerGameScene(const std::string& name);
    virtual ~TowerGameScene();

    void Init() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

private:
    void SetupTowerGameObjects();

    GameObject* m_towerGameObject;
    float m_currentTowerHeight;
};