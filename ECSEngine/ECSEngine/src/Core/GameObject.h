#pragma once

#include "../Components/TransformComponent.h"
#include "Component.h"
#include "../Components/ClickableComponent.h"
#include "PickingManager.h"
#include "../Components/RenderComponent.h"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>
class Camera3DComponent;


class GameObject {
public:
    GameObject(const std::string& name = "GameObject");
    ~GameObject();

    void Init();
    void Update(float deltaTime);
    void Render(const glm::mat4& view, const glm::mat4& projection);
    virtual void Shutdown() {} 

    template<typename T, typename... Args>
    T* addComponent(Args&&... args);

    template<typename T>
    T* getComponent();

    template<typename T>
    const T* getComponent() const; 

    template<typename T>
    void removeComponent();

    GameObject* addChild(std::unique_ptr<GameObject> child);
    void removeLastChild();

    const std::vector<std::unique_ptr<GameObject>>& getChildren() const { return m_children; }
    const std::string& getName() const { return m_name; }

    TransformComponent* getTransform() { return &m_transform; }
    const TransformComponent* getTransform() const { return &m_transform; } 

    GameObject* m_parent; 

private:
    std::string m_name;
    TransformComponent m_transform;
    std::vector<std::unique_ptr<Component>> m_components;
    std::vector<std::unique_ptr<GameObject>> m_children;

    void InitComponents();
    void UpdateComponents(float deltaTime);
    void RenderComponents(const glm::mat4& view, const glm::mat4& projection);
};


template<typename T, typename... Args>
T* GameObject::addComponent(Args&&... args) {
    static_assert(std::is_base_of<Component, T>::value, "T must be a Component type.");

    auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
    T* rawPtr = component.get();
    m_components.push_back(std::move(component));
    rawPtr->Init(); 

    if (ClickableComponent* clickable = dynamic_cast<ClickableComponent*>(rawPtr)) {
        PickingManager::getInstance().AddClickable(clickable);
    }
    return rawPtr;
}

template<typename T>
T* GameObject::getComponent() { 
    for (const auto& comp : m_components) {
        if (T* specificComp = dynamic_cast<T*>(comp.get())) {
            return specificComp;
        }
    }
    return nullptr;
}

template<typename T>
const T* GameObject::getComponent() const { 
    for (const auto& comp : m_components) {
        if (const T* specificComp = dynamic_cast<const T*>(comp.get())) {
            return specificComp;
        }
    }
    return nullptr;
}

template<typename T>
void GameObject::removeComponent() {
    m_components.erase(
        std::remove_if(m_components.begin(), m_components.end(),
            [&](const std::unique_ptr<Component>& comp) {
                if (dynamic_cast<T*>(comp.get())) {
                    if (ClickableComponent* clickable = dynamic_cast<ClickableComponent*>(comp.get())) {
                        PickingManager::getInstance().RemoveClickable(clickable);
                    }
                    return true; 
                }
                return false;
            }),
        m_components.end());
}