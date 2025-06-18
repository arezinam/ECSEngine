#pragma once

#include <GL/glew.h>
#include <string>
#include <iostream>
#include <memory>

class Texture {
public:
    Texture(const std::string& path, const std::string& type = "diffuse");

    Texture(GLuint id, const std::string& name, GLuint width, GLuint height, const std::string& type = "generated");

    ~Texture();

    void bind(GLuint unit = 0) const;
    void unbind() const;

    GLuint getID() const { return m_textureID; }
    const std::string& getType() const { return m_type; }
    const std::string& getPath() const { return m_path; } 
    GLuint getWidth() const { return m_width; }          
    GLuint getHeight() const { return m_height; }

private:
    GLuint m_textureID;
    std::string m_type;
    std::string m_path;
    GLuint m_width;
    GLuint m_height;

    void loadTexture(const std::string& path);

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
};