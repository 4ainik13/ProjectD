#define _CRT_SECURE_NO_WARNINGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include "shaderHandler.h"
#include "stopwatch.h"

using namespace glm;


//Функции
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void shaderUpdate(GLFWwindow* window);
void startSession(GLFWwindow* window);

vec2 NDC_oneDimsension_to_viewport(vec2 ndc, float viewportDimension);
float NDC_to_viewport(float ndc, float viewportDimension);
GLuint NDC_to_dimension(float firstPoint, float secondPoint, float viewportDimension);

mat4 createTransformMatrix(GLuint scrW, GLuint scrH, 
    vec2 normalXBounds, vec2 normalYBounds);

void printPixels(const unsigned char* pixels, int channels, GLsizei width, GLsizei height);
void printSymbolPixels(const unsigned char* pixels, int channels, GLsizei width, GLsizei height);

void saveCurrentImage(GLsizei width, GLsizei height, std::string name);
void saveImageCounting(int& imageCounter, GLsizei width, GLsizei height, std::string name);
void saveImage_fromData(const unsigned char* data, int width, int height, std::string name, int channels);
void denoiseImage(int height, int width, int channels);

void printAddress(const char* varName, const char* funcName, void* addresToPrint);
void init_byte_array(unsigned char* arr, int size, unsigned char initVal);

//Переменные
const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

GLuint global_shaderProgram = 0;
bool global_sessionStarted = false;
int global_imageCount = 0;

GLfloat movingPoint_epsilon = 0.01f;
GLfloat noiseProbability = 0.01f;

GLsizei pixelsW = NDC_to_dimension(-0.5f, 0.5f, SCR_WIDTH) / 2;
GLsizei pixelsH = NDC_to_dimension(-0.5f, 0.5f, SCR_HEIGHT) / 2;
GLuint pixelsX = NDC_to_viewport(-0.5f, SCR_WIDTH);
GLuint pixelsY = NDC_to_viewport(-0.5f, SCR_HEIGHT);

const int global_pixels_channels = 3;
const int global_pixels_size = pixelsW * pixelsH;

unsigned char* global_pixelsData = new unsigned char[global_pixels_size * global_pixels_channels]{};
unsigned char* global_pixelsMask_previous = new unsigned char[global_pixels_size]{};
unsigned char* global_pixelsMask_curent = new unsigned char[global_pixels_size] {};

float vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};
unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};


