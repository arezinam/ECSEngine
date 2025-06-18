#pragma once
#include "Core/Scene.h"
#include "Microwave.h"
#include <memory>
#include <string>
#include <functional>

class GameObject;
class RenderComponent;

class MicrowaveGameScene : public Scene {
public:
    MicrowaveGameScene(const std::string& name);
    ~MicrowaveGameScene() override;

    void Init() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

private:
    void SetupMicrowaveGameObjects();
    void updateTimerDisplay();

    glm::vec4 calculateFlicker(float time, float speed);

    Microwave m_microwave; 

    GameObject* m_microwaveGameObject;
    GameObject* m_windowGameObject;
    GameObject* m_hexContainerGameObject;
    GameObject* m_lightContainerGameObject;
    GameObject* m_windowPivotGameObject;
    GameObject* m_interiorContainerGameObject;
    GameObject* m_displayContainer;
    GameObject* m_smokeFilterGameObject;

    RenderComponent* m_timerTextRenderComponent;
    RenderComponent* m_hexRenderComponent;
    RenderComponent* m_smokeFilterRenderComponent;

    float m_doorAnimationTime;
    float m_animationDuration;
    float m_initialWindowWorldX; 
    float m_initialWindowWorldY;
    float m_initialWindowWidth;

    float m_flickerTimer;
    float m_flickerSpeed;       
    float m_flickerMinAlpha;    
    float m_flickerMaxAlpha;    
    glm::vec4 m_baseLightColor; 

    float m_errorBlinkTimer;
    float m_errorBlinkInterval; 
    bool m_isErrorTextVisible;

    float m_currentDoorTargetAngle;

    GameObject* m_runningStateIndicator;
    RenderComponent* m_runningStateIndicatorRenderComponent;
    float m_runningBlinkTimer;
    float m_runningBlinkInterval;
    bool m_isIndicatorVisible; 
};