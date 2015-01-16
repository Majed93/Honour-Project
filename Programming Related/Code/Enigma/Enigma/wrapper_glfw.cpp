/**
  wrapper_glfw.cpp
  Modified from the OpenGL GLFW example to provide a wrapper GLFW class
  and to include shader loader functions to include shaders as text files
  Iain Martin August 2014
  */
#ifdef _MSC_VER
#pragma warning (disable: 4996)         // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#include "imgui.h"
#include "wrapper_glfw.h"

// Glfw/Glew
#define GLEW_STATIC
#include <GL/glew.h>
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


/* Constructor for wrapper object */
GLWrapper::GLWrapper(int width, int height, char *title) {

	this->width = width;
	this->height = height;
	this->title = title;
	this->fps = 60;
	this->running = true;

	/* Initialise GLFW and exit if it fails */
	if (!glfwInit()) 
	{
		std::cout << "Failed to initialize GLFW." << std::endl;
		exit(EXIT_FAILURE);
	}

	setErrorCallback(error_callback);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DEBUG
	glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	window = glfwCreateWindow(width, height, title, NULL, NULL);
	const GLFWvidmode* vmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window, vmode->width / 2 - width / 2, vmode->height / 2 - height / 2); //Centre screen
	if (!window){
		std::cout << "Could not open GLFW window." << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	/* Obtain an OpenGL context and assign to the just opened GLFW window */
	glfwMakeContextCurrent(window);
	
	glewExperimental = GL_TRUE;
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

	
	/* Initialise GLLoad library. You must have obtained a current OpenGL */
	/*if (!ogl_LoadFunctions())
	{
		std::cerr << "oglLoadFunctions() failed. Exiting" << std::endl;
		glfwTerminate();
		return;
	}
	*/
	glfwSetInputMode(window, GLFW_STICKY_KEYS, true);
}


/* Terminate GLFW on destruvtion of the wrapepr object */
GLWrapper::~GLWrapper() {
	glfwTerminate();
}

/* An error callback function to output GLFW errors*/
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

/* Returns the GLFW window handle, required to call GLFW functions outside this class */
GLFWwindow* GLWrapper::getWindow()
{
	return window;
}


