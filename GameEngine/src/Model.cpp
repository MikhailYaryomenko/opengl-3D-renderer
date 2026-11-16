#include "Model.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Renderer.h"

namespace {
std::string ResolveAssetPath(const std::string& filepath)
{
    std::vector<std::string> candidates = {
        filepath,
        "./" + filepath,
        "GameEngine/" + filepath,
        "../GameEngine/" + filepath,
        "../../GameEngine/" + filepath
    };

    for (const std::string& candidate : candidates)
    {
        std::ifstream file(candidate);
        if (file.is_open())
            return candidate;
    }

    return filepath;
}

std::string Trim(const std::string& value)
{
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return {};

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

std::vector<std::string> Split(const std::string& value, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream stream(value);
    std::string item;
    while (std::getline(stream, item, delimiter))
    {
        if (!item.empty())
            result.push_back(item);
    }
    return result;
}

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

struct FaceIndex
{
    int position = -1;
    int texCoord = -1;
    int normal = -1;
};

FaceIndex ParseFaceToken(const std::string& token)
{
    FaceIndex index;
    const auto first = token.find('/');
    if (first == std::string::npos)
    {
        index.position = std::stoi(token);
        return index;
    }

    const auto second = token.find('/', first + 1);
    const std::string posText = token.substr(0, first);
    const std::string texText = (first == second) ? "" : token.substr(first + 1, second - first - 1);
    const std::string normalText = (second == std::string::npos) ? "" : token.substr(second + 1);

    if (!posText.empty())
        index.position = std::stoi(posText);
    if (!texText.empty())
        index.texCoord = std::stoi(texText);
    if (!normalText.empty())
        index.normal = std::stoi(normalText);

    return index;
}
}

Mesh::Mesh() = default;

Mesh::Mesh(const std::vector<Vertex>& vertices,
           const std::vector<unsigned int>& indices,
           const std::string& materialName,
           unsigned int textureId)
    : materialName(materialName), m_Vertices(vertices), m_Indices(indices), m_TextureId(textureId)
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_Vertices.size() * sizeof(Vertex)), m_Vertices.empty() ? nullptr : &m_Vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_Indices.size() * sizeof(unsigned int)), m_Indices.empty() ? nullptr : &m_Indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));

    glBindVertexArray(0);
}

Mesh::Mesh(Mesh&& other) noexcept
    : materialName(std::move(other.materialName)),
      m_VAO(other.m_VAO),
      m_VBO(other.m_VBO),
      m_EBO(other.m_EBO),
      m_Vertices(std::move(other.m_Vertices)),
      m_Indices(std::move(other.m_Indices)),
      m_TextureId(other.m_TextureId)
{
    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
    other.m_TextureId = 0;
    other.m_Vertices.clear();
    other.m_Indices.clear();
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        if (m_VAO != 0)
            glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO != 0)
            glDeleteBuffers(1, &m_VBO);
        if (m_EBO != 0)
            glDeleteBuffers(1, &m_EBO);

        materialName = std::move(other.materialName);
        m_VAO = other.m_VAO;
        m_VBO = other.m_VBO;
        m_EBO = other.m_EBO;
        m_Vertices = std::move(other.m_Vertices);
        m_Indices = std::move(other.m_Indices);
        m_TextureId = other.m_TextureId;

        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
        other.m_TextureId = 0;
        other.m_Vertices.clear();
        other.m_Indices.clear();
    }
    return *this;
}

Mesh::~Mesh()
{
    if (m_VAO != 0)
        glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO != 0)
        glDeleteBuffers(1, &m_VBO);
    if (m_EBO != 0)
        glDeleteBuffers(1, &m_EBO);
}

