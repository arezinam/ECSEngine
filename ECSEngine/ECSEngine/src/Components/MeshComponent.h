#pragma once

#include "Core/Component.h"
#include <memory>
#include <Core/Mesh.h>

class GameObject;

class MeshComponent : public Component {
public:

    MeshComponent(GameObject* owner);
    MeshComponent(GameObject* owner, std::shared_ptr<Mesh> mesh);

    virtual ~MeshComponent() = default;

    std::shared_ptr<Mesh> getMesh() const { return m_mesh; }

private:
    std::shared_ptr<Mesh> m_mesh;
};