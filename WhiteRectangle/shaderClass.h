#pragma once
#include <glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

//Функция, считывающая файл в строку
std::string get_file_contents(const char* fileName);

class Shader
{
public:
	//Ссылка на программу шейдеров
	GLuint ID;
	//Конструктор. Принимает пути к шейдерам
	Shader(const char* vertexFile, const char* fragmentFile);
	//Пустой конструктор. ID = 0
	Shader();
	//Активация программы
	void Activate();
	//Удаление программы
	void Delete();

private:
	//Отчёт об ошибках
	void compileErrors(GLuint shader, const char* type);
};