#include "Core/FontRenderer.h"
#include <iostream>
#include <ft2build.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <vector>
#include FT_FREETYPE_H
#include FT_GLYPH_H 
FontRenderer::FontRenderer()
    : m_textShader(nullptr), m_VAO(0), m_VBO(0), m_ft(nullptr), m_face(nullptr) {
}


FontRenderer::~FontRenderer() {
    if (m_VBO != 0) glDeleteBuffers(1, &m_VBO);
    if (m_VAO != 0) glDeleteVertexArrays(1, &m_VAO);

    if (m_face) {
        FT_Done_Face(m_face);
        m_face = nullptr;
    }
    if (m_ft) {
        FT_Done_FreeType(m_ft);
        m_ft = nullptr;
    }


    for (std::map<FT_ULong, Character>::const_iterator it = m_characters.begin(); it != m_characters.end(); ++it) {
        glDeleteTextures(1, &it->second.TextureID);
    }
    m_characters.clear();
}


bool FontRenderer::init(unsigned int windowWidth, unsigned int windowHeight) {
    if (FT_Init_FreeType(&m_ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }


    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_textShader = std::make_shared<Shader>("res/shaders/text.vert", "res/shaders/text.frag");
    if (!m_textShader) {
        std::cerr << "ERROR::FONTRENDERER: Could not create text shader. Check shader files (res/shaders/text.vert, res/shaders/text.frag) and their content." << std::endl;
        if (m_VBO != 0) glDeleteBuffers(1, &m_VBO);
        if (m_VAO != 0) glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0; m_VBO = 0; 
        return false;
    }

    setProjection(windowWidth, windowHeight);

    return true;
}

void loadGlyph(FT_Face face, FT_ULong charCode, std::map<FT_ULong, Character>& characters) {
    if (FT_Load_Char(face, charCode, FT_LOAD_RENDER)) {

        return;
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Character character = {
        texture,
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        static_cast<unsigned int>(face->glyph->advance.x)
    };
    characters.insert(std::pair<FT_ULong, Character>(charCode, character));
}

bool FontRenderer::loadFont(const std::string& fontPath, unsigned int fontSize, const std::string& textToWarmUp) {
    if (!m_ft) {
        std::cerr << "ERROR::FREETYPE: FreeType library not initialized. Call init() first." << std::endl;
        return false;
    }

    if (m_face) { 
        FT_Done_Face(m_face);
        m_face = nullptr;
    }
    for (std::map<FT_ULong, Character>::const_iterator it = m_characters.begin(); it != m_characters.end(); ++it) {
        glDeleteTextures(1, &it->second.TextureID);
    }
    m_characters.clear();

    if (FT_New_Face(m_ft, fontPath.c_str(), 0, &m_face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << ". Check path and font file validity." << std::endl;
        return false;
    }

    FT_Set_Pixel_Sizes(m_face, 0, fontSize);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    int loaded_count = 0;
    for (FT_ULong c = 0x0020; c <= 0x024F; c++) {
        if (FT_Get_Char_Index(m_face, c) == 0 && c != 0x0020) { 
            continue;
        }

        if (m_characters.find(c) == m_characters.end()) { 
            loadGlyph(m_face, c, m_characters);
            loaded_count++;
        }
    }

    if (!textToWarmUp.empty()) {
        std::string::const_iterator it = textToWarmUp.begin();
        while (it != textToWarmUp.end()) {
            FT_ULong charCode = decodeUtf8(it, textToWarmUp.end());
            if (charCode != 0 && m_characters.find(charCode) == m_characters.end()) { 
                loadGlyph(m_face, charCode, m_characters);
                loaded_count++;
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    std::cout << "DEBUG::FONTRENDERER: Successfully loaded " << loaded_count << " Unicode glyphs." << std::endl;
    if (loaded_count == 0) {
        std::cerr << "ERROR::FONTRENDERER: No characters were loaded from the font. Font file might be empty, corrupted, or incompatible." << std::endl;
        return false; 
    }
    return true;
}

void FontRenderer::renderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    if (!m_textShader || m_characters.empty()) {
        std::cerr << "ERROR::FONTRENDERER: Shader or characters not initialized. Cannot render text." << std::endl;
        return;
    }

    m_textShader->use();
    m_textShader->setVec3("textColor", color);
    m_textShader->setMat4("projection", m_projection);
    glm::mat4 model = glm::mat4(1.0f);
    m_textShader->setMat4("model", model);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);

    std::string::const_iterator it = text.begin();
    while (it != text.end()) {
        FT_ULong charCode = decodeUtf8(it, text.end());

        if (charCode == 0) { 
            break;
        }

        if (m_characters.find(charCode) == m_characters.end()) {
            std::cerr << "WARNING::FONTRENDERER: Glyph for Unicode code point 0x" << std::hex << charCode
                << " not found in font map. Skipping." << std::dec << std::endl;
            continue;
        }
        Character ch = m_characters[charCode];

        float xpos = x + ch.Bearing.x * scale;

        float ypos = y + (ch.Bearing.y - ch.Size.y) * scale;


        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f }, 
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f }, 

            { xpos,     ypos + h,   0.0f, 0.0f }, 
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }  
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void FontRenderer::setProjection(unsigned int windowWidth, unsigned int windowHeight) {
    if (windowWidth == 0 || windowHeight == 0) {
        std::cerr << "WARNING::FONTRENDERER: Attempted to set projection with zero window dimensions." << std::endl;
        m_projection = glm::ortho(0.0f, static_cast<float>(800), 0.0f, static_cast<float>(600));
        return;
    }
    m_projection = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight));
}

FT_ULong FontRenderer::decodeUtf8(std::string::const_iterator& it, const std::string::const_iterator& end) {
    if (it == end) return 0;

    FT_ULong codePoint = 0;
    unsigned char byte1 = static_cast<unsigned char>(*it);
    ++it; 

    if ((byte1 & 0x80) == 0x00) { 
        codePoint = byte1;
    }
    else if ((byte1 & 0xE0) == 0xC0) { 
        if (it == end) return 0;
        unsigned char byte2 = static_cast<unsigned char>(*it);
        if ((byte2 & 0xC0) != 0x80) return 0; 
        ++it;
        codePoint = ((byte1 & 0x1F) << 6) | (byte2 & 0x3F);
    }
    else if ((byte1 & 0xF0) == 0xE0) { 
        if (it == end) return 0;
        unsigned char byte2 = static_cast<unsigned char>(*it);
        if ((byte2 & 0xC0) != 0x80) return 0;
        ++it;
        if (it == end) return 0;
        unsigned char byte3 = static_cast<unsigned char>(*it);
        if ((byte3 & 0xC0) != 0x80) return 0;
        ++it;
        codePoint = ((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
    }
    else if ((byte1 & 0xF8) == 0xF0) { 
        if (it == end) return 0;
        unsigned char byte2 = static_cast<unsigned char>(*it);
        if ((byte2 & 0xC0) != 0x80) return 0;
        ++it;
        if (it == end) return 0;
        unsigned char byte3 = static_cast<unsigned char>(*it);
        if ((byte3 & 0xC0) != 0x80) return 0;
        ++it;
        if (it == end) return 0;
        unsigned char byte4 = static_cast<unsigned char>(*it);
        if ((byte4 & 0xC0) != 0x80) return 0;
        ++it;
        codePoint = ((byte1 & 0x07) << 18) | ((byte2 & 0x3F) << 12) | ((byte3 & 0x3F) << 6) | (byte4 & 0x3F);
    }
    else {
        return 0;
    }
    return codePoint;
}

std::unique_ptr<Texture> FontRenderer::GenerateTextTexture(const std::string& ttfPath,
    const std::string& text,
    int pxSize) {


    FT_Face face_local; 

    if (FT_New_Face(m_ft, ttfPath.c_str(), 0, &face_local)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font '" << ttfPath << "' for GenerateTextTexture." << std::endl;
        return nullptr;
    }
    FT_Set_Pixel_Sizes(face_local, 0, pxSize);

    std::string::const_iterator it_text = text.begin();
    std::vector<FT_ULong> u32_text_codes;
    while (it_text != text.end()) {
        FT_ULong code = decodeUtf8(it_text, text.end());
        if (code != 0) { 
            u32_text_codes.push_back(code);
        }
        else {
        }
    }

    if (u32_text_codes.empty()) {
        std::cerr << "WARNING::FREETYPE: Input text for GenerateTextTexture is empty or contains only invalid UTF-8. No texture generated." << std::endl;
        FT_Done_Face(face_local);
        return nullptr;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    struct TempGlyphData {
        glm::ivec2   Size;      
        glm::ivec2   Bearing;   
        unsigned int Advance;  
        std::vector<unsigned char> bitmap;
    };
    std::map<FT_ULong, TempGlyphData> local_raw_glyph_data; 

    int maxAscent = 0; 
    int maxDescent = 0;

    int currentPenX_measure = 0;   
    int maxRightExtent_measure = 0;

    for (FT_ULong ch_u32 : u32_text_codes) {
        if (FT_Load_Char(face_local, ch_u32, FT_LOAD_RENDER)) {
            std::cerr << "WARNING::FREETYPE: Could not load/render glyph for U+"
                << std::hex << (unsigned int)ch_u32 << std::dec
                << " for text texture generation. Skipping character." << std::endl;
            currentPenX_measure += (pxSize / 2);
            continue;
        }

        TempGlyphData temp_ch_data;
        temp_ch_data.Size = glm::ivec2(face_local->glyph->bitmap.width, face_local->glyph->bitmap.rows);
        temp_ch_data.Bearing = glm::ivec2(face_local->glyph->bitmap_left, face_local->glyph->bitmap_top);
        temp_ch_data.Advance = static_cast<unsigned int>(face_local->glyph->advance.x);

        if (face_local->glyph->bitmap.width > 0 && face_local->glyph->bitmap.rows > 0) {
            size_t bitmap_size = static_cast<size_t>(face_local->glyph->bitmap.width) * face_local->glyph->bitmap.rows;
            temp_ch_data.bitmap.assign(face_local->glyph->bitmap.buffer, face_local->glyph->bitmap.buffer + bitmap_size);
        }
        local_raw_glyph_data[ch_u32] = temp_ch_data; 

        maxAscent = glm::max(maxAscent, temp_ch_data.Bearing.y);
        int current_char_descent = temp_ch_data.Size.y - temp_ch_data.Bearing.y;
        maxDescent = glm::max(maxDescent, current_char_descent);

        int currentGlyphRightPixel = currentPenX_measure + temp_ch_data.Bearing.x + temp_ch_data.Size.x;
        maxRightExtent_measure = glm::max(maxRightExtent_measure, currentGlyphRightPixel);

        currentPenX_measure += (temp_ch_data.Advance >> 6);
    }
    FT_Done_Face(face_local);

    int padding = 2; 

    int finalWidth = glm::max(currentPenX_measure, maxRightExtent_measure) + 2 * padding;
    int finalHeight = (maxAscent + maxDescent) + 2 * padding;

    if (finalWidth <= 0 || finalHeight <= 0) {
        std::cerr << "WARNING::FREETYPE: Calculated text dimensions are non-positive (" << finalWidth << "x" << finalHeight << ") for text: '"
            << text.substr(0, std::min(text.size(), size_t(50))) << "'. Returning nullptr texture." << std::endl;
        return nullptr;
    }

    std::vector<unsigned char> finalBuffer(static_cast<size_t>(finalWidth) * static_cast<size_t>(finalHeight) * 4, 0);

    int penX_draw = padding;
    for (FT_ULong ch_u32 : u32_text_codes) {
        auto it_glyph_data = local_raw_glyph_data.find(ch_u32);
        if (it_glyph_data == local_raw_glyph_data.end()) {
            penX_draw += (pxSize / 2);
            continue;
        }
        const TempGlyphData& ch = it_glyph_data->second;

        int xPos = penX_draw + ch.Bearing.x;
        int yPos = maxAscent - ch.Bearing.y + padding;

        int glyphW = ch.Size.x;
        int glyphH = ch.Size.y;
        const std::vector<unsigned char>& glyphBitmap = ch.bitmap;

        for (int row = 0; row < glyphH; ++row) {
            for (int col = 0; col < glyphW; ++col) {
                int finalX = xPos + col;
                int finalY = yPos + row;

                assert(finalX >= 0 && finalX < finalWidth && "Generated texture X coordinate out of bounds!");
                assert(finalY >= 0 && finalY < finalHeight && "Generated texture Y coordinate out of bounds!");

                size_t glyphBufferIdx = static_cast<size_t>(row) * glyphW + col;
                assert(glyphBufferIdx < glyphBitmap.size() && "Glyph bitmap data access out of bounds!");

                unsigned char alpha = glyphBitmap[glyphBufferIdx];

                size_t finalBufferPixelIdx = (static_cast<size_t>(finalY) * finalWidth + static_cast<size_t>(finalX)) * 4;
                assert(finalBufferPixelIdx + 3 < finalBuffer.size() && "Final buffer pixel index out of bounds during copy!");

                finalBuffer[finalBufferPixelIdx + 0] = 255;
                finalBuffer[finalBufferPixelIdx + 1] = 255; 
                finalBuffer[finalBufferPixelIdx + 2] = 255; 
                finalBuffer[finalBufferPixelIdx + 3] = alpha;
            }
        }
        penX_draw += (ch.Advance >> 6); 
    }


    GLuint finalTexID;
    glGenTextures(1, &finalTexID);
    glBindTexture(GL_TEXTURE_2D, finalTexID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, finalWidth, finalHeight,
        0, GL_RGBA, GL_UNSIGNED_BYTE, finalBuffer.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    return std::make_unique<Texture>(
        finalTexID,
        "GeneratedText_" + text.substr(0, std::min(text.size(), size_t(50))),
        static_cast<GLuint>(finalWidth),
        static_cast<GLuint>(finalHeight),
        "generated_text_texture" 
    );
}