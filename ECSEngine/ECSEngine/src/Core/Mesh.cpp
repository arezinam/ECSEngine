#include "Mesh.h"
#include <iostream>
#include <numeric>
#include <limits> 

Mesh::Mesh(const std::string& name)
    : m_name(name), VAO(0), VBO(0), EBO(0),
    m_localAABBMin(std::numeric_limits<float>::max()), 
    m_localAABBMax(std::numeric_limits<float>::lowest())
{
    createDefaultCube();
    setupMesh();
    calculateLocalAABB();
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const std::string& name)
    : m_name(name), m_vertices(std::move(vertices)), m_indices(std::move(indices)), VAO(0), VBO(0), EBO(0),
    m_localAABBMin(std::numeric_limits<float>::max()),
    m_localAABBMax(std::numeric_limits<float>::lowest())
{
    setupMesh();
    calculateLocalAABB();
}

Mesh::~Mesh() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}

void Mesh::draw() {
    if (VAO == 0 || m_indices.empty()) {
        return;
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error inside Mesh::draw() for " << m_name << ": " << error << std::endl;
    }
    glBindVertexArray(0);
}

void Mesh::setupMesh() {
    if (m_vertices.empty() || m_indices.empty()) {
        std::cerr << "WARNING: Calling setupMesh with empty vertex or index data for mesh: " << m_name << std::endl;
        return;
    }

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::calculateLocalAABB() {
    m_localAABBMin = glm::vec3(std::numeric_limits<float>::max());
    m_localAABBMax = glm::vec3(std::numeric_limits<float>::lowest());

    if (m_vertices.empty()) {
        m_localAABBMin = glm::vec3(0.0f);
        m_localAABBMax = glm::vec3(0.0f);
        return;
    }

    for (const auto& vertex : m_vertices) {
        m_localAABBMin = glm::min(m_localAABBMin, vertex.Position);
        m_localAABBMax = glm::max(m_localAABBMax, vertex.Position);
    }
}


void Mesh::createDefaultCube() {
    m_vertices = {
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, 
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, 
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, 
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, 

        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}, 
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}, 
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, 
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, 

        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, 
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, 
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, 
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, 

        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, 
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, 
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, 
        {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, 

        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, 
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, 
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, 
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, 

        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}} 
    };

    m_indices = {
        0, 1, 2,  0, 2, 3, 
        4, 6, 5,  4, 7, 6,
        8, 9, 10, 8, 10, 11,  
        12, 13, 14, 12, 14, 15, 
        16, 17, 18, 16, 18, 19, 
        20, 21, 22, 20, 22, 23  
    };
}

void Mesh::generatePlane(float tileFactor) {
    m_name = "Generated Plane";
    m_vertices = {
        {{-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f * tileFactor, 1.0f * tileFactor}}, 
        {{ 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f * tileFactor, 1.0f * tileFactor}}, 
        {{ 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f * tileFactor, 0.0f * tileFactor}}, 
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f * tileFactor, 0.0f * tileFactor}} 
    };
    m_indices = {
        0, 1, 2,
        2, 3, 0
    };
    setupMesh();
    calculateLocalAABB(); 
    std::cout << "Mesh data generated: " << m_name << std::endl;
}

void Mesh::generateQuad2D() {
    m_name = "Generated 2D Quad";
    m_vertices.clear();
    m_indices.clear();

    m_vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, 
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}  
    };

    m_indices = {
        0, 1, 2, 
        2, 3, 0  
    };
    setupMesh();
    calculateLocalAABB(); 
}