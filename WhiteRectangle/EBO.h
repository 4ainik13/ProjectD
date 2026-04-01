#pragma once

#include <glad/glad.h>

class EBO
{
public:
	//Ссылка на EBO
	GLuint ID;
	//Констуркто по умолчанию. Оставляет ID равынм 0
	EBO();
	//Конструктор, генерирующий EBO и заполняющий буффер данными из массива indices
	EBO(const GLuint* indices, const GLsizeiptr size);
	//Привязка EBO
	void Bind();
	//Отвязка EBO
	void Unbind();
	//Удаление EBO
	void Delete();
};