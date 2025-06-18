#include "Components/MeshComponent.h"
#include "Core/GameObject.h"
#include <iostream>


MeshComponent::MeshComponent(GameObject* owner)
    : Component(owner),
    m_mesh(std::make_shared<Mesh>("DefaultCubeMesh"))
{
    m_mesh->createDefaultCube();
    std::cout << "MeshComponent created with default cube mesh for owner: " << (owner ? owner->getName() : "nullptr") << std::endl;
}

MeshComponent::MeshComponent(GameObject* owner, std::shared_ptr<Mesh> mesh)
    : Component(owner),
    m_mesh(std::move(mesh))
{
    if (!m_mesh) {
        std::cerr << "WARNING: MeshComponent created with a null Mesh pointer for owner: " << (owner ? owner->getName() : "nullptr") << std::endl;
    }
    std::cout << "MeshComponent created with existing mesh: " << (m_mesh ? m_mesh->getName() : "NULL") << " for owner: " << (owner ? owner->getName() : "nullptr") << std::endl;
}