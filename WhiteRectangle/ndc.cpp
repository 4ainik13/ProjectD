#include "ndc.h"

using namespace glm;

namespace NDC
{
    //Normalized Device Coordinate to viewport coordinate
    float to_viewport(float ndc, float viewportSize)
    {
        return (ndc + 1.0f) / 2.0f * viewportSize;
    }

    float to_ndc(float vpc, float viewportSize)
    {
        return 2.0f * vpc / viewportSize - 1.0f;
    }

    float to_add(float vpc, float viewportSize)
    {
        return 2.0f * vpc / viewportSize;
    }

    //Normalized Device Coordinates of two dimensions (x and y) to viewport coordinates
    vec2 twoDimsension_to_viewport(vec2 ndcs, float viewportWidth, float viewportHeight)
    {
        return vec2(to_viewport(ndcs.x, viewportWidth), to_viewport(ndcs.y, viewportHeight));
    }

    //Normalized Device Coordinates of one dimension (x or y) to viewport coordinates
    vec2 oneDimsension_to_viewport(vec2 ndcs, float viewportSize)
    {
        return twoDimsension_to_viewport(ndcs, viewportSize, viewportSize);
    }

    //Returns dimension value between two Normalized Device Coordinates of one dimension
    GLuint to_dimension(float firstCoord, float secondCoord, float viewportSize)
    {
        return to_viewport(secondCoord - firstCoord, viewportSize);
    }
}
