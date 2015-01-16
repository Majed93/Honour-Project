/*
Majed Monem
*/

#ifdef _MSC_VER
#pragma warning (disable: 4996)         // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#include "imgui.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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

#include "object_ldr.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


//GLuint positionBufferObject, colourObject;
GLuint program[2];
GLuint vao, current;

/* Position and view globals */
GLfloat angle_x, angle_x_inc;
GLfloat angle_z, angle_z_inc;
GLfloat angle_y, angle_y_inc;

GLfloat x, x_inc;

GLfloat y, y_inc;

GLfloat z, z_inc;

GLfloat scale, scale_inc;

/* Uniforms*/
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

static GLWrapper *glw = new GLWrapper(800, 500, "Graphical Enigma Simutlator - Main Menu");

static GLFWwindow* window;
static GLuint fontTex;
static bool mousePressed[2] = { false, false };

//Variables for components
object_ldr plate_contacts;
// Shader variables
static int texture_location, ortho_location;
static int position_location, uv_location, colour_location;
static size_t vbo_max_size = 20000;
static unsigned int vbo_handle, vao_handle;

void display();
void drawObjects();
void createObjects();
// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - try adjusting ImGui::GetIO().PixelCenterOffset to 0.0f or 0.5f
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
static void ImImpl_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
{
	if (cmd_lists_count == 0)
		return;

	if (glw->mode == "En")
	{
		display();
	}
	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	// Setup texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fontTex);

	// Setup orthographic projection matrix
	const float width = ImGui::GetIO().DisplaySize.x;
	const float height = ImGui::GetIO().DisplaySize.y;
	const float ortho_projection[4][4] =
	{
		{ 2.0f / width, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 2.0f / -height, 0.0f, 0.0f },
		{ 0.0f, 0.0f, -1.0f, 0.0f },
		{ -1.0f, 1.0f, 0.0f, 1.0f },
	};
	glUseProgram(program[0]);
	glUniform1i(texture_location, 0);
	glUniformMatrix4fv(ortho_location, 1, GL_FALSE, &ortho_projection[0][0]);

	// Grow our buffer according to what we need
	size_t total_vtx_count = 0;
	for (int n = 0; n < cmd_lists_count; n++)
		total_vtx_count += cmd_lists[n]->vtx_buffer.size();
	glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
	size_t neededBufferSize = total_vtx_count * sizeof(ImDrawVert);
	if (neededBufferSize > vbo_max_size)
	{
		vbo_max_size = neededBufferSize + 5000;  // Grow buffer
		glBufferData(GL_ARRAY_BUFFER, vbo_max_size, NULL, GL_STREAM_DRAW);
	}

	// Copy and convert all vertices into a single contiguous buffer
	unsigned char* buffer_data = (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (!buffer_data)
		return;
	for (int n = 0; n < cmd_lists_count; n++)
	{
		const ImDrawList* cmd_list = cmd_lists[n];
		memcpy(buffer_data, &cmd_list->vtx_buffer[0], cmd_list->vtx_buffer.size() * sizeof(ImDrawVert));
		buffer_data += cmd_list->vtx_buffer.size() * sizeof(ImDrawVert);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(vao_handle);

	int cmd_offset = 0;
	for (int n = 0; n < cmd_lists_count; n++)
	{
		const ImDrawList* cmd_list = cmd_lists[n];
		int vtx_offset = cmd_offset;
		const ImDrawCmd* pcmd_end = cmd_list->commands.end();
		for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++)
		{
			glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x), (int)(pcmd->clip_rect.w - pcmd->clip_rect.y));
			glDrawArrays(GL_TRIANGLES, vtx_offset, pcmd->vtx_count);
			vtx_offset += pcmd->vtx_count;
		}
		cmd_offset = vtx_offset;
	}

	// Restore modified state
	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_SCISSOR_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void display() {

	glEnable(GL_DEPTH_TEST);
	/* Define the background colour */
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program[1]);
	//glBindVertexArray(vao);
	//glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	//glEnableVertexAttribArray(0);

	///* glVertexAttribPointer(index, size, type, normalised, stride, pointer)
	//index relates to the layout qualifier in the vertex shader and in
	//glEnableVertexAttribArray() and glDisableVertexAttribArray() */
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	//glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	//glEnableVertexAttribArray(1);

	///* glVertexAttribPointer(index, size, type, normalised, stride, pointer)
	//index relates to the layout qualifier in the vertex shader and in
	//glEnableVertexAttribArray() and glDisableVertexAttribArray() */
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);


	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::rotate(model, -angle_x, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = glm::rotate(model, -angle_z, glm::vec3(0, 0, 1)); //rotate z axis
	model = glm::rotate(model, -angle_y, glm::vec3(0, 1, 0)); //rotate y axis
	model = glm::translate(model, glm::vec3(x, y, z));
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	
	// Send our transformations to the currently bound shader,

	glm::mat4 projection = glm::perspective(80.0f, aspect_ratio, 0.1f, 100.0f);

	glm::mat4 view = glm::lookAt(
		glm::vec3(0, 0, 4),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0)
		);

	glm::mat3 normalmatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));
	glm::vec4 lightpos = view *  glm::vec4(0.25, 0.25, 1, 1.0);

	// Send our transformations to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	drawObjects();

	glDisableVertexAttribArray(0);
	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);

	/* Modify our animation variables */
	angle_x += angle_x_inc;
	angle_z += angle_z_inc;
	angle_y += angle_y_inc;
	x += x_inc;
	y += y_inc;
	z += z_inc;
	scale += scale_inc;
}

static const char* ImImpl_GetClipboardTextFn()
{
	return glfwGetClipboardString(window);
}

