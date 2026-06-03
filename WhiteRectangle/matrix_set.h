#pragma once
#include <cstring>
#include "matrix.h"

namespace
{
	typedef unsigned int uint;
	typedef unsigned char byte;
}

class MatrixSet
{
public:

	MatrixSet()
	{
		height = width = setSize = matrixSize = 0;
		data = nullptr;
	}

	MatrixSet(uint height, uint width, uint setSize)
	{
		this->height = height;
		this->width = width;
		this->setSize = setSize;

		matrixSize = height * width * setSize;
		data = new byte[matrixSize]{};
	}

	~MatrixSet()
	{
		delete[] data;
		data = nullptr;
		height = width = setSize = matrixSize = 0;
	}

	//¬озвращаем указатель на данные
	byte* getData()
	{
		return data;
	}

	//¬озвращаем указатель на неизменные данные
	const byte* getConstData() const
	{
		return data;
	}

	//¬озвращаем указатель на незменное множество по координатам x, y
	const byte* getConstSet(const uint x, const uint y) const
	{ 
		return data + rawIndex(x, y, 0);
	}

	//«адаем новый указатель на данные
	void setData(byte* newData)
	{
		data = newData;
	}

	//¬озвращаем размер матрицы
	uint size() const
	{
		return matrixSize;
	}

	//ƒобавл€ем val в множество, расположенное по координатам x, y
	//¬ случае успеха возвращаем 1. »наче 0.
	int add(const uint x, const uint y, const byte val)
	{
		uint setIndex = rawIndex(x, y, 0);
		for (uint i = 0; i < setSize; i++)
		{
			if (data[setIndex + i] == val) break;
			if (data[setIndex + i] == 0)
			{
				data[setIndex + i] = val;
				return 1;
			}
		}
		return 0;
	}

	//»нициализируем все множества одним значением.
	void initializeSets(const byte initVal = 5)
	{
		clear();
		for (uint i = 0; i < height; i++)
		{
			for (uint j = 0; j < width; j++)
			{
				add(j, i, initVal);
			}
		}
	}

	//ќчищаем матрицу. ћожем задать начальное значение дл€ очистки.
	void clear(const byte initVal = 0)
	{
		std::memset(data, initVal, matrixSize);
	}

	// опируем матрицу src в текущую матрицу. src должна быть не меньше текущей матрицы
	void copy(const MatrixSet& src)
	{
		if (!data) return;
		if (matrixSize <= src.size()) 
			std::memcpy(data, src.getConstData(), matrixSize);
	}

	//ћен€ем текущую матрицу и матрицу src местами. ќбе матрицы должны иметь одинаковый размер больше нул€
	void swap(MatrixSet& src)
	{
		if (!data) return;
		if (matrixSize == src.size())
		{
			byte* buf = data;
			setData(src.getData());
			src.setData(buf);
		}
	}

	//¬озвращаем ссылку на значение в матрице по координатам x, y, z
	byte& at(const uint x, const uint y, const uint z)
	{
		return data[rawIndex(x, y, z)];
	}

	//¬озвращаем необработанный индекс матрицы по координатам x, y, z
	uint rawIndex(const uint x, const uint y, const uint z) const
	{
		//y должен быть на первом месте. Ѕудем считать, что строительство матрицы data начинаетс€ по направлению y.
		return mat3D::getRawIndex(y, x, z, width, setSize);
	}

	//ѕусто ли множество по координатам x, y
	bool empty(const uint x, const uint y) const
	{
		uint rawInd = rawIndex(x, y, 0);
		for (uint i = 0; i < setSize; i++)
		{
			if (data[rawInd + i] != 0) return false;
		}
		return true;
	}

private:
	uint height, width, setSize, matrixSize;
	byte* data;

};