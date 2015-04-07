/**
wrapper_glfw.h
Modified from the OpenGL GLFW example to provide a wrapper GLFW class
Majed Monem 2014/15 Graphical Enigma Simulator Honours Project

*/
#pragma once

#ifdef _MSC_VER
#pragma warning (disable: 4996)         // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#include "imgui.h"
#include "enigma.h"

#define GLEW_STATIC
#include <glew.h>
#include <GLFW/glfw3.h>
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))


/* GLM core */
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class GLWrapper {

	
private:

	char *title;
	double fps;
	void(*error_callback)(int error, const char* description);
	void(*renderer)();
	void(*reshape)(GLFWwindow* window, int w, int h);
	void(*keyCallBack)(GLFWwindow* window, int key, int scancode, int action, int mods);
	void(*mousebtn)(GLFWwindow* window, int button, int action, int mods);
	void(*scroll)(GLFWwindow* window, double xoffset, double yoffset);
	void(*chars)(GLFWwindow* window, unsigned int c);
	bool running;
	
public:
	GLFWwindow* window;
	GLWrapper(int width, int height, char *title);
	~GLWrapper();

	enigma machine;
	
	int cursorpos;
	void setPos(int i);

	bool calculated;
	bool show_rotor, show_help;
	bool transition_done;
	float transition_current, transition_limit;
	float trans_inc = 2.f;

	void setFPS(double fps) {
		this->fps = fps;
	}

	/* Callback registering functions */
	void setRenderer(void(*f)());
	void setReshapeCallback(void(*f)(GLFWwindow* window, int w, int h));
	void setKeyCallback(void(*f)(GLFWwindow* window, int key, int scancode, int action, int mods));
	void setMouseButtonCallback(void(*f)(GLFWwindow* window, int button, int action, int mods));
	void setScrollCallback(void(*f)(GLFWwindow* window, double xoffset, double yoffset));
	void setCharCallback(void(*f)(GLFWwindow* window, unsigned int c));
	void setErrorCallback(void(*f)(int error, const char* description));
	void UpdateImGui(bool mousePressed[]);

	/* Shader load and build support functions */
	GLuint LoadShader(const char *vertex_path, const char *fragment_path);
	GLuint BuildShader(GLenum eShaderType, const std::string &shaderText);
	GLuint BuildShaderProgram(std::string vertShaderStr, std::string fragShaderStr);
	std::string readFile(const char *filePath);

	std::string getRotorOne();
	void setRotorOne(std::string str);

	std::string getStaticrOne();
	void setStaticrOne(std::string str);

	std::string getReflector();
	void setRelfector(std::string str);

	std::string getAlphabet();


	int eventLoop(bool mousePressed[]);
	GLFWwindow* getWindow();
	std::string mode;
	int width;
	int height;
	bool resized;

	char strPlain[96];
	char strCipher[96];

	int count;
	std::string rotorOne;
	std::string encrypted;
	std::string decrypted;
	bool* changed;
	bool* platechange;
	bool complete;
	int changenum;

	GLfloat rotation, introtation = 0.0f;
	GLchar getCiphered(int index);
	GLchar getPlain(int index, char k);
	
	
	std::string rotors[7];
	std::string reflectors[5];

	int combopos1, combopos2;
	int prev1, prev2;
	void Encrypt(char k);
	void Decrypt(char k);
	void reset();

};



