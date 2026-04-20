#pragma once
#include "ndc.h"
#include "pixels_data.h"
#include "pixel.h"

using namespace pixel_const;

class PixelsHandler
{
public:
	unsigned int height;
	unsigned int width;
	unsigned int lowerY;
	unsigned int lowerX;
	unsigned int channels;
	unsigned int size;
	
	PixelsData pixelsData;

	PixelsHandler(unsigned int& height, unsigned int& width, unsigned int& lowerY,
		unsigned int& lowerX, unsigned int& channels)
	{
		this->height = height;
		this->width = width;
		this->lowerY = lowerY;
		this->lowerX = lowerX;
		this->channels = channels;
		this->size = height * width;
		this->pixelsData.init(height, width, channels);
	}

	//¬озвращает пиксель из pixelsData по указанному индексу
	pixel getPixel(int index)
	{
		return pixel{
			global_pixelsData[index],
			global_pixelsData[index + 1],
			global_pixelsData[index + 2]
		};
	}


private:
};