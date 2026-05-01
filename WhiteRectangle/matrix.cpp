#include "matrix.h"

namespace mat3D
{
    int getRawIndex(
        int first_index, int second_index, int third_index,
        int second_dimension, int third_dimension)
    {
        return first_index * second_dimension * third_dimension + second_index * third_dimension + third_index;
    }
}

namespace mat2D
{
    int getRawIndex(
        int first_index, int second_index,
        int second_dimension)
    {
        return first_index * second_dimension + second_index;
    }
}