//Программа
int main()
{
    //0. Инициализация GLFW, окна, GLAD и прочих функций
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Говорим OpenGL, что хотим использовать версию 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Говорим OpenGL, что хотим использовать версию x.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Говорим, что используем профиль core

    //Создаём окно
    GLFWwindow* window = glfwCreateWindow(800, 600, "Learn OpenGL", NULL, NULL);
    
    //Инициализируем переменные, передаваемые вместе с окном
    Stopwatch window_watch = Stopwatch();
    
    //Запоминаем адреса переменных
    glfwSetWindowUserPointer(window, &window_watch);

    if (window == NULL) //Проверяем, что окно создалось
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate(); //Освобождаем ресурсы
        return -1;
    }
    
    glfwMakeContextCurrent(window); //Указываем текущее главное окно

    //Инициализация GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //Определяем размер видимого окна
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    
    //Привязываем функцию framebuffer_size_callback к событию изменения размера окна. 
    //Теперь эта функция будет вызываться при каждом изменении размера окна
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    //1. Создаём vao
    GLuint vao;
    //!!!!
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    //2. Создаём vbo
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //3. Создаём ebo
    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //4-6. Создание шейдеров и программы шейдеров
    const char* vertexPath = "./default.vert";
    const char* fragmentPath = "./default.frag";
    ShaderHandler shaderHnd(vertexPath, fragmentPath);
    GLuint shaderProgram = shaderHnd.getShaderProgram();
    global_shaderProgram = shaderProgram;
    glUseProgram(shaderProgram);

    //7. Подготовка шейдерных переменных
    vec2 res = vec2(SCR_WIDTH, SCR_HEIGHT);
    mat4 trans = createTransformMatrix(SCR_WIDTH, SCR_HEIGHT, vec2(-0.5f, 0.5f), vec2(-0.5f, 0.5f));
    
    movingPoint_epsilon = 0.005f;
    noiseProbability = 0.05f;

    GLint u_resLoc = glGetUniformLocation(shaderProgram, "u_res");
    GLint u_timeLoc = glGetUniformLocation(shaderProgram, "u_time");
    GLint u_transformLoc = glGetUniformLocation(shaderProgram, "u_transform");
    GLint u_epsLoc = glGetUniformLocation(shaderProgram, "u_eps");
    GLint u_noiseProbLoc = glGetUniformLocation(shaderProgram, "u_noiseProb");

    //2fv => один вектор из двух float
    //count = 1 => один вектор из двух float
    glUniform2fv(u_resLoc, 1, glm::value_ptr(res));
    glUniformMatrix4fv(u_transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
    glUniform1f(u_timeLoc, glfwGetTime());
    glUniform1f(u_epsLoc, movingPoint_epsilon);
    glUniform1f(u_noiseProbLoc, noiseProbability);


    //8. Подготовка к рендеру
    //25 миллисекунд по 16 раз
    window_watch.set(0.025, 16);
    printAddress("window_watch", "main", &window_watch);

    init_byte_array(global_pixelsMask_previous, global_pixels_size, 1);
    init_byte_array(global_pixelsMask_curent, global_pixels_size, 0);

    //Задаём цвет очистки (заливки) окна
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //9. Рендер
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        shaderUpdate(window);

        glClear(GL_COLOR_BUFFER_BIT); //Очищаем буфер окна (задаём одноцветный фон)

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        if (window_watch.ticked())
        {
            int d_width = 160, d_height = 160;
            //saveImageCounting(global_imageCount, pixelsW, pixelsH, "test");
            saveImageCounting(global_imageCount, d_width, d_height, "test");
            printf("saved image %d\n", global_imageCount);
            //printSymbolPixels(global_pixelsData, 3, d_width, d_height);
            denoiseImage(d_height, d_width, 3);
            //denoiseImage(pixelsH, pixelsW, 3);
        }

        //clockUpdate(window_watch);

        glfwSwapBuffers(window); //Меняем местами передний и задний буферы рендера окна (два больших массива цветов)
        glfwPollEvents(); //Обрабатываем все произошедшие события. Вызываем связанные callback-функции

    }

    
    //10. Освобождаем ресурсы
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void setShaderTime(GLuint shaderProgram)
{
    GLint u_timeLoc = glGetUniformLocation(shaderProgram, "u_time");
    glUniform1f(u_timeLoc, glfwGetTime());
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        setShaderTime(global_shaderProgram);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        startSession(window);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        startSession(window);
}

void shaderUpdate(GLFWwindow* window)
{
    if (global_sessionStarted)
    {
        setShaderTime(global_shaderProgram);
    }
}

void init_byte_array(unsigned char* arr, int size, unsigned char initVal)
{
    for (int i = 0; i < size; i++)
    {
        arr[i] = initVal;
    }
}

//Копирует значения из одного массива в другой. Массивы должны иметь одинковый размер
void copy_byte_array(const unsigned char* arrGiver, unsigned char* arrTaker, int size)
{
    for (int i = 0; i < size; i++)
    {
        arrTaker[i] = arrGiver[i];
    }
}

void startSession(GLFWwindow* window)
{
    if (global_sessionStarted == false)
    {
        global_sessionStarted = true;

        glfwSetTime(0);

        Stopwatch* window_watch = (Stopwatch*)glfwGetWindowUserPointer(window);
        printAddress("window_watch", "startSession", window_watch);
        window_watch->start();
    }
}

//Сохраняем содержимое data на компьютере
void saveImage_fromData(const unsigned char* data, int height, int width, std::string name = "testFromData", int channels = 3)
{
    std::string filename = "images\\" + name + ".bmp";
    stbi_flip_vertically_on_write(1);
    stbi_write_bmp(filename.c_str(), width, height, channels, data);
}

void saveImage_fromData_counting(const unsigned char* data, int& imageCount, int height, int width, std::string name = "testFromData", int channels = 3, bool increment = true)
{
    if(increment) imageCount++;
    name = name + std::to_string(imageCount);
    saveImage_fromData(data, height, width, name);
}

