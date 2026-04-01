#include "shaderClass.h"

//Считываем файл и возвращаем его представление в виде строки
std::string get_file_contents(const char* fileName)
{
	//Создаём входной поток
	std::ifstream in(fileName, std::ios::binary);
	if (in)
	{
		std::string contents;
		//Перемещаем курсор в потоке на 0 позицию с конца, то есть в конец файла
		in.seekg(0, std::ios::end);
		//Меняем размер contents на счётный номер текущего символа в файле (в данном случае текущий символ будет eof)
		contents.resize(in.tellg());
		//Возвращаем курсор в потоке в самое начало
		in.seekg(0, std::ios::beg);
		//Считываем содержимое потока в contents
		in.read(&contents[0], contents.size());
		in.close();
		return contents;
	}
	throw(errno);
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	//Читаем шейдеры из файлов
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	//Подгоняем строки под стиль строк в си 
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	//Генерируем ссылку на vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//Указываем исходный код для vertex shader
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	//Компилирукм шейдер
	glCompileShader(vertexShader);
	//Проверяем ошибки после компиляции
	compileErrors(vertexShader, "VERTEX");

	//Генерируем ссылку на fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//Указываем исходный код fragment shader'a
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	//Компилируем шейдер
	glCompileShader(fragmentShader);
	//Проверяем на ошибки
	compileErrors(fragmentShader, "FRAGMENT");

	//Создаём ссылку на шейдер-программу
	ID = glCreateProgram();
	//Прикрепляем vertex shader к нашей программе
	glAttachShader(ID, vertexShader);
	//Прикрепляем fragment shader к нашей программе
	glAttachShader(ID, fragmentShader);
	//Соединяем все прикреплённые шейдеры вместе
	glLinkProgram(ID);
	//Проверяем программу на ошибки линковки
	compileErrors(ID, "PROGRAM");

	//Удаляем использованные шейдеры
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
}

Shader::Shader()
{
	ID = 0;
}

//Активация программы
void Shader::Activate()
{
	glUseProgram(ID);
}

//Удаляем программу
void Shader::Delete()
{
	glDeleteProgram(ID);
}

//Проверяем ошибки компиляции
void Shader::compileErrors(GLuint shader, const char* type)
{
	//Флаг успешной компиляции
	GLint hasCompiled;
	//Массив под сообщение об ошибке
	char infoLog[1024];

	//Проверка типа скомпилированного объекта
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for: " << type << '\n' << infoLog;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "PROGRAM_LINKING_ERROR for: " << type << '\n' << infoLog;
		}
	}
}