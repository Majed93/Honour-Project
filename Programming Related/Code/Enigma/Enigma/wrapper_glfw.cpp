/**
  wrapper_glfw.cpp
  Modified from the OpenGL GLFW example to provide a wrapper GLFW class
  and to include shader loader functions to include shaders as text files
  Iain Martin August 2014
  Majed Monem 2014/15 Graphical Enigma Simulator Honours Project
  */
#ifdef _MSC_VER
#pragma warning (disable: 4996)         // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#include "imgui.h"
#include "wrapper_glfw.h"
#include "enigma.h"
// Glfw/Glew
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
#include <cctype>

/* Constructor for wrapper object */
GLWrapper::GLWrapper(int width, int height, char *title) {

	this->width = width;
	this->height = height;
	this->title = title;
	this->fps = 60;
	this->running = true;
	complete = false;
	changed = new bool[25];
	platechange = new bool[25];
	reset();

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
		static bool help_notitlebar = false;
		static bool help_nomove = false;
		static bool help_noscrollbar = false;
		
		static bool rotorhelp_nomove = false;
		static bool rotorhelp_noscrollbar = false;


		static float fill_alpha = 0.65f;

		const char* rotorno[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII" };
		
		const char* reflectorno[] = { "Beta", "Gamma", "Reflector A", "Reflector B", "Reflector C" };
		
		ImGuiStyle style;

		const ImGuiWindowFlags layout_flags = (no_titlebar ? ImGuiWindowFlags_NoTitleBar : 0) | (no_border ? 0 : ImGuiWindowFlags_ShowBorders) | (no_resize ? ImGuiWindowFlags_NoResize : 0) | (no_move ? ImGuiWindowFlags_NoMove : 0) | (no_scrollbar ? ImGuiWindowFlags_NoScrollbar : 0);
		const ImGuiWindowFlags help_flags = (help_notitlebar ? ImGuiWindowFlags_NoTitleBar : 0) | (no_border ? 0 : ImGuiWindowFlags_ShowBorders) | (no_resize ? ImGuiWindowFlags_NoResize : 0) | (help_nomove ? ImGuiWindowFlags_NoMove : 0) | (help_noscrollbar ? ImGuiWindowFlags_NoScrollbar : 0);
		const ImGuiWindowFlags rotorhelp_flags = (help_notitlebar ? ImGuiWindowFlags_NoTitleBar : 0) | (no_border ? 0 : ImGuiWindowFlags_ShowBorders) | (no_resize ? ImGuiWindowFlags_NoResize : 0) | (rotorhelp_nomove ? ImGuiWindowFlags_NoMove : 0) | (rotorhelp_noscrollbar ? ImGuiWindowFlags_NoScrollbar : 0);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImColor::ImColor(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::ImColor(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::ImColor(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)));

		if (show_rotor == false)
		{
			transition_done = false;
			//transistion_current = 0.0f;
			if (transition_current >= 0.0f)
			{
				transition_current -= trans_inc;
			}
		}
		//Main Menu
		if (show_main == true)
		{
			//system("CLS");//CLEARS CONSOLE *****************TAKE THIS OUT WHEN NOT DEBUGGING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if (!resized)
			{
				width = 400;
				height = 300;
				glfwSetWindowSize(window, width, height);
				const GLFWvidmode* vmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwSetWindowPos(window, vmode->width / 2 - width / 2, vmode->height / 2 - height / 2); //Centre screen
				resized = true;
			}
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2((width / 2) - 50 - style.WindowPadding.x, (height / 4) + style.WindowPadding.y));

			mode = "";
			show_encrypt = false;
			show_decrypt = false;
			show_help = false;
			
			if (!complete)
			{
				reset();
			}

			ImGui::Begin("", &show_main, ImVec2(100, 100), fill_alpha, layout_flags);
			title = "Graphical Enigma Simulator - Main Menu";
			ImGui::SetWindowPos(ImVec2(0,0), 0);
			ImGui::SetWindowSize(ImVec2(width, height), 0);

		
			show_encrypt ^= ImGui::Button("Encrypt", ImVec2(100, 25));// , ImVec2(60, 20), true);
			show_decrypt ^= ImGui::Button("Decrypt", ImVec2(100, 25));
			show_exit ^= ImGui::Button("Exit", ImVec2(100, 25));
			

			ImGui::End();
			ImGui::PopStyleVar();

		}

		if (transition_done == false)
		{
			//Show Encryption screen
			if (show_encrypt == true)
			{
				if (complete == true)
				{
					resized = false;
				}
				if (!resized)
				{
					width = 800;
					height = 500;
					glfwSetWindowSize(window, width, height);
					const GLFWvidmode* vmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
					glfwSetWindowPos(window, vmode->width / 2 - width / 2, vmode->height / 2 - height / 2); //Centre screen
					resized = true;
				}
				mode = "En";

				show_main = false;
				ImGui::Begin("Encrypt", &show_encrypt, ImVec2(100, 100), fill_alpha, layout_flags);
				//no_titlebar = false;
				title = "Graphical Enigma Simulator - Encrypt";

				ImGui::SetWindowPos(ImVec2(0, (height * 0.72f) + transition_current), 0);
				ImGui::SetWindowSize(ImVec2(width, height - (height * 0.72f)), 0);
				if (show_rotor == true)
				{
					transition_current += trans_inc;
				}
				
				if (transition_current > transition_limit)
				{
					transition_done = true;
				}
				/*WAS TRYING TO GET ZOOM ON MOUSE OVER MAYBE TRY LATER?
				ImVec2 tex_screen_pos = ImGui::GetCursorScreenPos();

				struct Num{
				static inline float  ImClamp(float v, float mn, float mx)                       { return (v < mn) ? mn : (v > mx) ? mx : v; }

				};

				float focus_sz = 16.0f;//LOWER THE MORE ZOOM
				float tex_w = (float)ImGui::GetIO().Fonts->TexWidth;
				float tex_h = (float)ImGui::GetIO().Fonts->TexHeight;
				ImTextureID tex_id = ImGui::GetWindowDrawList;

				ImGui::BeginTooltip();

				float focus_x = Num::ImClamp(ImGui::GetMousePos().x - tex_screen_pos.x - focus_sz * 0.5f, 0.0f, tex_w - focus_sz);
				float focus_y = Num::ImClamp(ImGui::GetMousePos().y - tex_screen_pos.y - focus_sz * 0.5f, 0.0f, tex_h - focus_sz);
				ImGui::Text("Min: (%.2f, %.2f)", focus_x, focus_y);
				ImGui::Text("Max: (%.2f, %.2f)", focus_x + focus_sz, focus_y + focus_sz);
				ImVec2 uv0 = ImVec2((focus_x) / tex_w, (focus_y) / tex_h);
				ImVec2 uv1 = ImVec2((focus_x + focus_sz) / tex_w, (focus_y + focus_sz) / tex_h);
				ImGui::Image(tex_id, ImVec2(50, 50), uv0, uv1, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
				ImGui::EndTooltip();
				*/

				show_main ^= ImGui::Button("Back To Main Menu");
				ImGui::SameLine();
				show_help ^= ImGui::Button("Help");

				ImGui::Text("Rotor #");

				ImGui::SameLine();
				ImGui::Spacing();
				ImGui::SameLine();

				ImGui::PushItemWidth(200);
				ImGui::Combo(" ", &combopos1, rotorno, IM_ARRAYSIZE(rotorno), 4);
				ImGui::PopItemWidth();

				if (combopos1 != prev1)
				{
					setRotorOne(rotors[combopos1]);
					setStaticrOne(rotors[combopos1]);
					prev1 = combopos1;

				}

				ImGui::SameLine();
				ImGui::Text("  ");
				ImGui::SameLine();

				ImGui::Text("Reflector #");

				ImGui::SameLine();
				ImGui::Spacing();
				ImGui::SameLine();

				ImGui::PushItemWidth(200);
				ImGui::Combo("", &combopos2, reflectorno, IM_ARRAYSIZE(reflectorno), 4);
				ImGui::PopItemWidth();

				if (combopos2 != prev2)
				{
					setRelfector(reflectors[combopos2]);
					prev2 = combopos2;
				}
				static float wrap_width = 10.0f;

				ImGui::PushItemWidth(width - 25);
				ImGui::Text("Plain Text");
				struct TextFilters {
					static int FilterAZ(ImGuiTextEditCallbackData* data)
					{
						ImWchar c = data->EventChar;
						if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
						{
							return 1;
						}
						else
						{
							if ((c >= 'a' && c <= 'z'))
							{
								data->EventChar += 'A' - 'a';
							}
						}
						return 0;
					}
				};

				if (ImGui::GetWindowIsFocused() && !ImGui::IsAnyItemActive())
					ImGui::SetKeyboardFocusHere();

				ImGui::InputText("", strPlain, IM_ARRAYSIZE(strPlain), ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterAZ);


				ImGui::Text("Cipher Text");

				style.ItemInnerSpacing.x = 10.f;
				ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(wrap_width + width, 10.0f + transition_current), ImVec2(wrap_width + width, ImGui::GetTextLineHeight() + transition_current), 0xff808080);
				ImGui::Text(encrypted.c_str());

				ImGui::GetWindowDrawList()->AddRect(ImVec2(ImGui::GetItemBoxMin().x - 2.f, ImGui::GetItemBoxMin().y + ImGui::GetTextLineHeight() - 15.f + transition_current), ImVec2(width - 11.f, height - 15.0f + transition_current), 0xff808080);
				ImGui::PopItemWidth();

				//FIX BACKSPACE
				
				if (strlen(strPlain) <= encrypted.length())
				{
					if (strlen(strPlain) > 0)
					{
						if ((encrypted.length() != strlen(strPlain)))
						{
							encrypted.pop_back();
						}
					}

					//SPECIAL CASE
					if (strlen(strPlain) < 1 && encrypted != "")
					{
						encrypted.pop_back();
					}
				}
				complete = false;


				ImGui::End();
			}

			/***************************************************************************************************************/
			//Show Decryption screen
			if (show_decrypt == true)
			{

				if (complete == true)
				{
					resized = false;
				}

				if (!resized)
				{
					width = 800;
					height = 500;
					glfwSetWindowSize(window, width, height);
					const GLFWvidmode* vmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
					glfwSetWindowPos(window, vmode->width / 2 - width / 2, vmode->height / 2 - height / 2); //Centre screen
					resized = true;
				}
				mode = "De";
				show_main = false;

				ImGui::Begin("", &show_decrypt, ImVec2(100, 100), fill_alpha, layout_flags);
				title = "Graphical Enigma Simulator - Decrypt";

				ImGui::SetWindowPos(ImVec2(0, (height * 0.72f) + transition_current), 0);
				ImGui::SetWindowSize(ImVec2(width, height - (height * 0.72f)), 0);
				if (show_rotor == true)
				{
					transition_current += trans_inc;
				}

				if (transition_current > transition_limit)
				{
					transition_done = true;
				}

				show_main ^= ImGui::Button("Back To Main Menu");
				ImGui::SameLine();
				show_help ^= ImGui::Button("Help");

				ImGui::Text("Rotor #");

				ImGui::SameLine();
				ImGui::Spacing();
				ImGui::SameLine();

				ImGui::PushItemWidth(200);
				ImGui::Combo(" ", &combopos1, rotorno, IM_ARRAYSIZE(rotorno), 4);
				ImGui::PopItemWidth();

				if (combopos1 != prev1)
				{
					setRotorOne(rotors[combopos1]);
					setStaticrOne(rotors[combopos1]);
					prev1 = combopos1;
				}

				ImGui::SameLine();
				ImGui::Text("  ");
				ImGui::SameLine();

				ImGui::Text("Reflector #");

				ImGui::SameLine();
				ImGui::Spacing();
				ImGui::SameLine();

				ImGui::PushItemWidth(200);
				ImGui::Combo("", &combopos2, reflectorno, IM_ARRAYSIZE(reflectorno), 4);
				ImGui::PopItemWidth();

				if (combopos2 != prev2)
				{
					setRelfector(reflectors[combopos2]);
					prev2 = combopos2;
				}

				static float wrap_width = 10.0f;

				ImGui::PushItemWidth(width - 25);
				ImGui::Text("Plain Text");

				style.ItemInnerSpacing.x = 10.f;
				ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(wrap_width + width, 10.0f + transition_current), ImVec2(wrap_width + width, ImGui::GetTextLineHeight() + transition_current), 0xff808080);
				ImGui::Text(decrypted.c_str());

				ImGui::GetWindowDrawList()->AddRect(ImVec2(ImGui::GetItemBoxMin().x - 2.f, ImGui::GetItemBoxMin().y + ImGui::GetTextLineHeight() - 15.f + transition_current), ImVec2(width - 11.f, height - 55.0f + transition_current), 0xff808080);

				ImGui::Text("Cipher Text");

				struct TextFilters {
					static int FilterAZ(ImGuiTextEditCallbackData* data)
					{
						ImWchar c = data->EventChar;
						if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
						{
							return 1;
						}
						else
						{
							if ((c >= 'a' && c <= 'z'))
							{
								data->EventChar += 'A' - 'a';
							}
						}
						return 0;
					}
				};

				if (ImGui::GetWindowIsFocused() && !ImGui::IsAnyItemActive())
					ImGui::SetKeyboardFocusHere();

				ImGui::InputText(" ", strCipher, IM_ARRAYSIZE(strCipher), ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterAZ);
				ImGui::PopItemWidth();

				//FIX BACKSPACE
				if (strlen(strCipher) <= decrypted.length())
				{
					if (strlen(strCipher) > 0)
					{
						if ((decrypted.length() != strlen(strCipher)))
						{
							decrypted.pop_back();
						}
					}
					//SPECIAL CASE
					if (strlen(strCipher) < 1 && decrypted != "")
					{
						decrypted.pop_back();
					}
				}
				complete = false;

				ImGui::End();
			}

		}
		//Help Screen
		if (show_help == true)
		{
			ImGui::PushStyleColor(ImGuiCol_TitleBg, ImColor::ImColor(ImVec4(0.15f, 0.15f, 0.15f, 1.0f)));
			ImGui::PushStyleColor(ImGuiCol_CloseButton, ImColor::ImColor(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)));
			ImGui::PushStyleColor(ImGuiCol_CloseButtonHovered, ImColor::ImColor(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)));
			ImGui::PushStyleColor(ImGuiCol_CloseButtonActive, ImColor::ImColor(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)));
			
			ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImColor::ImColor(ImVec4(0.15f, 0.15f, 0.15f, 1.0f)));
			ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImColor::ImColor(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)));
			ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImColor::ImColor(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)));
			ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImColor::ImColor(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)));


			help_notitlebar = false;
			help_nomove = false;
			help_noscrollbar = false;

			ImGui::Begin("Help", &show_help, ImVec2(300, 250), 0.95f, help_flags);
			
			ImGui::SetWindowSize(ImVec2(300, 250), 0);

			if (show_rotor == true)
			{
				ImGui::SetWindowPos(ImVec2((width/1.25), (height / 12)), 0);
			}

			ImGui::TextWrapped("Graphical Enigma Simulator.");

			show_rotor ^= ImGui::Button("Show Rotor Details");

			ImGui::TextWrapped("Please refer to user guide for more information.");

			ImGui::Spacing();

			ImGui::TextWrapped("This simulation shows encryption and decryption through only one rotor. Input shall be auto capitalised.");
			
			ImGui::Spacing();
			
			ImGui::TextWrapped("Process explained: ");

			ImGui::Spacing();

			ImGui::TextWrapped("Once a letter pressed, it is passed to the pins which then map to the corresponding pins, depending on the rotor setting. Once passed through the pins, it is then passed to the reflector where it will be mapped to the reflector and passed back to the pins. Once passed back to the pins, the decrypted letter can then be calculate. ");

			ImGui::Spacing();

			ImGui::TextWrapped("The green wire represents the path of the letter you have pressed up until the reflector, where it is then crossed over and the path is returned, shown by the red wiring.");

			ImGui::Spacing();

			ImGui::TextWrapped("The rotor automatically rotates once you press a letter by click, and also reverse upon backspace. Please note that the rotor does take a few seconds to rotate each click therefore if many letters are entered in a short amount of time, there may be a delay in the rotor completing its rotation. For optimal experience it is recommended to take your time.");

			ImGui::Spacing();

			ImGui::TextWrapped("Cut, Copy and Paste are NOT facilitated");

			ImGui::Spacing();

			ImGui::TextWrapped("DO NOT HOLD LETTERS DOWN AS THE ROTOR CAN ONLY DO ONE LETTER AT A TIME. ESPECIALLY THE BACKSPACE KEY AS THIS WILL RESULT IN THE OUTPUT TEXT BEING INCORRECT!");

			ImGui::Spacing();

			ImGui::TextWrapped("The only controls available are the movements of the camera.");
			
			ImGui::Spacing();

			ImGui::TextWrapped("Controls");
			ImGui::TextWrapped("Arrow Keys - Left: Move view left (Camera anti-clockwise around y-axis) ");
			ImGui::TextWrapped("Arrow Keys - Right: Move view right (Camera clockwise around y-axis)");
			ImGui::TextWrapped("Arrow Keys - Up: Move view up");
			ImGui::TextWrapped("Arrow Keys - Down: Move view down");
			ImGui::TextWrapped("Numbers - 1: Rotate rotor clockwise");
			ImGui::TextWrapped("Numbers - 2: Rotate rotor anti-clockwise");
			ImGui::TextWrapped("Numbers - 3: Move camera around right");
			ImGui::TextWrapped("Numbers - 4: Move camera around left");
			ImGui::TextWrapped("Numbers - 5: Move camera around up");
			ImGui::TextWrapped("Numbers - 6: Move camera around down");
			ImGui::TextWrapped("Numbers - 7: Zoom out");
			ImGui::TextWrapped("Numbers - 8: Zoom in");
			ImGui::TextWrapped("Numbers - 9: Move left");
			ImGui::TextWrapped("Numbers - 0: Move right");
			ImGui::TextWrapped("Left Square Bracket ('['): Move up");
			ImGui::TextWrapped("Right Square Bracket (']'): Move down");

			if (show_rotor == true)
			{
				ImGui::SetWindowCollapsed(true);
			}
			else
			{
				ImGui::SetWindowCollapsed(false);
			}
			ImGui::End();
			ImGui::PopStyleColor(8);

		}

		//Exit Application
		if(show_exit == true){
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (show_rotor == true)
		{
			//std::cout << transistion_current << std::endl;
			
			ImGui::PushStyleColor(ImGuiCol_TitleBg, ImColor::ImColor(ImVec4(0.15f, 0.15f, 0.15f, 1.0f)));
			ImGui::PushStyleColor(ImGuiCol_CloseButton, ImColor::ImColor(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)));
			ImGui::PushStyleColor(ImGuiCol_CloseButtonHovered, ImColor::ImColor(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)));
			ImGui::PushStyleColor(ImGuiCol_CloseButtonActive, ImColor::ImColor(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)));


			rotorhelp_nomove = true;
			rotorhelp_noscrollbar = true;

			//CAN'T GET IT TO BE ALIGNED WITH IMAGE!
			ImGui::Begin("Rotor Details", &show_rotor, ImVec2((width / 2.2), 100), 0.0f, rotorhelp_flags);

			
			ImGui::SetWindowPos(ImVec2((width / 2) - ((width / 2.2) / 2), 25.0f));

			ImGui::End();
			ImGui::PopStyleColor(4);

		}

		//TO ENSURE THE FUNCTIONS COME BACK
		if (show_help == false)
		{
			show_rotor = false;
		}


		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSetWindowTitle(window, title);
		ImGui::Render();
		glfwSwapBuffers(window);
	}

	ImGui::Shutdown();
	glfwTerminate();
	return 0;
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

