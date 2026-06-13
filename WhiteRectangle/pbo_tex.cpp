#include "pbo_tex.h"
#include "shaderHandler.h"
#include "ndc.h"
#include "global_vars.h"

using namespace global;

namespace
{
	const char* vertexPath = "./pboTex.vert";
	const char* fragmentPath = "./pboTex.frag";

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint shaderProgram;

    namespace attrib
    {
        float x_left   = 0.5f;
        float x_right  = x_left + NDC::to_add(BMP_WIDTH, SCR_WIDTH);
        float y_bottom = -0.5f;
        float y_top = y_bottom + NDC::to_add(BMP_HEIGHT, SCR_HEIGHT);

        float vertices[] = {
         x_right,  y_top, 0.0f,  //right top 
         x_right, y_bottom, 0.0f,  //right bottom 
         x_left, y_bottom, 0.0f,  //left bottom
         x_left,  y_top, 0.0f   //left top  
        };

        float texCoords[] = {
            1.0f, 1.0f,     //right top
            1.0f, 0.0f,     //right bottom
            0.0f, 0.0f,     //left bottom
            0.0f, 1.0f,     //left top
        };

        unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,   // first triangle
            1, 2, 3    // second triangle
        };
    }

    
}

using namespace attrib;

namespace tex
{
	GLuint createShaderProgram()
	{
		ShaderHandler shaderHnd(vertexPath, fragmentPath);
		shaderProgram = shaderHnd.getShaderProgram();
		return shaderProgram;
	}

    void genTexBuffer()
    {

    }

    void genVertBuffers()
    {
        //Создаём vao
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        //Создаём vbo
        glGenBuffers(1, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        //Создаём ebo
        glGenBuffers(1, &ebo);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }

    void drawElements()
    {
        //glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void freeResources()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteProgram(shaderProgram);
    }
}
