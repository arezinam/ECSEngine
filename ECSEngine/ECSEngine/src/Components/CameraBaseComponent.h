#pragma once

#include "Core/Component.h"
#include <glm/glm.hpp>

class GameObject;

class CameraBaseComponent : public Component {
public:
    CameraBaseComponent(GameObject* owner, float nearPlane, float farPlane);

    virtual ~CameraBaseComponent() = default;

    virtual void recalculateProjection() = 0;
    virtual glm::mat4 getViewMatrix() const = 0;

    const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }


    void setNearPlane(float near);
    void setFarPlane(float far);
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }

protected:
    float m_nearPlane;
    float m_farPlane;
    glm::mat4 m_projectionMatrix;
};