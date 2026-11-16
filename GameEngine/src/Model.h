#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Shader.h"
#include "TextureManager.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Mesh
{
public:
    Mesh();
    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<unsigned int>& indices,
         const std::string& materialName,
         unsigned int textureId);
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    ~Mesh();

    void Draw() const;

    std::string materialName;

private:
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;
    std::vector<Vertex> m_Vertices;
    std::vector<unsigned int> m_Indices;
    unsigned int m_TextureId = 0;
};

class Model
{
public:
    explicit Model(const std::string& filepath);
    void Draw(Shader& shader) const;

private:
    void LoadObj(const std::string& filepath);
    static std::string ResolveTextureName(const std::string& materialName);

    std::vector<Mesh> m_Meshes;
};
