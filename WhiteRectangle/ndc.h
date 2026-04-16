#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

//NDC = Normalized Device Coordinate, Нормализованная координата устройства
//Viewport - это отображаемое при запуске программы окно

namespace NDC
{
	//Превращаем Нормализованную координату устройства в координату viewport
	//viewportSize - размер оси окна, вдоль которой мы хотим найти нку в координатах viewport
	float to_viewport(float ndc, float viewportSize);

	//Превращем две нормализованные координаты устройства (x и y) в соответсвующие координаты viewport.
	//Viewport имеет разные длину и ширину.
	vec2 twoDimsension_to_viewport(vec2 ndcs, float viewportWidth, float viewportHeight);

	//Превращем две нормализованные координаты устройства (x и y) в соответсвующие координаты viewport.
	//Viewport имеет одинаковую длину и ширину.
	//viewportSize - размер осей окна, вдоль которых мы хотим найти координаты нку в координатах viewport
	vec2 oneDimsension_to_viewport(vec2 ndcs, float viewportSize);

	//Returns dimension value between two Normalized Device Coordinates of one dimension
	//Возвращает расстояние между двмуя нормализованными координатами.
	//Расстояние возвращается в формате viewport.
	//Координаты должны принадлежать одной координатной оси.
	//viewportSize - размер оси окна, вдоль которой мы хотим найти расстояние между двумя нку в формате viewport
	GLuint to_dimension(float firstCoord, float secondCoord, float viewportSize);
}