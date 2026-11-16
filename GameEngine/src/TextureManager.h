#pragma once

#include <map>
#include <string>

#include "Renderer.h"

class TextureManager
{
public:
    static unsigned int Load(const std::string& path, const std::string& name);
    static unsigned int Get(const std::string& name);
    static void Bind(const std::string& name, unsigned int textureUnit = 0);
    static void Unload(const std::string& name);

private:
    static std::map<std::string, unsigned int> s_Textures;
};
