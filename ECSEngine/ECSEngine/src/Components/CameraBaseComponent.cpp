#include "CameraBaseComponent.h"
#include <iostream>

CameraBaseComponent::CameraBaseComponent(GameObject* owner, float nearPlane, float farPlane)
    : Component(owner),
    m_nearPlane(nearPlane), m_farPlane(farPlane),
    m_projectionMatrix(1.0f)
{

}

void CameraBaseComponent::setNearPlane(float near) {
    m_nearPlane = near;
    recalculateProjection();
}

void CameraBaseComponent::setFarPlane(float far) {
    m_farPlane = far;
    recalculateProjection();
}