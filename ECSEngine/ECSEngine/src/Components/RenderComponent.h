#pragma once
#include "Core/Component.h"
#include "Core/Shader.h"
#include "Core/Texture.h"
#include "Core/Mesh.h"
#include <glm/glm.hpp>
#include "../Core/GameObject.h"

class RenderComponent : public Component {
public:
 
    RenderComponent(GameObject* owner, std::shared_ptr<Shader> shader);
    ~RenderComponent();

    void Init() override;
    void Update(float deltaTime) override;
    void Render(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model);

    void setMesh(std::shared_ptr<Mesh> mesh) { m_mesh = mesh; }
    void setTexture(std::shared_ptr<Texture> texture) { m_texture = texture; }
    void setObjectColor(const glm::vec4& color) { m_objectColor = color; }
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Texture> m_texture;
    glm::vec4 m_objectColor;


private:
    std::shared_ptr<Shader> m_shader;
};