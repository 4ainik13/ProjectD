#include "matrix.h"
#include "ndc.h"
#include "denoise_alg.h"
#include "global_vars.h"

namespace
{
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;

    int pixelsW = NDC::to_dimension(-0.5f, 0.5f, SCR_WIDTH) / 2; //БАГ в to_dimension. Почему-то получается в 2 раза больше. Поэтому здесь делим на 2
    int pixelsH = NDC::to_dimension(-0.5f, 0.5f, SCR_HEIGHT) / 2;

    const int global_pixels_channels = 3;
    const int global_pixels_size = pixelsW * pixelsH;

    unsigned char* global_pixelsMask_previous = new unsigned char[global_pixels_size] {};
    unsigned char* global_pixelsMask_curent = new unsigned char[global_pixels_size] {};

    bool masksAreInited = false;

    struct pixel
    {
        unsigned char r, g, b;
    };
    const pixel red{ 255, 0, 0 }, black{ 0, 0, 0 }, white{ 255, 255, 255 };

    bool operator == (const pixel& p1, const pixel& p2)
    {
        return p1.r == p2.r && p1.g == p2.g && p1.b == p2.b;
    }

    bool operator != (const pixel& p1, const pixel& p2)
    {
        return p1.r != p2.r || p1.g != p2.g || p1.b != p2.b;
    }
    
    void init_byte_array(unsigned char* arr, int size, unsigned char initVal)
    {
        std::memset(arr, initVal, size);
    }

    //Копирует значения из одного массива в другой. Массивы должны иметь одинковый размер
    void copy_byte_array(const unsigned char* arrGiver, unsigned char* arrTaker, int size)
    {
        std::memcpy(arrTaker, arrGiver, size);
    }

    //Возвращает пиксель из массива data по указанному индексу
    pixel getPixel(const unsigned char* data, int index)
    {
        return pixel{
            data[index],
            data[index + 1],
            data[index + 2]
        };
    }

    //Задаёт указанный пиксель pix в трёхмерном массиве data по индексу index.
    //Сам index должен быть представлен как абсолютный индекс массива data, указывающий на 0-ой элемент, относящийся к цвету
    void setPixel(unsigned char* data, int index, pixel& pix)
    {
        data[index] = pix.r;
        data[index + 1] = pix.g;
        data[index + 2] = pix.b;
    }

    //Меняем пиксель по адресу index со смешиванием пикселя pix
    void blendPixel(unsigned char* data, int index, pixel& pix)
    {
        pixel pixAtIndex = getPixel(data, index);
        if (pixAtIndex == white)
        {
            setPixel(data, index, pix);
        }
    }

    //Помечаем пиксель по координатам width_index и height_index в маске при помощи 1
    void markPixel(int height_index, int width_index, int width)
    {
        int rawIndex = mat2D::getRawIndex(height_index, width_index, width);
        global_pixelsMask_curent[rawIndex] = 1;
    }

    //Помечаем всех соседей пикселя с координатами width_index и height_index
    void markNeighbors(int height_index, int width_index, int height, int width)
    {
        for (int i = height_index - 1; i <= height_index + 1; i++)
        {
            if (i < 0) continue;
            if (i >= height) break;
            for (int j = width_index - 1; j <= width_index + 1; j++)
            {
                if (j < 0) continue;
                if (j >= width) break;
                markPixel(i, j, width);
            }
        }
    }

    //Накладываем маску на картинку, которая хранится в data
    void applyMask_to_pixelData(unsigned char* data, const unsigned char* mask, int height, int width, int channels)
    {
        pixel pix{ 0, 255, 0 };
        int maskIndex;
        int pixelIndex;
        unsigned char maskVal;

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                //Получаем индекс точки (i,j) в маске
                maskIndex = mat2D::getRawIndex(i, j, width);
                //Получаем значение из маски
                maskVal = global_pixelsMask_curent[maskIndex];
                if (maskVal == 1)
                {
                    //Получаем индекс точки (i,j) в массиве изображения data
                    pixelIndex = mat3D::getRawIndex(i, j, 0, width, channels);
                    //Расскрашиваем пиксель точки (i,j) в зелённый цвет
                    blendPixel(data, pixelIndex, pix);
                }
            }
        }
    }
}

namespace alg
{
    //Инициализируем маски
    void initMasks()
    {
        init_byte_array(global_pixelsMask_previous, global_pixels_size, 1);
        init_byte_array(global_pixelsMask_curent, global_pixels_size, 0);
        masksAreInited = true;
    }

    void denoiseImage(unsigned char* data, int height, int width, int channels)
    {
        int pixelIndex;
        global::noiseCount = 0;

        if (!masksAreInited) initMasks();

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                //Если точка (i,j) не находится в области возможного движения, 
                //пропускаем иттерацию
                if (global_pixelsMask_previous[mat2D::getRawIndex(i, j, width)] == 0)
                    continue;

                //До этого момента мы доходим только в том случае, 
                //если точка лежит в области возможного движения

                //Индекс точки (i, j) в массиве data (трёхмерный массив изображения)
                pixelIndex = mat3D::getRawIndex(i, j, 0, width, channels);
                //Проверяем, имеет ли цвет пиксель по полученному индексу
                if (getPixel(data, pixelIndex) != white)
                {
                    //Глобальная переменная. Считаем +1 к помехам
                    global::noiseCount++;
                    //Помечаем 9 прилежащих пикселей, как область возможного движения
                    markNeighbors(i, j, height, width);
                }
            }
        }

        if (global::noiseCount < global::minNoiseCount)
        {
            global::minNoiseCount = global::noiseCount;
            global::bestImage = global::currentImage;
        }

        //Переносим информацию из маски на текущее изображение
        applyMask_to_pixelData(data, global_pixelsMask_curent, height, width, channels);

        //Запоминаем текущую маску в качестве предыдущей
        copy_byte_array(global_pixelsMask_curent, global_pixelsMask_previous, global_pixels_size);
        //Заполняем текущую маску нулями
        init_byte_array(global_pixelsMask_curent, global_pixels_size, 0);
    }
}