#pragma once

#include "CameraBaseComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject;

class Camera3DComponent : public CameraBaseComponent {
public:
    Camera3DComponent(GameObject* owner,
        float fovDegrees = 45.0f,
        float aspectRatio = 800.0f / 600.0f,
        float nearPlane = 0.1f,
        float farPlane = 100.0f);

    virtual ~Camera3DComponent() = default;


    void recalculateProjection() override;
    glm::mat4 getViewMatrix() const override;

    void setFov(float fovDegrees);
    void setAspectRatio(float aspectRatio);
    void setLookAtTarget(const glm::vec3& target);

    float getFov() const { return m_fov; }
    float getAspectRatio() const { return m_aspectRatio; }
    const glm::vec3& getLookAtTarget() const { return m_lookAtTarget; }

    void applyFovZoom(float scrollDeltaY);

private:
    float m_fov;
    float m_aspectRatio;
    glm::vec3 m_lookAtTarget;
};