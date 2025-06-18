#include "Core/AssetManager.h"
#include "Core/Shader.h" 
#include "Core/Texture.h"

#include <iostream>

std::shared_ptr<Shader> AssetManager::getShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
    std::string shaderKey = vertexPath + "|" + fragmentPath;
    if (!geometryPath.empty()) {
        shaderKey += "|" + geometryPath;
    }

    auto it = m_shaders.find(shaderKey);
    if (it != m_shaders.end()) {

        return it->second;
    }

    std::shared_ptr<Shader> newShader = std::make_shared<Shader>(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str());

    if (newShader->getID() == 0) { 
        std::cerr << "ERROR: AssetManager: Failed to load shader: " << shaderKey << std::endl;
        return nullptr;
    }

    m_shaders[shaderKey] = newShader;
    return newShader;
}

std::shared_ptr<Texture> AssetManager::getTexture(const std::string& path, const std::string& type) {
    std::string textureKey = path;
    if (!type.empty()) {
        textureKey += "|" + type;
    }

    auto it = m_textures.find(textureKey);
    if (it != m_textures.end()) {
        return it->second;
    }

    std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(path.c_str(), type.c_str());

    if (newTexture->getID() == 0) {
        std::cerr << "ERROR: AssetManager: Failed to load texture: " << textureKey << std::endl;
        return nullptr;
    }

    m_textures[textureKey] = newTexture;
    return newTexture;
}

void AssetManager::clearAllAssets() {
    std::cout << "AssetManager: Clearing all cached assets." << std::endl;
    m_shaders.clear();
    m_textures.clear();
}