/*Update current state of UI*/
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

//Returns alphabet string
std::string GLWrapper::getAlphabet()
{
	return machine.getAlphabet();
}

//Returns rotor one string
std::string GLWrapper::getRotorOne()
{
	return machine.getRotorOne();
}

//Sets rotor one string
void GLWrapper::setRotorOne(std::string str)
{
	machine.setRotorOne(str);
}

//Returns static rotor string
std::string GLWrapper::getStaticrOne()
{
	return machine.getStaticrOne();
}

//Sets static rotor string
void GLWrapper::setStaticrOne(std::string str)
{
	machine.setStaticrOne(str);
}

//Returns reflector string
std::string GLWrapper::getReflector()
{
	return machine.getReflector();
}

//Sets reflector string
void GLWrapper::setRelfector(std::string str)
{
	machine.setReflector(str);
}

//Encryption occurs here
void GLWrapper::Encrypt(char k)
{
	//std::cout << "letter index:" << machine.getIndex(k) << std::endl;
	//std::cout << "cipher letter:" << getCiphered(machine.getIndex(k)) << std::endl;
	//getCiphered(machine.getIndex(k));

	int index = machine.getIndex(k);

	for (int i = 0; i < 26; i++)
	{
		changed[i] = false;
		platechange[i] = false;
	}

	int totalindex = index + changenum;
	if (totalindex > 25)
	{
		totalindex -= 26;
	}
	changed[totalindex] = true;

	//DEBUGGING
	std::cout << "Encrypt - index " << totalindex << std::endl;

	encrypted += machine.encrypt(index);
	machine.offset(1);
	count += 1;
}

