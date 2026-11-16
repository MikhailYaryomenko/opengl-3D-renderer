#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
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
        std::ifstream test(candidate);
        if (test.is_open())
            return candidate;
    }

    return filepath;
}

std::string getShaderContent(const std::string& filepath)
{
    const std::string resolvedPath = ResolveAssetPath(filepath);
    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open shader source file: " << filepath << std::endl;
        return {};
    }

    std::stringstream source;
    source << file.rdbuf();
    return source.str();
}

void CheckShaderCompileStatus(unsigned int shader, const std::string& type)
{
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "[ERROR] Shader compile failure (" << type << "):\n" << infoLog << std::endl;
    }
}

void CheckProgramLinkStatus(unsigned int program)
{
    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "[ERROR] Program link failure:\n" << infoLog << std::endl;
    }
}
}

Shader::Shader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath)
    : m_RendererID(0)
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderContent = getShaderContent(vertexShaderFilepath);
    const char* vertexShaderSource = vertexShaderContent.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    CheckShaderCompileStatus(vertexShader, "vertex");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderContent = getShaderContent(fragmentShaderFilepath);
    const char* fragmentShaderSource = fragmentShaderContent.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    CheckShaderCompileStatus(fragmentShader, "fragment");

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    CheckProgramLinkStatus(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    m_RendererID = program;
}

Shader::~Shader()
{
    glDeleteProgram(m_RendererID);
}

void Shader::Bind() const
{
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetUniformVec3f(const std::string& name, const glm::vec3& vector)
{
    glUniform3fv(GetUniformLocation(name), 1, &vector[0]);
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    int location = glGetUniformLocation(m_RendererID, name.c_str());
    if (location == -1)
        std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;

    m_UniformLocationCache[name] = location;
    return location;
}
