#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <direct.h>

namespace
{
    const unsigned int PBO_QUANT = 2;
}

namespace dir
{
    bool exists(const std::string& dir)
    {
        struct stat buffer;
        return (stat(dir.c_str(), &buffer) == 0);
    }

    void summon(const std::string& dir)
    {
        if (!exists(dir))
        {
            int res = _mkdir(dir.c_str());
            if (res != 0) std::cout << "Directory have failed to be summoned";
        }
    }
}

class ImageHandler
{
public:
    unsigned int imageCounter;
    unsigned int pboSize;
    unsigned int pboIndex;
    unsigned int pboNextIndex;
    GLuint pboArray[PBO_QUANT];

    //GLubyte* curent_pbo_ptr;

    ImageHandler(unsigned int pboSize)
    {
        this->imageCounter = 0;
        this->pboSize = pboSize;
        this->pboIndex = 0;
        this->pboNextIndex = 0;
        init_pboArray(PBO_QUANT, pboSize);
    }

    void saveImage_fromData(const unsigned char* data,
        const unsigned int& height, const unsigned int& width,
        const int channels = 3, std::string name = "testFromData")
    {
        std::string directory = "images\\";
        std::string filename = directory + name + ".bmp";

        dir::summon(directory);

        stbi_flip_vertically_on_write(1);
        stbi_write_bmp(filename.c_str(), width, height, channels, data);
    }

    void saveImage_fromData_counting(const unsigned char* data,
        const unsigned int& height, const unsigned int& width,
        const int channels = 3, std::string name = "testFromData",
        bool increment = true)
    {
        if (increment) imageCounter++;
        saveImage_fromData(data, height, width, channels, name + std::to_string(imageCounter));
    }

    //Сохраняем текущий кадр в формате bmp на компьютере.
    //Кадр получаем из pbo.
    void saveImage_fromScreen(const int& startX, const int& startY,
        const unsigned int& height, const unsigned int& width,
        const unsigned int& channels = 3, std::string name = "test")
    {
        GLubyte* pbo_ptr = map_screenPixelData(startX, startY, height, width);

        if (pbo_ptr)
        {
            saveImage_fromData(pbo_ptr, height, width, channels, name);

            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
    }

    //Сохраняем текущий кадр в формате bmp с подсчётом сохранённых кадров.
    //Полученный кадр будт иметь имя в формате nameX.bmp, где X - номер кадра
    void saveImage_fromScreen_counting(const int& startX, const int& startY,
        const unsigned int& height, const unsigned int& width,
        const unsigned int& channels = 3, std::string name = "test")
    {
        imageCounter++;
        saveImage_fromScreen(startX, startY, height, width, 3, name + std::to_string(imageCounter));
    }

private:
    //Генерируем pbo и задаём их размер
    void init_pboArray(const unsigned int& pbo_quant, const unsigned int& pbo_size)
    {
        glGenBuffers(pbo_quant, pboArray);
        for (unsigned int i = 0; i < pbo_quant; i++)
        {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pboArray[i]);
            glBufferData(GL_PIXEL_PACK_BUFFER, pbo_size, 0, GL_STREAM_READ);
        }
    }

    //Возвращаем указатель на данные пикселей с экрана.
    GLubyte* map_screenPixelData(const int& startX, const int& startY,
        const unsigned int& height, const unsigned int& width)
    {
        //Меням индексы pbo местами, чтобы считывать из ранее 
        //записанного pbo и записывать в ранее считанный
        pbo_iterateIndex();

        //Считываем пиксели с экрана в буфер пикселей GL_PIXEL_PACK_BUFFER
        pbo_readPixels(startX, startY, width, height, pboIndex);

        //Получаем указатель на данные с экрана, считанные в прошлом цикле
        return pbo_mapBuffer(pboNextIndex);
    }

    void pbo_iterateIndex()
    {
        pboIndex = (pboIndex + 1) % 2;
        pboNextIndex = (pboIndex + 1) % 2;
    }

    void pbo_readPixels(const int& startX, const int& startY,
        const unsigned int& height, const unsigned int& width,
        const unsigned int& pboArrayIndex)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboArray[pboArrayIndex]);
        glReadPixels(startX, startY, width, height, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
    }

    GLubyte* pbo_mapBuffer(const unsigned int& pboArrayIndex)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboArray[pboArrayIndex]);
        return (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    }

    void pbo_unmapBuffer()
    {

    }
};