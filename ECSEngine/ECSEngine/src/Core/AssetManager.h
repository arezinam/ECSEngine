#pragma once

#include <string>
#include <map>
#include <memory>
#include <iostream>
class Shader;
class Texture;

class AssetManager {
private:
    AssetManager() {
        std::cout << "AssetManager instance created." << std::endl;
    }

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::map<std::string, std::shared_ptr<Texture>> m_textures;

public:
    static AssetManager& getInstance() {
        static AssetManager instance;
        return instance;
    }

    std::shared_ptr<Shader> getShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
    std::shared_ptr<Texture> getTexture(const std::string& path, const std::string& type = "");

    void clearAllAssets();
};