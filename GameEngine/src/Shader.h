#pragma once

#include <string>
#include <unordered_map>
#include "glm/glm.hpp"

class Shader {
private:
    unsigned int m_RendererID;
    std::unordered_map<std::string, int> m_UniformLocationCache;

public:
    Shader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath);
    ~Shader();

    void Bind() const;
    void Unbind() const;
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
    void SetUniformVec3f(const std::string& name, const glm::vec3& vector);
    void SetUniform1i(const std::string& name, int value);

private:
    int GetUniformLocation(const std::string& name);
};