#pragma once

class GameObject;

class Component {
public:
    Component(GameObject* owner) : m_gameObject(owner) {}

    virtual ~Component() = default;
    virtual void Init() {}
    virtual void Update(float dt) {}
    virtual void Render() {}

    GameObject* getOwner() const { return m_gameObject; }

protected:
    GameObject* m_gameObject = nullptr;
};