//Сохраняем текущий кадр в формате bmp на компьютере. 
//global_pixelsData указывает на последний записанный кадр в чистом формате.
void saveCurrentImage(GLsizei width, GLsizei height, std::string name = "test")
{
    unsigned char channels = 3;

    glReadPixels(pixelsX, pixelsY, width, height, GL_RGB, GL_UNSIGNED_BYTE, global_pixelsData);
        

    std::string filename = "images\\" + name + ".bmp";
    stbi_flip_vertically_on_write(1);
    stbi_write_bmp(filename.c_str(), width, height, channels, global_pixelsData);
}

//Сохраняем текущий кадр в формате bmp с подсчётом сохранённых кадров.
//Полученный кадр будт иметь имя в формате nameX.bmp, где X - номер кадра
void saveImageCounting(int& imageCounter, GLsizei width, GLsizei height, std::string name = "test")
{
    imageCounter++;
    saveCurrentImage(width, height, name + std::to_string(imageCounter));
}

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

//Возвращает пиксель из массива global_pixelData по указанному индексу
pixel getPixel(int index)
{
    return pixel{
        global_pixelsData[index],
        global_pixelsData[index+1], 
        global_pixelsData[index+2]
    };
}

//Задаёт указанный пиксель pix в трёхмерном массиве global_pixelData по индексу index.
//Сам index должен быть представлен как абсолютный индекс массива global_pixelData, указываюший на 0-ой элемент, относящийся к цвету
void setPixel(int index, pixel& pix)
{
    global_pixelsData[index] = pix.r;
    global_pixelsData[index + 1] = pix.g;
    global_pixelsData[index + 2] = pix.b;
}

//Меняем пиксель по адресу index со смешиванием пикслея pix
void blendPixel(int index, pixel& pix)
{
    pixel pixAtIndex = getPixel(index);
    if (pixAtIndex != white)
    {
        setPixel(index, pixAtIndex);
    }
    else
    {
        setPixel(index, pix);
    }
}


//Возвращает символьное представление пикселя (если оно задано)
char appraisePixel(const pixel& pix)
{
    if (pix == red) return 'R';
    if (pix == black) return '#';
    if (pix == white) return '.';
}

//Возвращает абсолютный индекс для трёхмерного массива по трём координатам first_index, second_index и third_index.
//Массив имеет размер first_dimension X second_dimension X third_dimension
int mat3D_getRawIndex(
    int first_index, int second_index, int third_index,
    int second_dimension, int third_dimension)
{
    return first_index * second_dimension * third_dimension + second_index * third_dimension + third_index;
}

//Возвращает абсолютный индекс для двухмерного массива по двум координатам first_index и second_index.
//Массив имеет размер first_dimension X second_dimension
int mat2D_getRawIndex(
    int first_index, int second_index,
    int second_dimension)
{
    return first_index * second_dimension + second_index;
}

//Отладочная функция.
//Выводит на консоль данные из массива пикселей в символьном виде
void printSymbolPixels(const unsigned char* pixels, int channels, GLsizei width, GLsizei height)
{
    unsigned char colorChars[4] = { 'R', 'G', 'B', 'A' };
    pixel pix{};

    for (GLsizei i = height-1; i >= 0; i--)
    {
        for (GLsizei j = 0; j < width; j++)
        {
            //pix = getPixel(i * width * channels + j * channels);
            pix = getPixel(mat3D_getRawIndex(i, j, 0, width, channels));
            printf("%c ", appraisePixel(pix));
        }
        printf("\n");
    }

    //printf("done\n");
}

//Помечаем пиксель по координатам width_index и height_index
void markPixel(int height_index, int width_index, int width)
{
    int rawIndex = mat2D_getRawIndex(height_index, width_index, width);
    global_pixelsMask_curent[rawIndex] = 1;
}

//Помечаем всех соседей пикселя с координатами width_index и height_index
void markNeighbors(int height_index, int width_index, int width)
{
    if (global_pixelsMask_previous[mat2D_getRawIndex(height_index, width_index, width)] == 0) return;
    for (int i = height_index-1; i <= height_index+1; i++)
    {
        if (i < 0) continue;
        for (int j = width_index-1; j <= width_index+1; j++)
        {
            if (j < 0) continue;
            markPixel(i, j, width);
        }
    }
}

