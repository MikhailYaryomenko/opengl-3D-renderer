#include "TextureManager.h"

#include "stb/stb_image.h"

#include <fstream>
#include <iostream>
#include <vector>

// TODO: Question if this class is necessary

namespace {
std::string ResolveAssetPath(const std::string& path)
{
    std::vector<std::string> candidates = {
        path,
        "./" + path,
        "GameEngine/" + path,
        "../GameEngine/" + path,
        "../../GameEngine/" + path
    };

    for (const std::string& candidate : candidates)
    {
        std::ifstream file(candidate);
        if (file.good())
            return candidate;
    }

    return path;
}
}

std::map<std::string, unsigned int> TextureManager::s_Textures;

unsigned int TextureManager::Load(const std::string& path, const std::string& name)
{
    if (s_Textures.count(name) != 0)
        return s_Textures[name];

    const std::string resolvedPath = ResolveAssetPath(path);

    unsigned int textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(resolvedPath.c_str(), &width, &height, &channels, 0);

    if (data)
    {
        GLenum format = GL_RGB;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "[ERROR] Failed to load texture: " << resolvedPath << std::endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    s_Textures[name] = textureId;
    return textureId;
}

unsigned int TextureManager::Get(const std::string& name)
{
    auto it = s_Textures.find(name);
    return it != s_Textures.end() ? it->second : 0;
}

void TextureManager::Bind(const std::string& name, unsigned int textureUnit)
{
    unsigned int textureId = Get(name);
    if (textureId == 0)
        return;

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void TextureManager::Unload(const std::string& name)
{
    auto it = s_Textures.find(name);
    if (it == s_Textures.end())
        return;

    glDeleteTextures(1, &it->second);
    s_Textures.erase(it);
}
