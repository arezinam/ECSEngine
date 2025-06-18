#include "ClickableComponent.h"
#include <iostream>

ClickableComponent::ClickableComponent(GameObject* owner, PickingMethod method)
    : Component(owner), m_pickingMethod(method) {
 
}

ClickableComponent::~ClickableComponent() {
    
}

void ClickableComponent::Init() {
   
}

void ClickableComponent::Update(float deltaTime) {
   
}

void ClickableComponent::Render() {
   
}


void ClickableComponent::onClick() {
    if (m_onClickCallback) {
      
        m_onClickCallback();
    }
}

void ClickableComponent::onHoverEnter() {
    if (!m_isHovered && m_onHoverEnterCallback) {

        m_onHoverEnterCallback();
    }
    m_isHovered = true;
}

void ClickableComponent::onHoverExit() {
    if (m_isHovered && m_onHoverExitCallback) {
        m_onHoverExitCallback();
    }
    m_isHovered = false;
}