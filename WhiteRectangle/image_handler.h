#pragma once
#include <glad/glad.h>
#include <iostream>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <direct.h>

#include "pixel_buffer_object_array.h"
#include "denoise_alg.h"
#include "global_vars.h"

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
    PboArray pboArray;

    ImageHandler()
    {
        this->imageCounter = 0;
    }

    ImageHandler(unsigned int sizeOfPbos, unsigned int numberOfPbos)
    {
        this->imageCounter = 0;
        pboArray.init(sizeOfPbos, numberOfPbos);
    }

    unsigned char* mapPixelDataPointer()
    {
        return pboArray.mapBuffer_curent();
    }

    void unmapPixelDataPointer()
    {
        pboArray.unmapBuffer();
    }

    //Читаем пиксели с экрана впервый раз, чтобы заполнить пустой pbo.
    void initialPixelsRead(const int& startX, const int& startY,
        const unsigned int& height, const unsigned int& width)
    {
        GLubyte* bufferData_ptr = pboArray.readPixels(startX, startY, height, width);
        if(bufferData_ptr) pboArray.unmapBuffer();
    }

    //Сохраняем картинку из pbo несколькими способами.
    //После выполнения функции, данные 
    //изображения, которое хранится в pbo, могут измениться.
    void saveImage_differentWays(const int& startX, const int& startY,
        const unsigned int& height, const unsigned int& width,
        const unsigned int& channels = 3, std::string name = "test", bool doSave = true)
    {
        GLubyte* bufferData_ptr = pboArray.readPixels(startX, startY, height, width);

        if (bufferData_ptr)
        {
            countImage();
            name = nameCount(name);

            saveImage_fromData(bufferData_ptr, height, width, channels, name, doSave);
            denoiseImage_fromData(bufferData_ptr, height, width, channels, "denoise" + name, doSave);
            

            pboArray.unmapBuffer();
        }
    }

    void saveImage_fromData(const unsigned char* data,
        const unsigned int& height, const unsigned int& width,
        const int channels = 3, std::string name = "testFromData", bool doSave = true)
    {
        if (!doSave) return;

        std::string directory = "images\\";
        std::string filename = directory + name + ".bmp";

        dir::summon(directory);

        stbi_flip_vertically_on_write(1);
        stbi_write_bmp(filename.c_str(), width, height, channels, data);
    }

    void saveImage_fromData_counting(const unsigned char* data,
        const unsigned int& height, const unsigned int& width,
        const int channels = 3, std::string name = "testFromData",
        bool increment = true, bool doSave = true)
    {
        countImage(increment);
        saveImage_fromData(data, height, width, channels, name + std::to_string(imageCounter), doSave);
    }

    //Сохраняем текущий кадр в формате bmp на компьютере.
    //Кадр получаем из pbo.
    void saveImage_fromScreen(const int& startX, const int& startY,
        const unsigned int& height, const unsigned int& width,
        const unsigned int& channels = 3, std::string name = "test")
    {
        GLubyte* bufferData_ptr = pboArray.readPixels(startX, startY, height, width);

        if (bufferData_ptr)
        {
            saveImage_fromData(bufferData_ptr, height, width, channels, name);

            pboArray.unmapBuffer();
        }
    }

    //Сохраняем текущий кадр в формате bmp с подсчётом сохранённых кадров.
    //Полученный кадр будт иметь имя в формате nameX.bmp, где X - номер кадра
    void saveImage_fromScreen_counting(const int& startX, const int& startY,
        const unsigned int& height, const unsigned int& width,
        const unsigned int& channels = 3, std::string name = "test")
    {
        countImage();
        saveImage_fromScreen(startX, startY, height, width, 3, name + std::to_string(imageCounter));
    }

    void denoiseImage_fromData(unsigned char* data,
        const unsigned int& height, const unsigned int& width,
        const int channels = 3, std::string name = "denoiseFromData", bool doSave = true)
    {
        alg::denoiseImage(data, height, width, channels);
        saveImage_fromData(data, height, width, channels, name, doSave);
    }
private:
    void countImage(bool increment = true)
    {
        if (increment)
        {
            global::currentImage = imageCounter;
            imageCounter++;
        }
    }

    std::string nameCount(std::string& name)
    {
        return name + std::to_string(imageCounter);
    }
};