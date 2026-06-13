#define _CRT_SECURE_NO_WARNINGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <sys/stat.h>
#include <direct.h>
#include <iostream>

//Мои заголовки
#include "shaderHandler.h"
#include "stopwatch.h"
#include "ndc.h"
#include "matrix.h"
#include "my_debug.h"
#include "image_handler.h"
#include "denoise_alg.h"
#include "pbo_tex.h"
#include "global_vars.h"
#include "latex.h"
#include "statistics.h"

using namespace glm;
using namespace global;

//Функции
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void processDisplay(GLFWwindow* window, const GLuint shaderProgram, const GLuint vao);
void shaderUpdate(GLFWwindow* window);
void statisticsUpdate(Statistics& stat, const bool& readInitialPixels, const int& row, const int& col, const double& val);
void startSession(GLFWwindow* window);

mat4 createTransformMatrix(GLuint scrW, GLuint scrH, 
    vec2 normalXBounds, vec2 normalYBounds);

void printPixels(const unsigned char* pixels, int channels, GLsizei width, GLsizei height);

void printAddress(const char* varName, const char* funcName, void* addresToPrint);

GLuint global_shaderProgram = 0;
bool global_sessionStarted = false;
int global_imageCount = 0;

//Общие переменные
ImageHandler imageHandler;
bool readInitialPixels;
bool saveImage;
int experiment;
int maxExperiment;
time_t randSeed;
time_t randSeedMod = 1779000000;
Stopwatch fpsWatch;

//uniform переменные
GLfloat noiseProbability;
GLint noiseSeed;
vec2 a_vec;
vec2 b_vec;

//Полный размер кадра (не окна)
GLsizei pixelsW = FRAME_WIDTH;
GLsizei pixelsH = FRAME_HEIGHT;
GLuint pixelsX = FRAME_X;
GLuint pixelsY = FRAME_Y;

const int global_pixels_channels = 3;
const int global_pixels_size = pixelsW * pixelsH;

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