void applyMask_to_pixelData(const unsigned char* mask, int height, int width, int channels = 3)
{
    pixel pix{ 0, 255, 0 };
    int maskIndex;
    int pixelIndex;
    unsigned char maskVal;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            maskIndex = mat2D_getRawIndex(i, j, width);
            maskVal = global_pixelsMask_curent[maskIndex];
            if (maskVal == 1)
            {
                pixelIndex = mat3D_getRawIndex(i, j, 0, width, channels);
                blendPixel(pixelIndex, pix);
            }
        }
    }
}

void denoiseImage(int height, int width, int channels = 3)
{
    int pixelIndex;

    //printSymbolPixels(global_pixelsData, 3, width, height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            pixelIndex = mat3D_getRawIndex(i, j, 0, width, channels);
            if (getPixel(pixelIndex) != white)
            {
                markNeighbors(i, j, width);
            }
        }
    }

    applyMask_to_pixelData(global_pixelsMask_curent, height, width, channels);

    //Копируем текущую маску в буфер, а затем очищаем её.
    copy_byte_array(global_pixelsMask_curent, global_pixelsMask_previous, global_pixels_size);
    init_byte_array(global_pixelsMask_curent, global_pixels_size, 0);

    saveImage_fromData_counting(
        global_pixelsData, global_imageCount, height, width, "denoise", 3, false
    );
}

//Отладочная функция.
//Выводит на консоль данные из массива пикселей в необработанном виде
void printPixels(const unsigned char* pixels, int channels, GLsizei width, GLsizei height)
{
    unsigned char colorChars[4] = { 'R', 'G', 'B', 'A' };

    for (GLsizei i = 0; i < height; i++)
    {
        for (GLsizei j = 0; j < width; j++)
        {
            for (int k = 0; k < channels; k++)
            {
                printf("%c: %d ", colorChars[k], pixels[mat3D_getRawIndex(i, j, k, width, channels)]);
            }
            printf("\n");
        }
        printf("\n");
    }

    //printf("done\n");
}

//Normalized Device Coordinate to viewport coordinate
float NDC_to_viewport(float ndc, float viewportDimension)
{
    return (ndc + 1.0f) / 2.0f * float(viewportDimension);
}

//Normalized Device Coordinates of two dimensions (x and y) to viewport coordinates
vec2 NDC_twoDimsension_to_viewport(vec2 ndc, float viewportWidth, float viewportHeight)
{
    return vec2(NDC_to_viewport(ndc.x, viewportWidth), NDC_to_viewport(ndc.y, viewportHeight));
}

//Normalized Device Coordinates of one dimension (x or y) to viewport coordinates
vec2 NDC_oneDimsension_to_viewport(vec2 ndc, float viewportDimension)
{
    return NDC_twoDimsension_to_viewport(ndc, viewportDimension, viewportDimension);
}

//Returns dimension value between two Normalized Device Coordinates of one dimension
GLuint NDC_to_dimension(float firstPoint, float secondPoint, float viewportDimension)
{
    return NDC_to_viewport(secondPoint - firstPoint, viewportDimension);
}

mat4 createTransformMatrix(GLuint scrW, GLuint scrH,
    vec2 normalXBounds, vec2 normalYBounds)
{
    vec2 xb = NDC_oneDimsension_to_viewport(normalXBounds, scrW);
    vec2 yb = NDC_oneDimsension_to_viewport(normalYBounds, scrH);

    float lamb1 = scrW / (xb[1] - xb[0]);
    float lamb2 = scrH / (yb[1] - yb[0]);

    mat4 p_s = mat4(
        vec4(lamb1, 0, 0, 0),
        vec4(0, lamb2, 0, 0),
        vec4(0, 0, 1, 0),
        vec4(0, 0, 0, 1)
    );

    mat4 p_t = mat4(
        vec4(1, 0, 0, 0),
        vec4(0, 1, 0, 0),
        vec4(0, 0, 1, 0),
        vec4(-xb[0], -yb[0], 0, 1)
    );

    return p_s * p_t;
}

//Отладочная функция.
//Вывод на консоль адреса переменной.
void printAddress(const char* varName, const char* funcName, void* addresToPrint)
{
    //printf(varName + " in " funcName + " : %p", addresToPrint);
    printf("%s in %s: %p\n", varName, funcName, addresToPrint);
}