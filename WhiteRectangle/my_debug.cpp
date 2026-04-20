#include <glad/glad.h>
#include <iostream>
#include "my_debug.h"

namespace
{
	std::string tab = "\t";
	bool badError = 0;

	std::string getGlErrorName(GLenum GlError)
	{
		switch (GlError)
		{
		case GL_NO_ERROR:
			badError = 1;
			return "GL_NO_ERROR";
		case GL_INVALID_ENUM:
			badError = 0;
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			badError = 0;
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			badError = 0;
			return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY:
			badError = 0;
			return "GL_OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			badError = 0;
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		default:
			badError = 1;
			return "not an error";
		}
	}

	void contextTab(std::string context)
	{
		if (context.length() > 0)
			std::cout << context << "\n" << tab;
	}
	void contextNoTab(std::string context)
	{
		if (context.length() > 0)
			std::cout << context << "\n";
	}
}

namespace dbg
{
	void printGlError(GLenum GlError, std::string context)
	{
		
		std::string startMsg = "CURENT GL ERROR:";
		std::string errorName = getGlErrorName(GlError);
		contextTab(context);
		std::cout << startMsg << " " << errorName << "\n";
	}

	void printAllGlErrors(std::string context)
	{
		badError = 0;
		contextNoTab(context);
		while(badError == 0)
		{
			std::cout << tab;
			printGlError(glGetError(), "");
		}
		std::cout << "\n";
	}
}