
#include "Renderer.h"

#include <iostream>
#include <vector>
#include <cstddef>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// TODO: Currently there is a .dll file that unmanaged by Visual Studio or Git.
// So repo is not runnable. Fix

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Window.h"

struct Vertex
{
    glm::vec3 position;
};

int main()
{
    Window window(720, 720, "Rotating model");

    // TODO: Refactor this

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        "./res/source/crashbandicoot.obj",
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType
    );

    if (scene == nullptr ||
        scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        scene->mRootNode == nullptr)
    {
        std::cout << "[ERROR] Assimp error: " << importer.GetErrorString() << std::endl;
        return -1;
    }

    // Extract geometry from every mesh.
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        const aiMesh* mesh = scene->mMeshes[i];

        const unsigned int vertexOffset =
            static_cast<unsigned int>(vertices.size());

        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            Vertex vertex;

            vertex.position = glm::vec3(
                mesh->mVertices[j].x,
                mesh->mVertices[j].y,
                mesh->mVertices[j].z
            );

            vertices.push_back(vertex);
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            const aiFace& face = mesh->mFaces[j];

            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(
                    vertexOffset + face.mIndices[k]
                );
            }
        }
    }

    if (vertices.empty() || indices.empty())
    {
        std::cout << "Model contains no renderable geometry.\n";
        return -1;
    }

    std::cout << "Meshes: " << scene->mNumMeshes << '\n';
    std::cout << "Vertices: " << vertices.size() << '\n';
    std::cout << "Indices: " << indices.size() << '\n';

    // Calculate model bounds.
    glm::vec3 min = vertices[0].position;
    glm::vec3 max = vertices[0].position;

    for (const Vertex& vertex : vertices)
    {
        min = glm::min(min, vertex.position);
        max = glm::max(max, vertex.position);
    }

    glm::vec3 center = (min + max) * 0.5f;
    glm::vec3 dimensions = max - min;

    float largestDimension = glm::max(
        dimensions.x,
        glm::max(dimensions.y, dimensions.z)
    );

    if (largestDimension <= 0.0001f)
    {
        std::cout << "Invalid model dimensions.\n";
        return -1;
    }

    // Normalize model size to approximately 2 units.
    float scale = 2.0f / largestDimension;

    std::cout << "Model center: "
        << center.x << ", "
        << center.y << ", "
        << center.z << '\n';

    std::cout << "Model scale: " << scale << '\n';

    VertexArray vao;
    vao.Bind();

    VertexBuffer vbo(
        vertices.data(),
        vertices.size() * sizeof(Vertex)
    );

    IndexBuffer ibo(
        indices.data(),
        indices.size()
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, position)
    );

    glEnableVertexAttribArray(0);

    Shader shader(
        "./res/shaders/vert.glsl",
        "./res/shaders/frag.glsl"
    );

    while (window.IsOpen())
    {
        window.Clear();

        float time = static_cast<float>(glfwGetTime());

        glm::mat4 model(1.0f);

        // Scale and center the imported model.
        model = glm::scale(
            model,
            glm::vec3(scale)
        );

        model = glm::translate(
            model,
            -center
        );

        // Rotate around the model's center.
        model = glm::rotate(
            glm::mat4(1.0f),
            time,
            glm::vec3(0.0f, 1.0f, 0.0f)
        ) * model;

        glm::mat4 view = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(0.0f, 0.0f, -4.0f)
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            1.0f,
            0.1f,
            100.0f
        );

        glm::mat4 mvp = projection * view * model;

        shader.Bind();
        shader.SetUniformMat4f("u_MVP", mvp);

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(ibo.GetCount()),
            GL_UNSIGNED_INT,
            nullptr
        );

        window.Update();
    }

    return 0;
}
