#include "Camera3DComponent.h"
#include "Core/GameObject.h"
#include "Components/TransformComponent.h"
#include <iostream>
#include <algorithm>

Camera3DComponent::Camera3DComponent(GameObject* owner, float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
    : CameraBaseComponent(owner, nearPlane, farPlane),
    m_fov(fovDegrees), m_aspectRatio(aspectRatio),
    m_lookAtTarget(0.0f, 0.0f, 0.0f)
{
    recalculateProjection();
    std::cout << "CameraComponent (3D) created for owner: " << (owner ? owner->getName() : "nullptr")
        << ". FOV: " << m_fov << ", Aspect: " << m_aspectRatio << std::endl;
}

void Camera3DComponent::recalculateProjection() {
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

glm::mat4 Camera3DComponent::getViewMatrix() const {
    if (m_gameObject && m_gameObject->getTransform()) {
        glm::vec3 cameraPosition = m_gameObject->getTransform()->getLocalPosition();
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        return glm::lookAt(cameraPosition, m_lookAtTarget, upVector);
    }
    return glm::mat4(1.0f);
}

void Camera3DComponent::setFov(float fovDegrees) {
    m_fov = fovDegrees;
    recalculateProjection();
}

void Camera3DComponent::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
    recalculateProjection();
}

void Camera3DComponent::setLookAtTarget(const glm::vec3& target) {
    m_lookAtTarget = target;
}

void Camera3DComponent::applyFovZoom(float scrollDeltaY) {
    float zoomSpeed = 5.0f;

    m_fov -= scrollDeltaY * zoomSpeed;

    float minFov = 10.0f;
    float maxFov = 90.0f;
    m_fov = glm::clamp(m_fov, minFov, maxFov);

    recalculateProjection();
}