void Mesh::Draw() const
{
    if (m_VAO == 0)
        return;

    glActiveTexture(GL_TEXTURE0);
    if (m_TextureId != 0)
        glBindTexture(GL_TEXTURE_2D, m_TextureId);
    else
        glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

Model::Model(const std::string& filepath)
{
    LoadObj(filepath);
}

std::string Model::ResolveTextureName(const std::string& materialName)
{
    const std::string lower = ToLower(Trim(materialName));
    if (lower.find("shoe") != std::string::npos)
        return "shoes";
    if (lower.find("back") != std::string::npos)
        return "back";
    if (lower.find("color") != std::string::npos || lower.find("pallete") != std::string::npos)
        return "color_pallete";
    return "color_pallete";
}

void Model::Draw(Shader& shader) const
{
    shader.Bind();
    for (const Mesh& mesh : m_Meshes)
    {
        mesh.Draw();
    }
}

void Model::LoadObj(const std::string& filepath)
{
    std::ifstream file(ResolveAssetPath(filepath));
    if (!file.is_open())
    {
        std::cerr << "[ERROR] Failed to open model file: " << filepath << std::endl;
        return;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::string currentMaterial = "color_pallete";

    struct MaterialMeshBuilder
    {
        std::string materialName;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };

    std::vector<MaterialMeshBuilder> materialBuilders;
    auto ensureBuilder = [&](const std::string& materialName) -> MaterialMeshBuilder& {
        for (auto& builder : materialBuilders)
        {
            if (builder.materialName == materialName)
                return builder;
        }

        materialBuilders.push_back({ materialName, {}, {} });
        return materialBuilders.back();
    };

    std::string line;
    while (std::getline(file, line))
    {
        const std::string trimmed = Trim(line);
        if (trimmed.empty() || trimmed[0] == '#')
            continue;

        if (trimmed.rfind("v ", 0) == 0)
        {
            std::istringstream stream(trimmed.substr(2));
            float x = 0.0f, y = 0.0f, z = 0.0f;
            stream >> x >> y >> z;
            positions.emplace_back(x, y, z);
            continue;
        }

        if (trimmed.rfind("vt ", 0) == 0)
        {
            std::istringstream stream(trimmed.substr(3));
            float u = 0.0f, v = 0.0f;
            stream >> u >> v;
            texCoords.emplace_back(u, v);
            continue;
        }

        if (trimmed.rfind("vn ", 0) == 0)
        {
            std::istringstream stream(trimmed.substr(3));
            float x = 0.0f, y = 0.0f, z = 0.0f;
            stream >> x >> y >> z;
            normals.emplace_back(x, y, z);
            continue;
        }

        if (trimmed.rfind("usemtl ", 0) == 0)
        {
            currentMaterial = Trim(trimmed.substr(7));
            ensureBuilder(currentMaterial);
            continue;
        }

        if (trimmed.rfind("f ", 0) == 0)
        {
            std::istringstream stream(trimmed.substr(2));
            std::string token;
            std::vector<FaceIndex> faceTokens;
            while (stream >> token)
                faceTokens.push_back(ParseFaceToken(token));

            if (faceTokens.size() < 3)
                continue;

            MaterialMeshBuilder& builder = ensureBuilder(currentMaterial);
            for (size_t i = 1; i + 1 < faceTokens.size(); ++i)
            {
                const std::array<FaceIndex, 3> triangle = { faceTokens[0], faceTokens[i], faceTokens[i + 1] };
                for (const FaceIndex& index : triangle)
                {
                    Vertex vertex{};
                    if (index.position > 0 && index.position <= static_cast<int>(positions.size()))
                        vertex.position = positions[index.position - 1];
                    else
                        vertex.position = glm::vec3(0.0f);

                    if (index.normal > 0 && index.normal <= static_cast<int>(normals.size()))
                        vertex.normal = normals[index.normal - 1];
                    else
                        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);

                    if (index.texCoord > 0 && index.texCoord <= static_cast<int>(texCoords.size()))
                        vertex.texCoord = texCoords[index.texCoord - 1];
                    else
                        vertex.texCoord = glm::vec2(0.0f, 0.0f);

                    const unsigned int vertexIndex = static_cast<unsigned int>(builder.vertices.size());
                    builder.vertices.push_back(vertex);
                    builder.indices.push_back(vertexIndex);
                }
            }
        }
    }

    for (const MaterialMeshBuilder& builder : materialBuilders)
    {
        if (builder.vertices.empty())
            continue;

        const std::string textureName = ResolveTextureName(builder.materialName);
        const std::string texturePath = "./res/textures/" + textureName + ".png";
        const unsigned int textureId = TextureManager::Load(texturePath, textureName);
        m_Meshes.emplace_back(builder.vertices, builder.indices, builder.materialName, textureId);
    }
}


