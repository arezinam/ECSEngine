#define _CRT_SECURE_NO_WARNINGS

#include "Texture.h"
#include "stb_image.h"
#include <cstdio>

#include <filesystem>

Texture::Texture(const std::string& path, const std::string& type)
    : m_textureID(0), m_type(type), m_path(path), m_width(0), m_height(0)
{
    loadTexture(path);
    if (m_textureID != 0) {
        std::cout << "Texture loaded: " << m_path << " (Type: " << m_type << ", ID: " << m_textureID << ", Dims: " << m_width << "x" << m_height << ")" << std::endl;
    }
}

Texture::Texture(GLuint id, const std::string& name, GLuint width, GLuint height, const std::string& type)
    : m_textureID(id), m_type(type), m_path(name), m_width(width), m_height(height)
{
    std::cout << "Texture wrapped: " << m_path << " (Type: " << m_type << ", ID: " << m_textureID << ", Dims: " << m_width << "x" << m_height << ")" << std::endl;
}

Texture::Texture(Texture&& other) noexcept
    : m_textureID(other.m_textureID), m_type(std::move(other.m_type)),
    m_path(std::move(other.m_path)), m_width(other.m_width), m_height(other.m_height)
{
    other.m_textureID = 0;
    other.m_width = 0;
    other.m_height = 0;
    other.m_path = "";
}


Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (m_textureID != 0) {
            glDeleteTextures(1, &m_textureID);
        }

        m_textureID = other.m_textureID;
        m_type = std::move(other.m_type);
        m_path = std::move(other.m_path);
        m_width = other.m_width;
        m_height = other.m_height;

        other.m_textureID = 0;
        other.m_width = 0;
        other.m_height = 0;
        other.m_path = "";
    }
    return *this;
}

Texture::~Texture() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
               if (!m_path.empty()) {
            std::cout << "Texture destroyed: " << m_path << " (ID: " << m_textureID << ")" << std::endl;
        }
        else {
            std::cout << "Texture (ID: " << m_textureID << ") destroyed." << std::endl;
        }
    }
    else {
         std::cout << "Texture object " << m_path << " already moved or not initialized. No OpenGL texture to delete." << std::endl;
    }
}

void Texture::bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::loadTexture(const std::string& path) {
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        GLint internalFormat = GL_RGB8;
        if (format == GL_RED) internalFormat = GL_R8;
        else if (format == GL_RGBA) internalFormat = GL_RGBA8;

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        m_width = width;
        m_height = height;
    }
    else {
        std::cerr << "ERROR::TEXTURE::FAILED_TO_LOAD: " << path << " - " << stbi_failure_reason() << std::endl;
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
}