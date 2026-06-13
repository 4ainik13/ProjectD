#pragma once

#include <glad/glad.h>

class VBO
{
public:
	//Ссылка на буффер-объект (его ID в системе)
	GLuint ID;
	//Конструктор, генерирующий VBO и привязывающий к нему вершины
	VBO(const GLsizeiptr size, const GLfloat* vertices)
	{
		glGenBuffers(1, &ID);
		glBindBuffer(GL_ARRAY_BUFFER, ID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}
	//Констуркто по умолчанию. Оставляет ID равынм 0
	VBO()
	{
		ID = 0;
	}
	//Осуществляем разметку данных, хранящихся в VBO.
	//index - атрибут вертекса (его номер в шейдере), который мы хотим разметить
	//attribSize - размер атрибута: количество значений, хранящихся в одном атрибуте (1 для атрибутов типа int, float и т.п., 3 для vec3 и т.д.)
	//type - тип значений атрибута
	//stride - расстояние от начала атрибута до его повторного появления, в байтах
	//offset - расстояние от 0-го атрибута до начала текущего, в байтах
	void VertexAttribPointer(GLuint index, GLuint attribSize, GLenum type, GLsizeiptr stride, void* offset)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, attribSize, type, GL_FALSE, stride, offset);
	}
	//Привязка VBO
	void Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, ID);
	}
	//Отвязка VBO
	void Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	//Удаление VBO
	void Delete()
	{
		glDeleteBuffers(1, &ID);
	}
};
