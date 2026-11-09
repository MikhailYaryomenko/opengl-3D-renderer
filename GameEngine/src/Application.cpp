#include "Renderer.h"

#include <iostream>

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

int main(void)
{
    Window window(720, 720, "Rotating cube");

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile("./res/source/crashbandicoot.obj",
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (nullptr == scene) {
        std::cout << importer.GetErrorString() << std::endl;
        return -1;
    }

    float positions[] = {
        0.0f, 0.0f, -0.5f, // 0
        0.0f, 0.5f, -0.5f, // 1
        0.5f, 0.5f, -0.5f, // 2
        0.5f, 0.0f, -0.5f, // 3
        0.0f, 0.0f,  0.0f, // 4
        0.0f, 0.5f,  0.0f, // 5
        0.5f, 0.5f,  0.0f, // 6
        0.5f, 0.0f,  0.0f  // 7
    };

    unsigned int indices[] = {
        0, 1, 2,  2, 3, 0, // Back
        4, 6, 5,  6, 4, 7, // Front
        0, 4, 5,  5, 1, 0, // Left
        3, 2, 6,  6, 7, 3, // Right
        1, 5, 6,  6, 2, 1, // Top
        0, 3, 7,  7, 4, 0  // Bottom
    };

    VertexArray vao;
    vao.Bind();

    VertexBuffer vbo(positions, sizeof(positions));
    IndexBuffer ibo(indices, sizeof(indices));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    Shader shader("./res/shaders/vert.glsl", "./res/shaders/frag.glsl");

    while (window.IsOpen())
    {
        window.Clear();

        float time = (float)glfwGetTime();

        glm::mat4 model = glm::rotate(
            glm::mat4(1.0f),
            time * 4,
            glm::vec3(1.0f, 8.5f, 2.5f)
        );

        glm::mat4 view = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(0.0f, 0.0f, -2.0f)
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            720.0f / 720.0f,
            0.1f,
            100.0f
        );

        glm::mat4 mvp = projection * view * model;

        shader.Bind();
        shader.SetUniformMat4f("u_MVP", mvp);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

        window.Update();
    }

    return 0;
}