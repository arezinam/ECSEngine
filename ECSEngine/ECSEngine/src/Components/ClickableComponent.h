#pragma once

#include "Core/Component.h"
#include "Core/GameObject.h"
#include <glm/glm.hpp>
#include <functional>

enum class PickingMethod {
    Method2D,
    Method3D 
};

class ClickableComponent : public Component {
public:
    ClickableComponent(GameObject* owner, PickingMethod method = PickingMethod::Method2D);
    ~ClickableComponent();

    void Init() override;
    void Update(float deltaTime) override;
    void Render() override;

    PickingMethod getPickingMethod() const { return m_pickingMethod; }

    void setOnClickCallback(std::function<void()> callback) { m_onClickCallback = callback; }
    void onClick();

    void setOnHoverEnterCallback(std::function<void()> callback) { m_onHoverEnterCallback = callback; }
    void setOnHoverExitCallback(std::function<void()> callback) { m_onHoverExitCallback = callback; }
    void onHoverEnter();
    void onHoverExit();
    bool isHovered() const { return m_isHovered; }
    void setHovered(bool hovered) { m_isHovered = hovered; }

private:
    PickingMethod m_pickingMethod;
    std::function<void()> m_onClickCallback;
    std::function<void()> m_onHoverEnterCallback;
    std::function<void()> m_onHoverExitCallback;
    bool m_isHovered = false;
};