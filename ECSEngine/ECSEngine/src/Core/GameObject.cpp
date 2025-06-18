#include "GameObject.h"
#include "Components/RenderComponent.h"
#include "Components/MeshComponent.h"
#include "Components/ClickableComponent.h"
#include "Core/PickingManager.h"

#include <iostream> 

GameObject::GameObject(const std::string& name)
    : m_name(name), m_transform(this), m_parent(nullptr)
{
}

GameObject::~GameObject() {
    for (const auto& comp_ptr : m_components) {
        if (comp_ptr) { 
            if (ClickableComponent* clickable = dynamic_cast<ClickableComponent*>(comp_ptr.get())) {
                PickingManager::getInstance().RemoveClickable(clickable);
            }
        }
    }
    m_components.clear(); 
}

void GameObject::Init() {
    for (const auto& child : m_children) {
        if (child) {
            child->Init();
        }
    }
}

void GameObject::Update(float deltaTime) {
    m_transform.getWorldMatrix();

    UpdateComponents(deltaTime);
    for (const auto& child : m_children) {
        if (child) {
            child->Update(deltaTime); 
        }
    }
}

void GameObject::Render(const glm::mat4& view, const glm::mat4& projection) {
    RenderComponents(view, projection); 
    for (const auto& child : m_children) {
        if (child) {
            child->Render(view, projection);
        }
    }
}

void GameObject::InitComponents() {
}

void GameObject::UpdateComponents(float deltaTime) {
    for (const auto& comp : m_components) {
        if (comp) { 
            comp->Update(deltaTime);
        }
    }
}

void GameObject::RenderComponents(const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 modelMatrix = m_transform.getWorldMatrix();

    for (const auto& comp : m_components) {
        if (RenderComponent* renderComp = dynamic_cast<RenderComponent*>(comp.get())) {
            renderComp->Render(view, projection, modelMatrix);
        }
    }
}

GameObject* GameObject::addChild(std::unique_ptr<GameObject> child) {
    if (child) {
        GameObject* rawPtr = child.get();

        rawPtr->m_parent = this; 

        if (rawPtr->getTransform()) {
            rawPtr->getTransform()->invalidateWorldMatrix();
        }

        m_children.push_back(std::move(child));
        return rawPtr;
    }
    return nullptr;
}

void GameObject::removeLastChild() {
    if (!m_children.empty()) {
        m_children.back()->m_parent = nullptr;
        m_children.pop_back(); 
    }
}