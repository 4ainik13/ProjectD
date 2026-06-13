#include "matrix.h"
#include "ndc.h"
#include "denoise_alg.h"
#include "global_vars.h"
#include "pixel_struct.h"
#include "matrix_set.h"

using namespace std;

namespace
{
    //Таблица границ для циклов в markNeighbors
    //table[a][5], table[b][9], table[c][1] и т.д.
    int boundariesTable[4][9] =
    {    /* 1   2   3   4   5   6   7   8  9 */
    /*a*/ {-1, -1, -1, -1, -1, -1,  0,  1, 0},
    /*b*/ { 0, -1,  0,  1,  1,  1,  1,  1, 1},
    /*c*/ {-1, -1,  0, -1, -1,  1, -1, -1, 0},
    /*d*/ { 0,  1,  1, -1,  1,  1,  0,  1, 1},
    };

    //int frameWidth = NDC::to_dimension(-0.5f, 0.5f, global::SCR_WIDTH) / 2; //БАГ в to_dimension. Почему-то получается в 2 раза больше. Поэтому здесь делим на 2
    //int frameHeight = NDC::to_dimension(-0.5f, 0.5f, global::SCR_HEIGHT) / 2;
    int frameWidth = global::BMP_WIDTH;
    int frameHeight = global::BMP_HEIGHT;
    int setSize = 9;

    const int frameSize = frameWidth * frameHeight;
    
    MatrixSet previousFrameDirections(frameHeight, frameWidth, setSize);
    MatrixSet curentFrameDirections(frameHeight, frameWidth, setSize);

    bool directionsAreInited = false;

    //Помечаем направление пикселя по координатам width_index и height_index в матрице curentFrameDirections при помощи dir
    void markPixel(int height_index, int width_index, int dir)
    {
        curentFrameDirections.add(width_index, height_index, dir);
    }

    //Помечаем всех соседей пикселя с координатами width_index и height_index по направлению dir
    void markNeighbors(int height_index, int width_index, int height, int width, int dir)
    {
        if (dir == 0) return;

        int a, b, c, d, newDir;
        a = boundariesTable[0][dir - 1];
        b = boundariesTable[1][dir - 1];
        c = boundariesTable[2][dir - 1];
        d = boundariesTable[3][dir - 1];

        //Оставляем в исходном пикселе исходное направление
        markPixel(height_index, width_index, dir);

        //Заносим новое направление в прилежащие пиксели.
        for (int i = height_index + a; i <= height_index + b; i++)
        {
            if (i < 0) continue;
            if (i >= height) break;
            for (int j = width_index + c; j <= width_index + d; j++)
            {
                if (j < 0) continue;
                if (j >= width) break;
                if (i == height_index && j == width_index) continue;
                newDir = mat2D::getRawIndex(i - height_index + 1, j - width_index + 1, 3) + 1;
                markPixel(i, j, newDir);
            }
        }
    }

    //Накладываем данные из матрицы направлений текущего кадра на изображение data
    void applyMask_to_pixelData(unsigned char* data, int height, int width, int channels)
    {
        int pixelIndex;

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (!curentFrameDirections.empty(j, i))
                {
                    //Получаем индекс точки (i,j) в массиве изображения data
                    pixelIndex = mat3D::getRawIndex(i, j, 0, width, channels);
                    //Расскрашиваем пиксель точки (i,j) в зелённый цвет
                    colorPixel(data, pixelIndex, green);
                }
            }
        }
    }

    namespace denoise
    {
        //Убираем шум
        void applyMask_to_pixelData(unsigned char* data, int height, int width, int channels)
        {
            int pixelIndex;

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    if (curentFrameDirections.empty(j, i))
                    {
                        //Получаем индекс точки (i,j) в массиве изображения data
                        pixelIndex = mat3D::getRawIndex(i, j, 0, width, channels);
                        if (getPixel(data, pixelIndex) == white) continue;
                        //Расскрашиваем пиксель точки (i,j) в белый цвет
                        setPixel(data, pixelIndex, white);
                    }
                }
            }
        }
    }
}

namespace alg
{
    //Инициализируем маски
    void beta::initMasks()
    {
        previousFrameDirections.initializeSets(5);
        curentFrameDirections.clear(0);
        directionsAreInited = true;
    }

    void beta::denoiseImage(unsigned char* data, int height, int width, int channels, bool saveImage)
    {
        int pixelIndex;
        const unsigned char* directionsSet;
        global::noiseCount = 0;

        if (!directionsAreInited) initMasks();

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                //Если точка (i,j) не находится в области возможного движения, 
                //пропускаем иттерацию
                if (previousFrameDirections.empty(j, i)) continue;

                //До этого момента мы доходим только в том случае, 
                //если точка лежит в области возможного движения

                //Индекс точки (i, j) в массиве data (трёхмерный массив изображения)
                pixelIndex = mat3D::getRawIndex(i, j, 0, width, channels);
                //Проверяем, имеет ли цвет пиксель по полученному индексу
                if (getPixel(data, pixelIndex) != white)
                {
                    //Глобальная переменная. Считаем +1 к помехам
                    global::noiseCount++;

                    directionsSet = previousFrameDirections.getConstSet(j, i);

                    //Помечаем прилежащие пиксели, на основании направления их движения, как область возможного движения
                    for (int k = 0; k < setSize; k++)
                    {
                        markNeighbors(i, j, height, width, *(directionsSet + k));
                    }
                }
            }
        }

        if (global::noiseCount < global::minNoiseCount)
        {
            global::minNoiseCount = global::noiseCount;
            global::bestImage = global::currentImage;
        }

        //Переносим информацию из маски на текущее изображение
        if(saveImage) applyMask_to_pixelData(data, height, width, channels);

        //Меняем местами предыдущую и текущую матрицы направлений 
        previousFrameDirections.swap(curentFrameDirections);
        //Заполняем текущую матрицу направлений нулями
        curentFrameDirections.clear(0);
    }
}
