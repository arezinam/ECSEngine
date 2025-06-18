#pragma once

#include "CameraBaseComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject;

class Camera2DComponent : public CameraBaseComponent {
public:
    Camera2DComponent(GameObject* owner,
        float screenWidth = 800.0f,
        float screenHeight = 600.0f,
        float nearPlane = -1.0f,
        float farPlane = 1.0f);

    virtual ~Camera2DComponent() = default;

    void recalculateProjection() override;
    glm::mat4 getViewMatrix() const override;

    void setScreenDimensions(float width, float height);
    void setZoom(float zoom);

    float getScreenWidth() const { return m_screenWidth; }
    float getScreenHeight() const { return m_screenHeight; }
    float getZoom() const { return m_zoom; }

private:
    float m_screenWidth;
    float m_screenHeight;
    float m_zoom;
};