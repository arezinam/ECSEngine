#include <glm/gtx/string_cast.hpp> 
#include <GL/glew.h>
#include "RenderComponent.h"
#include "Core/GameObject.h"
#include <iostream> 

RenderComponent::RenderComponent(GameObject* owner, std::shared_ptr<Shader> shader)
    : Component(owner), m_shader(shader), m_mesh(nullptr), m_objectColor(1.0f, 1.0f, 1.0f, 0.5f) 
{

}

RenderComponent::~RenderComponent() {
}

void RenderComponent::Init() {
}

void RenderComponent::Update(float deltaTime) {
}

void RenderComponent::Render(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model) {
    if (m_shader) {
        m_shader->use();

        m_shader->setMat4("model", model);
        m_shader->setMat4("view", view);
        m_shader->setMat4("projection", projection);

        m_shader->setVec4("objectColor", m_objectColor);

        if (m_texture) {
            glActiveTexture(GL_TEXTURE0);
            m_texture->bind();
            m_shader->setInt("texture_diffuse1", 0);
            m_shader->setInt("hasTexture", 1);
        }
        else {
            m_shader->setInt("hasTexture", 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (m_mesh) {
            m_mesh->draw();

            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cerr << "OpenGL Error after Mesh::draw() for " << m_mesh->getName() << ": " << error << std::endl;
            }

        }
        else {
            std::cerr << "WARNING: RenderComponent on GameObject '" << (getOwner() ? getOwner()->getName() : "Unknown")
                << "' trying to render without a Mesh assigned!" << std::endl;
        }
    }
    else {
        std::cerr << "WARNING: RenderComponent on GameObject '" << (getOwner() ? getOwner()->getName() : "Unknown")
            << "' trying to render without a Shader assigned!" << std::endl;
    }
    m_shader->detach();
}