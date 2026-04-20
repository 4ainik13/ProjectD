#pragma once
#include <vector>
#include "pixel.h"
#include "matrix.h"

using namespace pixel_const;
using std::vector;

//PixelsData представляет из себя трёхмерный массив, записанный в одной строчке.
//Этот массив имеет размер heigth * width * channels.
//heigth и width - соответственно высота и ширина картинки.
//channels - количество использованных цветов, обычно используется 3 цвета.
class PixelsData
{
public:
	unsigned int height;
	unsigned int width;
	unsigned int channels;
	unsigned int size;

	vector<vector<vector<unsigned char>>> data;

	PixelsData() : height(0), width(0), channels(0), size(0) {}

	pixel operator[](unsigned int index) const
	{
		return pixel
		{

		}
	}

	void init(unsigned int height, unsigned int width, unsigned int channels)
	{
		this->height = height;
		this->width = width;
		this->channels = channels;
		assign(height, width, channels);
	}

	void assign(unsigned int height, unsigned int width, unsigned int channels)
	{
		this->size = height * width * channels;
		data.assign(height, vector<vector<unsigned char>>(width, vector<unsigned char>(channels)));
	}

private:

};