int seedArray[] = { 880928, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool useSeedArray = false;

//Программа
int main()
{
    //0. Инициализация GLFW, окна, GLAD и прочих функций
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Говорим OpenGL, что хотим использовать версию 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Говорим OpenGL, что хотим использовать версию x.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Говорим, что используем профиль core

    //Создаём окно
    GLFWwindow* window = glfwCreateWindow(800, 600, "ProjectD", NULL, NULL);
    
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

    //glEnableVertexAttribArray(0); можно выполнять и после того, как мы разметили
    //данные атрибута (потому что он включается на vao?). 
    //Но для здравого смысла будем включать соответсвующий
    //атрибут перед тем как разметить соответсвующие ему данные, а не после.
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

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

    //6.5. Создаём второй набор буферов и шейдеров для отрисовки pbo
    tex::createShaderProgram();
    tex::genVertBuffers();
    tex::genTexBuffer();

    //7. Подготовка шейдерных переменных
    vec2 res = vec2(SCR_WIDTH, SCR_HEIGHT);
    mat4 trans = createTransformMatrix(SCR_WIDTH, SCR_HEIGHT, vec2(-0.5f, 0.5f), vec2(-0.5f, 0.5f));
    a_vec = vec2(5.f, 5.f); //5.0 5.0
    b_vec = vec2(0.f, 0.f);   //0.0 0.0

    noiseProbability = 0.2f; //0.05f

    if (!useSeedArray) randSeed = time(0) % randSeedMod;
    else randSeed = seedArray[0];
    srand(randSeed);
    noiseSeed = rand();

    GLint u_resLoc = glGetUniformLocation(shaderProgram, "u_res");
    GLint u_timeLoc = glGetUniformLocation(shaderProgram, "u_time");
    GLint u_transformLoc = glGetUniformLocation(shaderProgram, "u_transform");
    GLint u_noiseProbLoc = glGetUniformLocation(shaderProgram, "u_noiseProb");
    GLint u_seedLoc = glGetUniformLocation(shaderProgram, "u_seed");
    GLint u_aLoc = glGetUniformLocation(shaderProgram, "u_a");
    GLint u_bLoc = glGetUniformLocation(shaderProgram, "u_b");

    //2fv => один вектор из двух float
    //count = 1 => один вектор из двух float
    glUniform2fv(u_resLoc, 1, glm::value_ptr(res));
    glUniformMatrix4fv(u_transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
    glUniform1f(u_timeLoc, glfwGetTime());
    glUniform1f(u_noiseProbLoc, noiseProbability);
    glUniform1i(u_seedLoc, noiseSeed);
    glUniform2fv(u_aLoc, 1, glm::value_ptr(a_vec));
    glUniform2fv(u_bLoc, 1, glm::value_ptr(b_vec));


    //8. Подготовка к рендеру
    imageHandler = ImageHandler(SCR_WIDTH * SCR_HEIGHT * CLR_CHANNELS, 2);
    readInitialPixels = false;

    Latex<int> latex;
    Latex<double> frameTable = Latex<double>(1, 10);

    experiment = 0;
    maxExperiment = 0; // 9 для десяти экспериментов

    double fps = 0;
    Statistics statist;
    statist.initRows(maxExperiment+1);

    int ticks = 10000;
    double delay = 0; //0.025 = 25 миллисекунд
    window_watch.set(delay, ticks); //0.025
    printAddress("window_watch", "main", &window_watch);

    //Задаём цвет очистки (заливки) окна
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //9. Рендер
    latex.parseSeed(randSeed, experiment);
    saveImage = false;
    while (!glfwWindowShouldClose(window))
    {
        fps = 1 / fpsWatch.lap();
        statisticsUpdate(statist, readInitialPixels, experiment, imageHandler.imageCounter - 1, fps);

        processInput(window);
        processDisplay(window, shaderProgram, vao);

        //tex::drawElements();

        if (readInitialPixels)
        {
            readInitialPixels = false;
            imageHandler.initialPixelsRead(pixelsX, pixelsY, BMP_HEIGHT, BMP_WIDTH);
            processDisplay(window, shaderProgram, vao);
        }

        if (window_watch.ticked() && minNoiseCount > 1)
        {
            //Артефакаты при [201-275] + pbo
            int d_width = BMP_WIDTH, d_height = BMP_HEIGHT; //изначально 160
            //if (imageHandler.imageCounter % 100 == 0) saveImage = true;
            //else saveImage = false;
            saveImage = false;
            imageHandler.saveImage_differentWays(pixelsX, pixelsY, d_height, d_width, CLR_CHANNELS, "pboTest"+to_string(experiment)+"_", saveImage);
            printf("saved image %d\t", imageHandler.imageCounter);
            printf("noise count: %d\t", noiseCount);
            printf("min noise: %d\t", minNoiseCount);
            printf("best image: %d\t", bestImage);
            printf("experiment: %d\t", experiment+1);
            printf("fps: %.2f\n", fps);
            latex.parse(noiseCount, experiment, imageHandler.imageCounter-1);
        }

        glfwSwapBuffers(window); //Меняем местами передний и задний буферы рендера окна (два больших массива цветов)
        glfwPollEvents(); //Обрабатываем все произошедшие события. Вызываем связанные callback-функции

        if (window_watch.noTicks() || minNoiseCount <= 1)
        {
            //перезапускаем всё это дело
            //!!! Отменить инициализацию в denoise_alg !!!
            experiment++;

            imageHandler.imageCounter = 0;
            readInitialPixels = true;

            global::noiseCount = 0;
            global::minNoiseCount = INT_MAX;
            global::bestImage = 0;
            global::currentImage = 0;

            alg::beta::initMasks();

            double fpsCorrectionTime = fpsWatch.lap();
            glfwSetTime(0);

            if (!useSeedArray) randSeed = (time(0) + experiment) % randSeedMod;
            else randSeed = seedArray[experiment];
            srand(randSeed);
            glUniform1i(u_seedLoc, rand());

            latex.parseSeed(randSeed, experiment);

            window_watch.setStart(delay, ticks);
            fpsWatch.setStartTime(-fpsCorrectionTime);
        }
        //if (window_watch.noTicks() || minNoiseCount <= 1) experiment++;
        if (experiment > maxExperiment) glfwSetWindowShouldClose(window, true);
    }
    
    latex.writeTable(a_vec, b_vec, noiseProbability);
    frameTable.parseVector(statist.average(), 0);
    frameTable.writeTabular("frameTable.txt");

    //10. Освобождаем ресурсы
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shaderProgram);
    tex::freeResources();

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

void processDisplay(GLFWwindow* window, const GLuint shaderProgram, const GLuint vao)
{
    shaderUpdate(window);

    glClear(GL_COLOR_BUFFER_BIT); //Очищаем буфер окна (задаём одноцветный фон)

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void shaderUpdate(GLFWwindow* window)
{
    if (global_sessionStarted)
    {
        setShaderTime(global_shaderProgram);
    }
}

void statisticsUpdate(Statistics& stat, const bool& readInitialPixels, const int& row, const int& col, const double& val)
{
    if (global_sessionStarted && !readInitialPixels)
    {
        stat.parse(row, col, val);
    }
}

void printBasisInf()
{
    printf("a vector = (%.3f, %.3f)\t", a_vec.x, a_vec.y);
    printf("b vector = (%.3f, %.3f)\t", b_vec.x, b_vec.y);
    printf("P = %.3f\n", noiseProbability);
}

void startSession(GLFWwindow* window)
{
    if (global_sessionStarted == false)
    {
        printBasisInf();

        global_sessionStarted = true;

        glfwSetTime(0);

        //imageHandler.initialPixelsRead(pixelsX, pixelsY, BMP_HEIGHT, BMP_WIDTH);
        readInitialPixels = true;
        Stopwatch* window_watch = (Stopwatch*)glfwGetWindowUserPointer(window);
        printAddress("window_watch", "startSession", window_watch);
        window_watch->start();
        fpsWatch.start();
    }
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
                printf("%c: %d ", colorChars[k], pixels[mat3D::getRawIndex(i, j, k, width, channels)]);
            }
            printf("\n");
        }
        printf("\n");
    }

    //printf("done\n");
}

mat4 createTransformMatrix(GLuint scrW, GLuint scrH,
    vec2 normalXBounds, vec2 normalYBounds)
{
    vec2 xb = NDC::oneDimsension_to_viewport(normalXBounds, scrW);
    vec2 yb = NDC::oneDimsension_to_viewport(normalYBounds, scrH);

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