//Decryption
void GLWrapper::Decrypt(char k)
{
	int index = machine.getIndex(k);

	for (int i = 0; i < 26; i++)
	{
		changed[i] = false;
		platechange[i] = false;
	}
	changenum = count;
	
	int totalindex = index;// +changenum;
	if (totalindex > 25)
	{
		totalindex -= 26;
	}

	changed[totalindex] = true;

	//DEBUGGING
	std::cout << "Decrypt - index " << totalindex << std::endl;

	decrypted += machine.decrypt(index, k);
	machine.offset(1);
	count += 1;
}

//Reset variables
void GLWrapper::reset()
{
	encrypted = "";
	decrypted = "";
	count = 0;
	for (int i = 0; i < 26; i++)
	{
		changed[i] = false;
		platechange[i] = false;
	}

	for (int i = 0; i < 70; i++)
	{
		strPlain[i] = '\0';
		strCipher[i] = '\0';
	}
	std::string strRotorArray[] = { "EKMFLGDQVZNTOWYHXUSPAIBRCJ", 
		"AJDKSIRUXBLHWTMCQGZNPYFVOE", 
		"BDFHJLCPRTXVZNYEIWGAKMUSQO",
		"ESOVPZJAYQUIRHXLNFTGKDCMWB",
		"VZBRGITYUPSDNHLXAWMJQOFECK",
		"JPGVOUMFYQBENHZRDKASXLICTW",
		"NZJHGRCXMYSWBOUFAIVLPEKQDT",
		"FKQHTLXOCBJSPDZRAMEWNIUYGV" };
	
	std::string strReflectorArray[] = {"LEYJVCNIXWPBQMDRTAKZGFUHOS",
		"FSOKANUERHMBTIYCWLQPZXVGJD",
		"EJMZALYXVBWFCRQUONTSPIKHGD",
		"YRUHQSLDPXNGOKMIEBFZCWVJAT",
		"FVPJIAOYEDRZXWGCTKUQSBNMHL" };

	for (int i = 0; i < 7; i++)
	{
		rotors[i] = strRotorArray[i];
		if (i < 5)
		{
			reflectors[i] = strReflectorArray[i];
		}
	}
	prev1, combopos1 = 0;
	prev2, combopos2 = 3;

	setRotorOne(rotors[combopos1]);
	setStaticrOne(rotors[combopos1]);
	setRelfector(reflectors[combopos2]);
	rotation, introtation = 0.0f;
	machine.reset();
	
	complete = true;
	resized = false;
	show_rotor = false;
	transition_current = 0.0f;
	transition_limit = height - 175.0f;
	transition_done = false;
}