/*
GLFW_Main function normally starts the windows system, calls any init routines
and then starts the event loop which runs until the program ends
*/
int GLWrapper::eventLoop(bool mousePressed[])
{
	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheel = 0;
		mousePressed[0] = mousePressed[1] = false;
		glfwPollEvents();
		UpdateImGui(mousePressed);
				
		static bool show_main = true;
		static bool show_encrypt = false;
		static bool show_decrypt = false;
		static bool show_exit = false;
		static bool no_titlebar = true;
		static bool no_border = true;
		static bool no_resize = true;
		static bool no_move = true;
		static bool no_scrollbar = true;
		static float fill_alpha = 0.65f;

		const ImGuiWindowFlags layout_flags = (no_titlebar ? ImGuiWindowFlags_NoTitleBar : 0) | (no_border ? 0 : ImGuiWindowFlags_ShowBorders) | (no_resize ? ImGuiWindowFlags_NoResize : 0) | (no_move ? ImGuiWindowFlags_NoMove : 0) | (no_scrollbar ? ImGuiWindowFlags_NoScrollbar : 0);

		if (show_main == true)
		{
			mode = "";
			show_encrypt = false;
			show_decrypt = false;
			ImGui::Begin("", &show_main, ImVec2(100, 100), fill_alpha, layout_flags);
			title = "Graphical Enigma Simutlator - Main Menu";

			ImGui::SetWindowPos(ImVec2(0, 0), 0);
			ImGui::SetWindowSize(ImVec2(width, height), 0);
			//FIX ROUNDED EDGES
			ImGuiStyle style;
			style.WindowRounding = 5.1f;
			
			show_encrypt ^= ImGui::Button("Encrypt");// , ImVec2(60, 20), true);
			show_decrypt ^= ImGui::Button("Decrypt");
			show_exit ^= ImGui::Button("Exit");
			ImGui::End();
		}

		//Show Encryption screen
		if (show_encrypt == true)
		{
			mode = "En";
			
			show_main = false;
			ImGui::Begin("Encrypt", &show_main, ImVec2(100, 100), fill_alpha, layout_flags);
			//no_titlebar = false;
			title = "Graphical Enigma Simutlator - Encrypt";
			ImGui::SetWindowPos(ImVec2(0, 350), 0);
			ImGui::SetWindowSize(ImVec2(width, height - 250), 0);

			show_main ^= ImGui::Button("Back To Main Menu");

			ImGui::Text("Rotor #");
			ImGui::SameLine();
			ImGui::Spacing();
			ImGui::SameLine();
			ImGui::PushItemWidth(200);
			const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK" };
			static int item2 = -1;
			ImGui::Combo("", &item2, items, IM_ARRAYSIZE(items));
			ImGui::PopItemWidth();

			static float wrap_width = 10.0f;

			ImGui::PushItemWidth(width - 25);
			ImGui::Text("Plain Text");
			static char strPlain[512] = "";
			ImGui::InputText("", strPlain, IM_ARRAYSIZE(strPlain));

			ImGui::Text("Cipher Text");
			static char strCipher[512] = "";
			ImGui::InputText(" ", strCipher, IM_ARRAYSIZE(strCipher));
			ImGui::PopItemWidth();

			ImGui::End();
		}
		//Show Decryption screen
		if (show_decrypt == true)
		{
			mode = "De";
			show_main = false;

			show_main = false;
			ImGui::Begin("", &show_main, ImVec2(100, 100), fill_alpha, layout_flags);
			title = "Graphical Enigma Simutlator - Decrypt";
			ImGui::SetWindowPos(ImVec2(0, 350), 0);
			ImGui::SetWindowSize(ImVec2(width, height - 250), 0);

			show_main ^= ImGui::Button("Back To Main Menu");

			ImGui::Text("Rotor #");
			ImGui::SameLine();
			ImGui::Spacing();
			ImGui::SameLine();
			ImGui::PushItemWidth(200);
			const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK" };
			static int item2 = -1;
			ImGui::Combo("", &item2, items, IM_ARRAYSIZE(items));
			ImGui::PopItemWidth();

			static float wrap_width = 10.0f;

			ImGui::PushItemWidth(width - 25);
			ImGui::Text("Plain Text");
			static char strPlain[512] = "";
			ImGui::InputText("", strPlain, IM_ARRAYSIZE(strPlain));

			ImGui::Text("Cipher Text");
			static char strCipher[512] = "";
			ImGui::InputText(" ", strCipher, IM_ARRAYSIZE(strCipher));
			ImGui::PopItemWidth();

			ImGui::End();
		}
		//Exit Application
		if(show_exit == true){
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSetWindowTitle(window, title);
		ImGui::Render();
		glfwSwapBuffers(window);
	}

	ImGui::Shutdown();
	glfwTerminate();
	return 0;
}

void GLWrapper::swap()
{
	//glfwSwapBuffers(window);
}
/* Register an error callback function */
void GLWrapper::setErrorCallback(void(*func)(int error, const char* description))
{
	this->error_callback = func;
	glfwSetErrorCallback(func);
}

/* Register a display function that renders in the window */
void GLWrapper::setRenderer(void(*func)()) {
	this->renderer = func;
}

/* Register a callback that runs after the window gets resized */
void GLWrapper::setReshapeCallback(void(*func)(GLFWwindow* window, int w, int h)) {
	reshape = func;
	glfwSetFramebufferSizeCallback(window, reshape);
}

/* Register a callback for mouse buttons*/
void GLWrapper::setMouseButtonCallback(void(*func)(GLFWwindow* window, int button, int action, int mods)) {
	mousebtn = func;
	glfwSetMouseButtonCallback(window, mousebtn);
}
/* Register a callback for scrolling*/
void GLWrapper::setScrollCallback(void(*func)(GLFWwindow* window, double xoffset, double yoffset)) {
	scroll = func;
	glfwSetScrollCallback(window, scroll);
}
/* Register a callback for chars */
void GLWrapper::setCharCallback(void(*func)(GLFWwindow* window, unsigned int c)) {
	chars = func;
	glfwSetCharCallback(window, chars);
}

