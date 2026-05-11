#pragma once
#include <GLFW/glfw3.h>

class Stopwatch
{
public:
	double tickInterval = 1;

	//Возвращаем количество оставшихся тиков
	int getTicksLeft()
	{
		return ticksLeft;
	}

	//Задаём длину одного тика в секундах и количество тиков
	void set(double tickInterval, int tickLimit = 8)
	{
		this->tickInterval = tickInterval;
		this->ticksLeft = tickLimit;
	}

	//Запускаем таймер, запоминаем время запуска
	void start()
	{
		started = true;
		lastTickTime = glfwGetTime();
	}

	//Задаём время одного тика, количество тиков и запускаем таймер
	void setStart(double tickInterval, int tickLimit = 8)
	{
		set(tickInterval, tickLimit);
		start();
	}

	//Проверка на начало нового тика.
	bool ticked()
	{
		if (!started) return false;
		if (noTicks()) return false;

		double curentTime = glfwGetTime();
		if (curentTime - lastTickTime >= tickInterval)
		{
			lastTickTime = curentTime;
			ticksLeft--;
			return true;
		}
		return false;
	}

	//Проверяем, кончились ли уже тики
	bool noTicks()
	{
		return ticksLeft <= 0;
	}

private:
	double lastTickTime = 0;
	int ticksLeft = 0;
	bool started = false;
};