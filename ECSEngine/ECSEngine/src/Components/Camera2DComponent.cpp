#include "Components/Camera2DComponent.h"
#include "Core/GameObject.h"
#include "Components/TransformComponent.h"
#include <iostream>
#include <algorithm>

Camera2DComponent::Camera2DComponent(GameObject* owner, float screenWidth, float screenHeight, float nearPlane, float farPlane)
    : CameraBaseComponent(owner, nearPlane, farPlane),
    m_screenWidth(screenWidth), m_screenHeight(screenHeight),
    m_zoom(1.0f)
{
    recalculateProjection();
    std::cout << "Camera2DComponent (2D) created for owner: " << (owner ? owner->getName() : "nullptr")
        << ". Screen: " << m_screenWidth << "x" << m_screenHeight << ", Z-range: " << m_nearPlane << " to " << m_farPlane << std::endl;
}

void Camera2DComponent::recalculateProjection() {
    m_projectionMatrix = glm::ortho(0.0f, m_screenWidth / m_zoom,
        0.0f, m_screenHeight / m_zoom,
        m_nearPlane, m_farPlane);
}

glm::mat4 Camera2DComponent::getViewMatrix() const {
    if (m_gameObject && m_gameObject->getTransform()) {
        glm::vec3 cameraPosition = m_gameObject->getTransform()->getLocalPosition();

        return glm::translate(glm::mat4(1.0f), -cameraPosition);
    }
    return glm::mat4(1.0f);
}

void Camera2DComponent::setScreenDimensions(float width, float height) {
    m_screenWidth = width;
    m_screenHeight = height;
    recalculateProjection();
}

void Camera2DComponent::setZoom(float zoom) {
    m_zoom = glm::max(0.1f, zoom);
    recalculateProjection();
}