#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <memory> 

#include "Core/Shader.h"
#include "Texture.h"


struct Character {
    unsigned int TextureID;
    glm::ivec2   Size;     
    glm::ivec2   Bearing;
    unsigned int Advance;
};

class FontRenderer {
public:
    FontRenderer();
    ~FontRenderer();

    bool init(unsigned int windowWidth, unsigned int windowHeight);

    bool loadFont(const std::string& fontPath, unsigned int fontSize, const std::string& textToWarmUp = "");

    void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color);


    std::unique_ptr<Texture> GenerateTextTexture(const std::string& ttfPath, const std::string& text, int pxSize);

    void setProjection(unsigned int windowWidth, unsigned int windowHeight);

private:
    std::map<FT_ULong, Character> m_characters;
    std::shared_ptr<Shader> m_textShader; 
    unsigned int m_VAO, m_VBO;
    glm::mat4 m_projection;

    FT_Library m_ft;
    FT_Face m_face;

    FT_ULong decodeUtf8(std::string::const_iterator& it, const std::string::const_iterator& end);
};