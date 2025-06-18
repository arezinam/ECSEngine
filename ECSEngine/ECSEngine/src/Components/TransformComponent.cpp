#include "TransformComponent.h"
#include "Core/GameObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp> 
#include <glm/gtx/euler_angles.hpp>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

TransformComponent::TransformComponent(GameObject* owner)
    : Component(owner),
    m_localPosition(0.0f, 0.0f, 0.0f),
    m_localRotation(1.0f, 0.0f, 0.0f, 0.0f),
    m_localScale(1.0f, 1.0f, 1.0f),
    m_worldMatrix(1.0f),
    m_isDirty(true)
{
}

void TransformComponent::setLocalPosition(const glm::vec3& pos) {
    m_localPosition = pos;
    invalidateWorldMatrix();
}

void TransformComponent::setLocalRotation(const glm::quat& rot) {
    m_localRotation = glm::normalize(rot);
    invalidateWorldMatrix();
}

void TransformComponent::setLocalScale(const glm::vec3& scale) {
    m_localScale = scale;
    invalidateWorldMatrix();
}

void TransformComponent::setLocalEulerRotation(const glm::vec3& eulerAnglesDegrees) {
    glm::vec3 eulerRadians = glm::radians(eulerAnglesDegrees);
    glm::quat rotY = glm::angleAxis(eulerRadians.y, glm::vec3(0.0f, 1.0f, 0.0f));
    m_localRotation = rotY;

    m_localRotation = glm::normalize(m_localRotation);
    invalidateWorldMatrix();
}

void TransformComponent::translate(const glm::vec3& delta) {
    m_localPosition += delta;
    invalidateWorldMatrix();
}

void TransformComponent::rotate(const glm::quat& deltaRotation) {
    m_localRotation = deltaRotation * m_localRotation; 
    m_localRotation = glm::normalize(m_localRotation);
    invalidateWorldMatrix();
}

void TransformComponent::rotate(float angleDegrees, const glm::vec3& axis) {
    glm::quat rotationDelta = glm::angleAxis(glm::radians(angleDegrees), glm::normalize(axis));
    m_localRotation = rotationDelta * m_localRotation;
    m_localRotation = glm::normalize(m_localRotation); 
    invalidateWorldMatrix();
}

glm::mat4 TransformComponent::getLocalMatrix() const {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_localPosition);
    glm::mat4 rotationMatrix = glm::toMat4(m_localRotation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_localScale);

    return translationMatrix * rotationMatrix * scaleMatrix;
}

const glm::mat4& TransformComponent::getWorldMatrix() {
    if (m_isDirty) {
        calculateWorldMatrix();
    }
    return m_worldMatrix;
}

void TransformComponent::calculateWorldMatrix() {
    glm::mat4 localMatrix = getLocalMatrix();

    if (m_gameObject && m_gameObject->m_parent) { 
        TransformComponent* parentTransform = m_gameObject->m_parent->getTransform();
        if (parentTransform) {
            m_worldMatrix = parentTransform->getWorldMatrix() * localMatrix;
        }
        else {
            m_worldMatrix = localMatrix;
        }
    }
    else {
        m_worldMatrix = localMatrix;
    }
    m_isDirty = false;
}

void TransformComponent::invalidateWorldMatrix() {
    if (m_isDirty) return;

    m_isDirty = true;

    if (m_gameObject) { 
        for (const auto& childUniquePtr : m_gameObject->getChildren()) {
            if (childUniquePtr) {
                TransformComponent* childTransform = childUniquePtr->getTransform();
                if (childTransform) {
                    childTransform->invalidateWorldMatrix();
                }
            }
        }
    }
}

glm::vec3 TransformComponent::getLocalEulerAnglesDegrees() const {
    float angleY_radians = 2.0f * std::atan2(m_localRotation.y, m_localRotation.w);
    float angleY_degrees = glm::degrees(angleY_radians);

    return glm::vec3(0.0f, angleY_degrees, 0.0f);
}


void TransformComponent::calculateWorldAABB(glm::vec3& outMin, glm::vec3& outMax) {
    MeshComponent* meshComp = m_gameObject->getComponent<MeshComponent>();
    if (!meshComp || !meshComp->getMesh()) {
        outMin = glm::vec3(0.0f);
        outMax = glm::vec3(0.0f);
        std::cerr << "WARNING: GameObject " << m_gameObject->getName() << " has no MeshComponent or Mesh for AABB calculation." << std::endl;
        return;
    }

    glm::vec3 localMin = meshComp->getMesh()->getLocalAABBMin();
    glm::vec3 localMax = meshComp->getMesh()->getLocalAABBMax();

    glm::mat4 world = getWorldMatrix();

    outMin = glm::vec3(std::numeric_limits<float>::max());
    outMax = glm::vec3(std::numeric_limits<float>::lowest());

    glm::vec3 corners[8] = {
        glm::vec3(localMin.x, localMin.y, localMin.z),
        glm::vec3(localMax.x, localMin.y, localMin.z),
        glm::vec3(localMin.x, localMax.y, localMin.z),
        glm::vec3(localMin.x, localMin.y, localMax.z),
        glm::vec3(localMax.x, localMax.y, localMin.z),
        glm::vec3(localMax.x, localMin.y, localMax.z),
        glm::vec3(localMin.x, localMax.y, localMax.z),
        glm::vec3(localMax.x, localMax.y, localMax.z)
    };

    for (int i = 0; i < 8; ++i) {
        glm::vec4 transformedCorner = world * glm::vec4(corners[i], 1.0f);
        glm::vec3 worldCorner = glm::vec3(transformedCorner) / transformedCorner.w;

        outMin = glm::min(outMin, worldCorner);
        outMax = glm::max(outMax, worldCorner);
    }
}