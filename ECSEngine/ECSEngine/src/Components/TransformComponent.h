#pragma once

#include "../Core/Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>
#include "MeshComponent.h"

class GameObject;

class TransformComponent : public Component {
public:
    TransformComponent(GameObject* owner);

    const glm::vec3& getLocalPosition() const { return m_localPosition; }
    const glm::quat& getLocalRotation() const { return m_localRotation; }
    const glm::vec3& getLocalScale() const { return m_localScale; }

    void setLocalPosition(const glm::vec3& pos);
    void setLocalRotation(const glm::quat& rot);
    void setLocalScale(const glm::vec3& scale);
    void setLocalEulerRotation(const glm::vec3& eulerAnglesDegrees);

    void translate(const glm::vec3& delta);
    void rotate(const glm::quat& deltaRotation);
    void rotate(float angleDegrees, const glm::vec3& axis);

    glm::mat4 getLocalMatrix() const;

    const glm::mat4& getWorldMatrix();

    void invalidateWorldMatrix();

    bool m_isDirty;
    glm::vec3 getLocalEulerAnglesDegrees() const;
    void calculateWorldAABB(glm::vec3& outMin, glm::vec3& outMax);

private:
    glm::vec3 m_localPosition;
    glm::quat m_localRotation;
    glm::vec3 m_localScale;

    glm::mat4 m_worldMatrix;

    void calculateWorldMatrix();
};