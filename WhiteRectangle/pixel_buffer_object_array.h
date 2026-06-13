#pragma once
#include <glad/glad.h>
#include <vector>

using std::vector;

class PboArray
{
public:

	GLuint index;
	GLuint nextIndex;

	GLuint size;
	vector<GLuint> ids;

	PboArray()
	{
		size = 0;
		index = 0;
		nextIndex = 0;
	}

	void init(GLuint sizeOfPbos, GLuint numberOfPbos)
	{
		size = sizeOfPbos;
		ids.resize(numberOfPbos);
		generatePbos(sizeOfPbos, numberOfPbos);
	}

	void bind(GLuint pboIndex)
	{
		if (pboIndex >= ids.size()) return;
		glBindBuffer(GL_PIXEL_PACK_BUFFER, ids[pboIndex]);
	}

	void unbind()
	{
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

	GLubyte* mapBuffer(GLuint pboIndex)
	{
		bind(pboIndex);
		return (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	}

	GLubyte* mapBuffer_curent()
	{
		return mapBuffer(nextIndex);
	}

	void unmapBuffer()
	{
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}

	GLubyte* readPixels(GLuint startX, GLuint startY, GLuint height, GLuint width)
	{
		index = (index + 1) % 2;
		nextIndex = (index + 1) % 2;
		
		bind(index);
		glReadPixels(startX, startY, width, height, GL_RGB, GL_UNSIGNED_BYTE, 0);

		bind(nextIndex);
		return (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	}

	GLuint operator[](const unsigned int& pboIndex) const
	{
		return ids[pboIndex];
	}

private:
	void generatePbos(GLuint sizeOfPbos, GLuint numberOfPbos)
	{
		glGenBuffers(numberOfPbos, ids.data());
		for (GLuint i = 0; i < numberOfPbos; i++)
		{
			bufferPboSize(sizeOfPbos, i);
		}
	}

	void bufferPboSize(GLuint sizeToBuffer, GLuint pboIndex)
	{
		bind(pboIndex);
		glBufferData(GL_PIXEL_PACK_BUFFER, sizeToBuffer, 0, GL_STREAM_READ);
	}
};
