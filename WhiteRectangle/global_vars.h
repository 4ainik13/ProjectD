#pragma once
#include <glad/glad.h>
#include "ndc.h"

namespace
{
	typedef unsigned int uint;
}

namespace global
{
	const GLuint SCR_WIDTH = 800;
	const GLuint SCR_HEIGHT = 600;
	const GLuint CLR_CHANNELS = 3;

	const uint FRAME_WIDTH = NDC::to_dimension(-0.5f, 0.5f, SCR_WIDTH) / 2; //БАГ в to_dimension. Почему-то получается в 2 раза больше. Поэтому здесь делим на 2
	const uint FRAME_HEIGHT = NDC::to_dimension(-0.5f, 0.5f, SCR_HEIGHT) / 2;
	const uint FRAME_X = NDC::to_viewport(-0.5f, SCR_WIDTH);
	const uint FRAME_Y = NDC::to_viewport(-0.5f, SCR_HEIGHT);

	const uint BMP_WIDTH = 200;		//Изначально 160 200 400
	const uint BMP_HEIGHT = 200;	//Изначально 160 200 300

	extern uint noiseCount;
	extern uint minNoiseCount;
	extern uint currentImage;
	extern uint bestImage;
}