#pragma once

#include<glad/glad.h>
#include"VBO.h"

class VAO
{
public:
	//Ссылка на VAO 
	GLuint ID;
	//Конструктор VAO, который лишь генерирует ссылку
	VAO();
	//Связываем VAO с местом памяти, отвечающим за обработку функционала VAO
	void Bind();
	//Отвязываем VAO
	void Unbind();
	//Удаляем VAO
	void Delete();
};