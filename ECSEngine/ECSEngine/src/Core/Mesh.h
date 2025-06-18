#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};


struct MeshTexture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:

    Mesh(const std::string& name = "Default Cube");
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const std::string& name = "Custom Mesh"); 
    ~Mesh();

    void draw();
    const std::string& getName() const { return m_name; }
    void createDefaultCube();

    void generatePlane(float tileFactor = 1.0f);
    void generateQuad2D();

    const glm::vec3& getLocalAABBMin() const { return m_localAABBMin; }
    const glm::vec3& getLocalAABBMax() const { return m_localAABBMax; }

private:
    std::string m_name;
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    unsigned int VAO, VBO, EBO;

    glm::vec3 m_localAABBMin;
    glm::vec3 m_localAABBMax;

    void setupMesh();
    void calculateLocalAABB(); 
};