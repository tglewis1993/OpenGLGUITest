#pragma once

#include <glew/glew.h>
#include <iostream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GL_CALL(x) ClearGLErrors();\
	x;\
	ASSERT(LogGLCall(#x, __FILE__, __LINE__))


static void ClearGLErrors()
{
	while (glGetError() != GL_NO_ERROR);
}

static bool LogGLCall(const char* func, const char* file, const unsigned int& lineNum)
{
	while (GLenum err = glGetError())
	{
		std::cout << "OpenGL Error (" << err << "): " << func << " " << file << ":" << lineNum << std::endl;

		return false;
	}

	return true;
}