/* Register a callback to respond to keyboard events */
void GLWrapper::setKeyCallback(void(*func)(GLFWwindow* window, int key, int scancode, int action, int mods))
{
	keyCallBack = func;
	glfwSetKeyCallback(window, keyCallBack);
}


/* Build shaders from strings containing shader source code */
GLuint GLWrapper::BuildShader(GLenum eShaderType, const std::string &shaderText)
{
	GLuint shader = glCreateShader(eShaderType);
	const char *strFileData = shaderText.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		// Output the compile errors
		
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		const char *strShaderType = NULL;
		switch (eShaderType)
		{
			case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
			case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
			case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}

		std::cerr << "Compile error in " << strShaderType << "\n\t" << strInfoLog << std::endl;
		delete[] strInfoLog;

		throw std::exception("Shader compile exception");
	}

	return shader;
}

/* Read a text file into a string*/
std::string GLWrapper::readFile(const char *filePath)
{
	std::string content;
	std::ifstream fileStream(filePath, std::ios::in);

	if (!fileStream.is_open()) {
		std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
		return "";
	}

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

/* Load vertex and fragment shader and return the compiled program */
GLuint GLWrapper::LoadShader(const char *vertex_path, const char *fragment_path)
{
	GLuint vertShader, fragShader;

	// Read shaders
	std::string vertShaderStr = readFile(vertex_path);
	std::string fragShaderStr = readFile(fragment_path);

	GLint result = GL_FALSE;
	int logLength;

	vertShader = BuildShader(GL_VERTEX_SHADER, vertShaderStr);
	fragShader = BuildShader(GL_FRAGMENT_SHADER, fragShaderStr);

	std::cout << "Linking program " << vertex_path << " & " << fragment_path << std::endl;
	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<char> programError((logLength > 1) ? logLength : 1);
	glGetProgramInfoLog(program, logLength, NULL, &programError[0]);
	std::cout << &programError[0] << std::endl;

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}

/* Load vertex and fragment shader and return the compiled program */
GLuint GLWrapper::BuildShaderProgram(std::string vertShaderStr, std::string fragShaderStr)
{
	GLuint vertShader, fragShader;
	GLint result = GL_FALSE;

	try
	{
		vertShader = BuildShader(GL_VERTEX_SHADER, vertShaderStr);
		fragShader = BuildShader(GL_FRAGMENT_SHADER, fragShaderStr);
	}
	catch (std::exception &e)
	{
		throw std::exception("BuildShaderProgram() Build shader failure. Abandoning");
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		std::cerr << "Linker error: " << strInfoLog << std::endl;

		delete[] strInfoLog;
		throw std::runtime_error("Shader could not be linked.");
	}
	
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}


void GLWrapper::UpdateImGui(bool mousePressed[2])
{
	ImGuiIO& io = ImGui::GetIO();

	// Setup resolution (every frame to accommodate for window resizing)
	int w, h;
	int display_w, display_h;
	glfwGetWindowSize(window, &w, &h);
	glfwGetFramebufferSize(window, &display_w, &display_h);
	io.DisplaySize = ImVec2((float)display_w, (float)display_h);                                   // Display size, in pixels. For clamping windows positions.

	// Setup time step
	static double time = 0.0f;
	const double current_time = glfwGetTime();
	io.DeltaTime = (float)(current_time - time);
	time = current_time;

	// Setup inputs
	// (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
	double mouse_x, mouse_y;
	glfwGetCursorPos(window, &mouse_x, &mouse_y);
	mouse_x *= (float)display_w / w;                                                               // Convert mouse coordinates to pixels
	mouse_y *= (float)display_h / h;
	io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);                                          // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
	io.MouseDown[0] = mousePressed[0] || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != 0;  // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
	io.MouseDown[1] = mousePressed[1] || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != 0;

	// Start the frame
	ImGui::NewFrame();
}
