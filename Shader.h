#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // 必须在 glfw3.h 之前
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

// 着色器类，用于编译和链接顶点着色器和片段着色器
class Shader
{
public:
	unsigned int ID; // 着色器程序ID

	Shader(const char* vertexPath, const char* fragmentPath); // 构造函数，读取并编译着色器

	void use(); // 激活着色器程序

	void setMat4(const std::string& name, const glm::mat4& mat) const; // 设置4x4矩阵统一变量

private:
	void checkCompileErrors(unsigned int shader, std::string type); // 检查编译和链接错误
};
#endif