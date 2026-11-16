#include "Renderer.h"

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"
#include "Shader.h"
#include "TextureManager.h"
#include "Window.h"

int main()
{
    Window window(720, 720, "Textured model");
    if (!window.IsOpen())
        return -1;

    glEnable(GL_DEPTH_TEST);

    Model model("./res/source/crashbandicoot.obj");
    Shader shader("./res/shaders/vert.glsl", "./res/shaders/frag.glsl");

    glClearColor(0.08f, 0.08f, 0.12f, 1.0f);

    while (window.IsOpen())
    {
        window.Clear();

        const float time = static_cast<float>(glfwGetTime());

        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.6f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, time * 0.75f, glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.024f));

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -6.0f));
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), 720.0f / 720.0f, 0.1f, 100.0f);
        glm::mat4 mvp = projection * view * modelMatrix;

        shader.Bind();
        shader.SetUniformMat4f("u_MVP", mvp);
        shader.SetUniform1i("u_Texture", 0);

        model.Draw(shader);
        window.Update();
    }

    return 0;
}