static void ImImpl_SetClipboardTextFn(const char* text)
{
	glfwSetClipboardString(window, text);
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glw->width = w;
	glw->height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 1280.f*4.f) / ((float)h / 960.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int k, int s, int action, int mods)
{
	//if (action != GLFW_PRESS) return;

	if (k == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (k == 'Q') angle_x_inc += 0.1f;
	if (k == 'W') angle_x_inc -= 0.1f;
	if (k == 'A') angle_z_inc += 0.1f;
	if (k == 'S') angle_z_inc -= 0.1f;
	if (k == 'Z') angle_y_inc += 0.1f;
	if (k == 'X') angle_y_inc -= 0.1f;

	if (k == GLFW_KEY_UP) y_inc += 0.0001f;

	if (k == GLFW_KEY_DOWN) y_inc -= 0.0001f;

	if (k == GLFW_KEY_LEFT) x_inc += 0.0001f;

	if (k == GLFW_KEY_RIGHT) x_inc -= 0.0001f;

	if (k == 'I') scale_inc += 0.0001f;
	if (k == 'K') scale_inc -= 0.0001f;

	ImGuiIO& io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[k] = true;
	if (action == GLFW_RELEASE)
		io.KeysDown[k] = false;
	io.KeyCtrl = (mods & GLFW_MOD_CONTROL) != 0;
	io.KeyShift = (mods & GLFW_MOD_SHIFT) != 0;

}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS && button >= 0 && button < 2)
		mousePressed[button] = true;
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel += (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
	if (c > 0 && c < 0x10000)
		ImGui::GetIO().AddInputCharacter((unsigned short)c);
}


// GLFW callbacks to get events
static void glfw_error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

//Initialize objects
void createObjects()
{
	plate_contacts.load_obj("objects/spring-loaded_level.obj");
	//plate_contacts.smoothNormals(); //Might not need this
	plate_contacts.createObject();
}

//Draw all the components
void drawObjects()
{
	plate_contacts.drawObject();
}
void init(GLWrapper *glw)
{
	angle_x = 0;
	angle_x_inc = 0;

	angle_z = 0;
	angle_z_inc = 0;

	angle_y = 0;
	angle_y_inc = 0;

	x = 0;
	x_inc = 0;

	y = 0;
	y_inc = 0;

	z = 0;
	z_inc = 0;

	scale = 0.5;
	scale_inc = 0;
	aspect_ratio = 1.3333f;
	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	///* Create a vertex buffer object to store vertices */
	//glGenBuffers(1, &positionBufferObject);
	//glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);


	///* Create a vertex buffer object to store vertex colours */
	//glGenBuffers(1, &colourObject);
	//glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColours), vertexColours, GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	try
	{
		program[0] = glw->LoadShader("imgui.vert", "imgui.frag");
		program[1] = glw->LoadShader("basic.vert", "basic.frag");
	}
	catch (std::exception &e)
	{
		std::cout << "Caught exception: " << e.what() << std::endl;
		std::cin.ignore();
		exit(0);
	}

	texture_location = glGetUniformLocation(program[0], "Texture");
	ortho_location = glGetUniformLocation(program[0], "ortho");
	position_location = glGetAttribLocation(program[0], "Position");
	uv_location = glGetAttribLocation(program[0], "UV");
	colour_location = glGetAttribLocation(program[0], "Colour");

	glGenBuffers(1, &vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
	glBufferData(GL_ARRAY_BUFFER, vbo_max_size, NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &vao_handle);
	glBindVertexArray(vao_handle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
	glEnableVertexAttribArray(position_location);
	glEnableVertexAttribArray(uv_location);
	glEnableVertexAttribArray(colour_location);

	glVertexAttribPointer(position_location, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(colour_location, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	createObjects();

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program[1], "model");
	viewID = glGetUniformLocation(program[1], "view");
	projectionID = glGetUniformLocation(program[1], "projection");
	lightposID = glGetUniformLocation(program[1], "lightpos");
	normalmatrixID = glGetUniformLocation(program[1], "normalmatrix");
}

void InitImGui()
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = 1.0f / 60.0f;                                  // Time elapsed since last frame, in seconds (in this sample app we'll override this every frame because our timestep is variable)
	io.PixelCenterOffset = 0.5f;                                  // Align OpenGL texels
	io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
	io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
	io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
	io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
	io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
	io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
	io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
	io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
	io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
	io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
	io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

	io.RenderDrawListsFn = ImImpl_RenderDrawLists;
	io.SetClipboardTextFn = ImImpl_SetClipboardTextFn;
	io.GetClipboardTextFn = ImImpl_GetClipboardTextFn;

	// Load font texture
	glGenTextures(1, &fontTex);
	glBindTexture(GL_TEXTURE_2D, fontTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	const void* png_data;
	unsigned int png_size;
	ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
	int tex_x, tex_y, tex_comp;
	void* tex_data = stbi_load_from_memory((const unsigned char*)png_data, (int)png_size, &tex_x, &tex_y, &tex_comp, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_x, tex_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
	stbi_image_free(tex_data);
}


//Main Application Code
int main(int argc, char ** argv)
{

	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);
	glw->setMouseButtonCallback(glfw_mouse_button_callback);
	glw->setScrollCallback(glfw_scroll_callback);
	glw->setCharCallback(glfw_char_callback);
	
	init(glw);
	InitImGui();
	
	glw->eventLoop(mousePressed);

	if (vao_handle) glDeleteVertexArrays(1, &vao_handle);
	if (vbo_handle) glDeleteBuffers(1, &vbo_handle);
	delete(glw);
	return 0;
}
