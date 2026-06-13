#pragma once

struct pixel
{
    unsigned char r, g, b;
};

const pixel red{ 255, 0, 0 }, green{ 0, 255, 0 }, blue{ 0, 0, 255 },
            black{ 0, 0, 0 }, white{ 255, 255, 255 };

bool operator == (const pixel& p1, const pixel& p2)
{
    return p1.r == p2.r && p1.g == p2.g && p1.b == p2.b;
}

bool operator != (const pixel& p1, const pixel& p2)
{
    return p1.r != p2.r || p1.g != p2.g || p1.b != p2.b;
}


//Возвращает пиксель из матрицы изображения data по указанному индексу.
//Предполагается, что data - это трёхмерный массив с высотой, шириной и количеством цветов.
//index - это необработанный (raw) индекс, который должен указывать на 0-ой элемент цвета: data(x, y, 0).
pixel getPixel(const unsigned char* data, const int& index)
{
    return pixel{
        data[index],
        data[index + 1],
        data[index + 2]
    };
}

//Задаёт пиксель pix в матрице изображения data по индексу index.
//Предполагается, что data - это трёхмерный массив с высотой, шириной и количеством цветов.
//index - это необработанный (raw) индекс, который должен указывать на 0-ой элемент цвета: data(x, y, 0).
void setPixel(unsigned char* data, const int& index, const pixel& pix)
{
    data[index] = pix.r;
    data[index + 1] = pix.g;
    data[index + 2] = pix.b;
}

//Меняем пиксель в матрице изображения data по индексу index на пиксель newPix.
//Пиксель меняется на newPix только в том случае, если он имеет цвет changablePix.
//Предполагается, что data - это трёхмерный массив с высотой, шириной и количеством цветов.
//index - это необработанный (raw) индекс, который должен указывать на 0-ой элемент цвета: data(x, y, 0).
void colorPixel(unsigned char* data, const int& index, const pixel& newPix, const pixel& changablePix = white)
{
    if (getPixel(data, index) == changablePix)
    {
        setPixel(data, index, newPix);
    }
}
