#pragma once
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <string>

class ShaderHandler
{
public:
	const char* vertexPath = "";
	const char* fragmentPath = "";

	ShaderHandler(const char* vertexPath, const char* fragmentPath)
	{
		this->vertexPath = vertexPath;
		this->fragmentPath = fragmentPath;
		
		createVertexShader(vertexPath);
		createFragmentShader(fragmentPath);
		createShaderProgram();
		deleteShaders();
	}

	GLuint getShaderProgram()
	{
		return shaderProgram_ID;
	}


private:
	GLuint shaderProgram_ID = 0;

	GLuint vertexShader_ID = 0;
	GLuint fragmentShader_ID = 0;

	//Создаём Vertex шейдер
	void createVertexShader(const char* vertexPath)
	{
		createShader(vertexPath, vertexShader_ID, GL_VERTEX_SHADER);
	}

	//Создаём Fragment шейдер
	void createFragmentShader(const char* vertexPath)
	{
		createShader(vertexPath, fragmentShader_ID, GL_FRAGMENT_SHADER);
	}

	//Создаём шейдерную программу
	void createShaderProgram()
	{
		shaderProgram_ID = glCreateProgram();
		glAttachShader(shaderProgram_ID, vertexShader_ID);
		glAttachShader(shaderProgram_ID, fragmentShader_ID);
		glLinkProgram(shaderProgram_ID);
		programLinkingErrors(shaderProgram_ID);
	}

	//Удалям использованные шейдеры
	void deleteShaders()
	{
		glDeleteShader(vertexShader_ID);
		glDeleteShader(fragmentShader_ID);
	}

	//Обобщённая функция создания шейдеров
	void createShader(const char* shaderPath, GLuint& shaderID, GLenum shaderType)
	{
		std::string shader_string = readShaderFile(shaderPath);
		const char* shader_cString = shader_string.c_str();

		shaderID = glCreateShader(shaderType);
		glShaderSource(shaderID, 1, &shader_cString, NULL);
		glCompileShader(shaderID);
		shaderCompileErrors(shaderID);
	}

	//Вывод ошибок компиляции шейдера
	void shaderCompileErrors(GLuint& shaderID)
	{
		//Флаг успешной компиляции
		GLint hasCompiled;
		//Массив под сообщение об ошибке
		char infoLog[1024];

		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR\n" << infoLog;
		}
	}

	//Вывод ошибок связывания программы
	void programLinkingErrors(GLuint& programID)
	{
		//Флаг успешной компиляции
		GLint hasCompiled;
		//Массив под сообщение об ошибке
		char infoLog[1024];

		glGetProgramiv(programID, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(programID, 1024, NULL, infoLog);
			std::cout << "PROGRAM_LINKING_ERROR for\n" << infoLog;
		}
	}

	//Чтение шейдера из файла
	std::string readShaderFile(std::string shaderPath)
	{
		std::ifstream in_shaderStream(shaderPath);

		std::string nextLine;
		std::string shaderText;
		
		if (in_shaderStream.is_open())
		{
			while (std::getline(in_shaderStream, nextLine))
			{
				shaderText.append(nextLine+"\n");
			}
			in_shaderStream.close();
		}
		else
		{
			std::cerr << "No shader with path " << shaderPath << " was found\n";
			return "";
		}

		return shaderText;
	}
};
