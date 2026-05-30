#pragma once

namespace mat3D
{
    //Возвращает абсолютный индекс для трёхмерного массива по трём координатам first_index, second_index и third_index.
    //Массив имеет размер first_dimension X second_dimension X third_dimension.
    //Размеры first_dimension, second_dimension и third_dimension должны соответственно относиться к first_index, second_index и third_index.
    int getRawIndex(
        int first_index, int second_index, int third_index,
        int second_dimension, int third_dimension);
}

namespace mat2D
{
    //Возвращает абсолютный индекс для двумерного массива по двум координатам first_index и second_index.
    //Массив имеет размер first_dimension X second_dimension.
    //Размеры first_dimension и second_dimension должны соответственно относиться к first_index и second_index.
    int getRawIndex(
        int first_index, int second_index,
        int second_dimension);
}