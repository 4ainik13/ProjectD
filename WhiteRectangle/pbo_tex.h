#pragma once
#include <glad/glad.h>

namespace tex
{
	GLuint createShaderProgram();
	void genTexBuffer();
	void genVertBuffers();
	void drawElements();
	void freeResources();
}