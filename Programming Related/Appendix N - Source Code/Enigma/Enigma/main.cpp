/*
Majed Monem 2014/15 Graphical Enigma Simulator Honours Project
Any commented out code remains as it is has a purpose such as printing out to console for debugging(need to change main function) 
and code which would be helpful for future development.
*/

#ifdef _MSC_VER
#pragma warning (disable: 4996)         // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"   // warning: unused function
#endif

#pragma comment(lib, "soil.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")

#include "imgui.h"
#include <Windows.h>
#include "resource.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include "wrapper_glfw.h"

// Glfw/Glew
#define GLEW_STATIC
#include <glew.h>
#include <GLFW/glfw3.h>
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))


/* GLM core */
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtc/random.hpp"

#include "object_ldr.h"
#include <SOIL.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>


GLuint program[2];
GLuint vao, current;
GLuint positionBufferObject, colourObject;


/* Position and view globals */
GLfloat angle_x, angle_x_inc;
GLfloat angle_z, angle_z_inc;
GLfloat angle_y, angle_y_inc;
GLfloat lightx, lighty, lightz;
GLfloat x, y, z;
GLfloat vx, vy, vz;
GLfloat cx, cy, cz;
GLfloat scale, scale_inc;

GLfloat platex[26], platez[26];
GLfloat pinx[26], pinz[26];

GLfloat pi = 3.141592;
GLfloat slice = 2 * pi / 26;
GLuint numpoints = 2;
GLfloat rotationinc = 0.1f;

GLuint count, countpin, refcount, pincountv, platecountv = 0.0f;
GLfloat newnumplatez, newnumpinx, newnumpinz, newnumplatex, newnumrefz, newnumrefx = 0.0f;
GLfloat convplatex, convplatez, convpinx, convpinz, convrefx, convrefz = 0.0f;

std::size_t newcount;
GLfloat* line_vertex;
GLboolean done = false;

/* Uniforms*/
GLuint modelID, viewID, projectionID, normalmatrixID, lightposID, componentID, color1ID, color2ID, color3ID, color4ID, emitmodeID, textmodeID;
glm::mat4 model, view, projection;
glm::vec4 lightpos;
glm::mat3 normalmatrix;

GLuint comp, emitmode, textmode;
GLfloat color4;
GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

//Our window created by GLFW
static GLWrapper *glw = new GLWrapper(400, 300, "Graphical Enigma Simulator - Main Menu");

static GLuint fontTex;
static bool mousePressed[2] = { false, false };

//Variables for components
object_ldr notched_ring;
object_ldr contact;
object_ldr alphabet_tyre;
object_ldr plate_contacts;
object_ldr pin_contact;
object_ldr spring_loaded_lever;
object_ldr hub;
object_ldr finger_wheel;
object_ldr ratchet_wheel;
object_ldr back_contact;
object_ldr reflector;

// Shader variables
static int texture_location, proj_mtx_location;
static int position_location, uv_location, colour_location;
static size_t vbo_max_size = 20000;
static unsigned int vbo_handle, vao_handle;


GLuint texID, texCoordsObject;
GLuint textpositionBufferObject, textcolourObject, textnormalsBufferObject;


//Function definitions as required
void display();
void displayHelp();
void drawObjects();
void drawPlates();
void drawPins();
void createObjects();
void drawBuffers();
void createBuffers();
void calculateXZ();
void illuminate();
GLuint mapAlphabet(int number,int type);
void set_linevertex(GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2);


// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - try adjusting ImGui::GetIO().PixelCenterOffset to 0.0f or 0.5f
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
static void ImImpl_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
{
	if (cmd_lists_count == 0)
		return;

	//Check if Encrypt or Decrypt mode
	if (glw->mode == "En")
	{
		//Do once for efficiency.
		if (done == false)
		{
			createBuffers();
		}
		display();
	}
	else if (glw->mode == "De")
	{
		//Do once for efficiency.
		if (done == false)
		{
			createBuffers();
		}
		display();
	}

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

    // Setup orthographic projection matrix
    const float width = ImGui::GetIO().DisplaySize.x;
    const float height = ImGui::GetIO().DisplaySize.y;
    const float ortho_projection[4][4] =
    {
        { 2.0f/width,	0.0f,			0.0f,		0.0f },
        { 0.0f,			2.0f/-height,	0.0f,		0.0f },
        { 0.0f,			0.0f,			-1.0f,		0.0f },
        { -1.0f,		1.0f,			0.0f,		1.0f },
    };

    glUseProgram(program[0]);
    glUniform1i(texture_location, 0);
    glUniformMatrix4fv(proj_mtx_location, 1, GL_FALSE, &ortho_projection[0][0]);

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
            glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->texture_id);
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

//Objects drawing to screen here NOT GUI
void display() {

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Define the background colour */
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program[1]);
	
	// Model matrix : an identity matrix (model will be at the origin)
	model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(x, y, z));
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	
	// Send our transformations to the currently bound shader,

	projection = glm::perspective(80.0f, aspect_ratio, 0.1f, 100.0f);

	view = glm::lookAt(
		glm::vec3(vx, vy, vz),
		glm::vec3(cx, cy, cz),
		glm::vec3(0, 1, 0)
		);

	//// Apply rotations to the view position
	view = glm::rotate(view, -angle_x, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = glm::rotate(view, -angle_y, glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = glm::rotate(view, -angle_z, glm::vec3(0, 0, 1));

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));
	lightpos = view *  glm::vec4(lightx, lighty, lightz, 1.0);

	// Send our transformations to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));
	
	drawObjects();

	if (glw->show_rotor == true)
	{
		displayHelp();
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

/*DRAW OUR HELP GUIDE*/
void displayHelp()
{
	//Make view front on
	view = glm::lookAt(
		glm::vec3(0, 0, 4),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0)
		);

	textmode = 1;
	glUniform1ui(textmodeID, textmode);

	/* Bind cube vertices. Note that this is in attribute index 0 */
	glBindBuffer(GL_ARRAY_BUFFER, textpositionBufferObject);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube colours. Note that this is in attribute index 1 */
	glBindBuffer(GL_ARRAY_BUFFER, textcolourObject);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube normals. Note that this is in attribute index 2 */
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, textnormalsBufferObject);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	/* Bind cube texture coords. Note that this is in attribute index 3 */
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsObject);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, z));
	model = glm::scale(model, glm::vec3(12, 12, -0.00000000001f));//scale equally in all axis
	model = glm::rotate(model, 0.0f, glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis to make it face user

	lightpos = view *  glm::vec4(0.0f, 0.0f, -2.5f, 1.0);

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	/* Draw our texture*/
	glBindTexture(GL_TEXTURE_2D, texID);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	textmode = 0;
	glUniform1ui(textmodeID, textmode);
}

//Paste
static const char* ImImpl_GetClipboardTextFn()
{
	return glfwGetClipboardString(glw->window);
}

//Copy
static void ImImpl_SetClipboardTextFn(const char* text)
{
	glfwSetClipboardString(glw->window, text);
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glw->width = w;
	glw->height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 1280.f*4.f) / ((float)h / 960.f*3.f);
}

/* change camera, and callback on encrypt and decrypt methods on key press */
static void keyCallback(GLFWwindow* window, int k, int s, int action, int mods)
{
	//64 and 90 are all alphabet values
	//WHILE HELP MENUS ARE NOT ACTIVE
	if (glw->show_rotor == false || glw->show_help == false)
	{

		if (k > 64 && k < 91 && action == GLFW_PRESS && strlen(glw->strPlain) < 95)
		{
			if (glw->mode == "En")
			{
				glw->Encrypt((char)k);
			}
			else if (glw->mode == "De")
			{
				glw->Decrypt((char)k);
			}
		}

		//Spacebar
		else if (k == 32)
		{
			//MAYBE DO FOR A CHARACTER LIKE X AND 4 BLOCK TEXT
		}

		//Backspace
		if (k == 259 && action == GLFW_PRESS)
		{
			if ((glw->encrypted != "" || glw->decrypted != ""))
			{
				if (glw->mode == "En")
				{
					glw->calculated = false;
					glw->count = 0;
				}
				else if (glw->mode == "De")
				{
					glw->calculated = false;
					glw->count = 0;
				}
			}
		}
		
	}

	if (k == GLFW_KEY_LEFT) angle_y -= 0.5f;

	if (k == GLFW_KEY_RIGHT) angle_y += 0.5f;

	if (k == GLFW_KEY_UP) angle_x -= 0.5f;

	if (k == GLFW_KEY_DOWN) angle_x += 0.5f;

	if (k == GLFW_KEY_1) angle_z += 0.75f;
	
	if (k == GLFW_KEY_2) angle_z -= 0.75f;

	if (k == GLFW_KEY_3) vx += 0.1f;

	if (k == GLFW_KEY_4) vx -= 0.1f;

	if (k == GLFW_KEY_5) vy += 0.1f;

	if (k == GLFW_KEY_6) vy -= 0.1f;

	if (k == GLFW_KEY_7) vz += 0.1f;

	if (k == GLFW_KEY_8) vz -= 0.1f;

	if (k == GLFW_KEY_9) cx += 0.1f;

	if (k == GLFW_KEY_0) cx -= 0.1f;

	//THESE ARE SAME AS cz, MOVING UP AND DOWN
	//if (k == GLFW_KEY_COMMA) cy += 0.1f;

	//if (k == 46) cy -= 0.1f;// '.' DOT

	if (k == GLFW_KEY_LEFT_BRACKET) cz += 0.1f;

	if (k == GLFW_KEY_RIGHT_BRACKET) cz -= 0.1f;

	ImGuiIO& io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[k] = true;
	if (action == GLFW_RELEASE)
		io.KeysDown[k] = false;
	io.KeyCtrl = (mods & GLFW_MOD_CONTROL) != 0;
	io.KeyShift = (mods & GLFW_MOD_SHIFT) != 0;
}

//Mouse callback
static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS && button >= 0 && button < 2)
		mousePressed[button] = true;
}

//Scroll callback
static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel += (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
}

//Character callback
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
	//Notched Ring
	notched_ring.load_obj("objects/notched_ring.obj");
	notched_ring.smoothNormals(); //Might not need this
	notched_ring.createObject();

	//Contact
	contact.load_obj("objects/contact.obj");
	contact.smoothNormals(); //Might not need this
	contact.createObject();

	//Alphabet Tyre
	alphabet_tyre.load_obj("objects/alphabet_tyre.obj");
	alphabet_tyre.smoothNormals(); //Might not need this
	alphabet_tyre.createObject();

	//Plate contacts
	plate_contacts.load_obj("objects/plate_contact.obj");
	plate_contacts.smoothNormals(); //Might not need this
	plate_contacts.createObject();

	//Pin contacts
	pin_contact.load_obj("objects/pin_contact.obj");
	pin_contact.smoothNormals(); //Might not need this
	pin_contact.createObject();
	
	//Spring-loaded lever
	spring_loaded_lever.load_obj("objects/spring-loaded_lever.obj");
	spring_loaded_lever.smoothNormals(); //Might not need this
	spring_loaded_lever.createObject();

	//Hub
	hub.load_obj("objects/hub.obj");
	hub.smoothNormals(); //Might not need this
	hub.createObject();

	//Finger Wheel
	finger_wheel.load_obj("objects/finger_wheel.obj");
	finger_wheel.smoothNormals(); //Might not need this
	finger_wheel.createObject();

	//Ratchet Wheel
	ratchet_wheel.load_obj("objects/ratchet_wheel.obj");
	ratchet_wheel.smoothNormals(); //Might not need this
	ratchet_wheel.createObject();

	//Back contact
	back_contact.load_obj("objects/back_contact.obj");
	back_contact.smoothNormals(); //Might not need this
	back_contact.createObject();

	//Reflector
	reflector.load_obj("objects/reflector.obj");
	reflector.smoothNormals(); //Might not need this
	reflector.createObject();

}

//Draw all the components
void drawObjects()
{
	////Variables for components
	model = glm::mat4(1.0f);
	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis

	//ROTATE ON KEYPRESS
	glw->rotation = (360.0f / 26.0f) * (float)glw->count;

	model = glm::rotate(model, -glw->introtation, glm::vec3(0, 1, 0)); //rotate y axis

	if (glw->introtation < glw->rotation)
	{
		glw->introtation += rotationinc;
	}
	else if (glw->introtation > glw->rotation)
	{
		glw->introtation -= rotationinc;
	}

	model = glm::translate(model, glm::vec3(x, y + 5.5f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	////////////////NOTCHED RINGS
	glm::vec4 lightpos = view *  glm::vec4(0.5f, 0.5f, 7.0f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	comp = 0;
	glUniform1ui(componentID, comp);

	notched_ring.drawObject();

	//
	model = glm::translate(model, glm::vec3(x, y - 0.35f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 1;
	glUniform1ui(componentID, comp);

	
	notched_ring.drawObject();

	////////////////ALPHABET TYRE
	lightpos = view *  glm::vec4(0.75f, 1.75f, 5.5f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	//
	model = glm::translate(model, glm::vec3(x, y - 0.5f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 2;
	glUniform1ui(componentID, comp);
	
	contact.drawObject();

	//
	model = glm::translate(model, glm::vec3(x, y - 0.7f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 3;
	glUniform1ui(componentID, comp);

	alphabet_tyre.drawObject();


	////////////////// DRAW THE PINS
	lightpos = view *  glm::vec4(1.75f, 2.5f, 3.5f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	//
	model = glm::translate(model, glm::vec3(x - 0.2f, y - 1.f, z - 1.45f));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 4;
	glUniform1ui(componentID, comp);

	drawPlates();

	////

	model = glm::translate(model, glm::vec3(x, y - 1.8f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 5;
	glUniform1ui(componentID, comp);

	drawPins();

	//

	lightpos = view *  glm::vec4(0.0f, 1.5f, 0.0f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));
	
	model = glm::translate(model, glm::vec3(x, y + 0.45f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	/* WIRES HERE */

	glBindVertexArray(vao);

	for (int i = 0; i < 26; i++)
	{
		newnumplatez = 0.0f;
		newnumpinx = 0.0f;
		newnumpinz = 0.0f;
		newnumplatex = 0.0f;
		
		count = 0;
		count = mapAlphabet(i, 0);

		
		for (int k = 0; k < i + 1; k++)
		{
			newnumplatez += platez[k];
			newnumplatex += platex[k];
		}

		for (int j = 0; j < count + 1; j++)
		{
			newnumpinx += pinx[j];
			newnumpinz += pinz[j];
		}
		
		set_linevertex(newnumpinx, 0.0f, newnumpinz, newnumplatex, 1.35f, newnumplatez);
		
		comp = 11;
		glUniform1f(color4ID, 1.0f);
		

		if (glw->changed[i] == true)
		{
			glUniform1f(color1ID, 0.0f);
			glUniform1f(color2ID, 1.0f);
			glUniform1f(color3ID, 0.0f);
		}
		else
		{
			glUniform1f(color1ID, 0.2f);
			glUniform1f(color2ID, 0.2f);
			glUniform1f(color3ID, 0.9f);
		}

		if (glw->platechange[i] == true)
		{
			comp = 13;
			glUniform1ui(componentID, comp);
		}

		glUniform1ui(componentID, comp);

		model = glm::translate(model, glm::vec3(x, y, z));
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);


		drawBuffers();

		if (glw->changed[i] == true)
		{
			comp = 12;
			glUniform1ui(componentID, comp);
			illuminate();
		}
		if (glw->platechange[i] == true)
		{
			comp = 13;
			glUniform1ui(componentID, comp);
			illuminate();

		}
		if (glw->changed[i] == true && glw->platechange[i] == true)
		{
			line_vertex[0] += 0.05f;
			line_vertex[3] += 0.05f;
			comp = 12;
			glUniform1ui(componentID, comp);
			drawBuffers();
		}
	}

	//
	model = glm::mat4(1.0f);
	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis

	//ROTATE ON KEYPRESS
	glw->rotation = (360.0f / 26.0f) * (float)glw->count;

	model = glm::rotate(model, -glw->introtation, glm::vec3(0, 1, 0)); //rotate y axis

	if (glw->introtation < glw->rotation)
	{
		glw->introtation += rotationinc;
	}
	else if (glw->introtation > glw->rotation)
	{
		glw->introtation -= rotationinc;
	}


	//////////////SPRING-LOADED RING ADJUSTING LEVER

	lightpos = view *  glm::vec4(0.3f, 2.5f, -0.5f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));


	model = glm::translate(model, glm::vec3(x, y - 1.3f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 6;
	glUniform1ui(componentID, comp);
	
	
	spring_loaded_lever.drawObject();


	///////////HUB

	lightpos = view *  glm::vec4(0.5f, 0.25f, 1.0f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	model = glm::translate(model, glm::vec3(x, y - 0.5f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 7;
	glUniform1ui(componentID, comp);

	hub.drawObject();


	///////////FINGER WHEEL

	lightpos = view *  glm::vec4(0.75f, 2.05f, -1.5f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	model = glm::translate(model, glm::vec3(x, y - 0.7f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 8;
	glUniform1ui(componentID, comp);

	finger_wheel.drawObject();


	////////////////// RATCHET WHEEL

	lightpos = view *  glm::vec4(0.1f, 0.7f, -3.3f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	model = glm::mat4(1.0f);
	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = glm::rotate(model, 3.5f, glm::vec3(0, 1, 0));
	model = glm::rotate(model, -glw->introtation, glm::vec3(0, 1, 0)); //rotate y axis

	if (glw->introtation < glw->rotation)
	{
		glw->introtation += rotationinc;
	}
	else if (glw->introtation > glw->rotation)
	{
		glw->introtation -= rotationinc;
	}


	model = glm::translate(model, glm::vec3(x, y - 3.5f, z + 0.05f));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 9;
	glUniform1ui(componentID, comp);

	ratchet_wheel.drawObject();



	/////BACK CONTACT

	float rot = float(360.0f / 26.0f) / 2.0f;

	lightpos = view *  glm::vec4(0.1f, 0.9f, -4.8f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	model = glm::translate(model, glm::vec3(x, y - 0.7f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 2;
	glUniform1ui(componentID, comp);

	back_contact.drawObject();

	
	//////////REFLECTOR

	lightpos = view *  glm::vec4(0.7f, 2.95f, -5.5f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	model = glm::mat4(1.0f);

	glw->rotation = (360.0f / 26.0f) * (float)glw->count;

	
	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis

	//PUT BACK IN IF YOU WANT REFLECTOR TO ROTATE
	model = glm::rotate(model, -glw->introtation, glm::vec3(0, 1, 0)); //rotate y axis

	if (glw->introtation < glw->rotation)
	{
		glw->introtation += rotationinc;
	}
	else if (glw->introtation > glw->rotation)
	{
		glw->introtation -= rotationinc;
	}

	model = glm::rotate(model, -rot, glm::vec3(0, 1, 0));
	
	model = glm::translate(model, glm::vec3(x, y - 7.3f, z + 0.1f));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	
	
	comp = 10;
	glUniform1ui(componentID, comp);

	reflector.drawObject();
	
	//
	lightpos = view *  glm::vec4(0.0f, 0.05f, -6.5f, 1.0);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	model = glm::translate(model, glm::vec3(x - 0.02f, y + 1.8f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	comp = 2;
	glUniform1ui(componentID, comp);

	contact.drawObject();

	model = glm::mat4(1.0f);

	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis

	//PUT BACK IN IF YOU WANT REFLECTOR TO ROTATE

	glw->rotation = (360.0f / 26.0f) * (float)glw->count;

	model = glm::rotate(model, -glw->introtation, glm::vec3(0, 1, 0)); //rotate y axis

	if (glw->introtation < glw->rotation)
	{
		glw->introtation += rotationinc;
	}
	else if (glw->introtation > glw->rotation)
	{
		glw->introtation -= rotationinc;
	}


	model = glm::rotate(model, 180.0f, glm::vec3(0, 0, 1));
	model = glm::translate(model, glm::vec3(x - 0.18f, y + 5.4f, z - 1.455f));

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 4;
	glUniform1ui(componentID, comp);

	drawPins();

	//DRAW LINES FROM POINT TO REFLECTOR

	model = glm::mat4(1.0f);
	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));

	//ROTATE ON KEYPRESS
	glw->rotation = (360.0f / 26.0f) * (float)glw->count;

	model = glm::rotate(model, -glw->introtation, glm::vec3(0, 1, 0)); //rotate y axis

	if (glw->introtation < glw->rotation)
	{
		glw->introtation += rotationinc;
	}	
	else if (glw->introtation > glw->rotation)
	{
		glw->introtation -= rotationinc;
	}

	model = glm::translate(model, glm::vec3(x - 0.2f, y + 1.f, z - 1.45f));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

		
		for (int a = 0; a < 26; a++)
		{
			newcount = a;
			if (glw->changed[newcount] == true)
			{
				newnumpinx = 0.0f;
				newnumpinz = 0.0f;
				newnumplatex = 0.0f;
				newnumplatez = 0.0f;
				newnumrefz = 0.0f;
				newnumrefx = 0.0f;
				convplatex = 0.0f;
				convplatez = 0.0f;
				convpinx = 0.0f;
				convpinz = 0.0f;
				convrefx = 0.0f;
				convrefz = 0.0f;

				count = mapAlphabet(newcount, 1);
				countpin = mapAlphabet(newcount, 0);

				refcount = mapAlphabet(newcount, 2);//E
				pincountv = mapAlphabet(newcount, 3);//Q
				platecountv = mapAlphabet(newcount, 4);//X

				for (int j = 0; j < platecountv + 1 ; j++)
				{
					convplatex += platex[j];
					convplatez += platez[j];
				}

				for (int k = 0; k < refcount + 1; k++)
				{
					convrefx += pinx[k];
					convrefz += pinz[k];
				}

				for (int l = 0; l < pincountv + 1; l++)
				{
					convpinx += pinx[l];
					convpinz += pinz[l];
				}

				for (int m = 0; m < newcount + 1; m++)
				{
					newnumplatex += platex[m];
					newnumplatez += platez[m];
				}

				for (int n = 0; n < countpin + 1; n++)
				{
					newnumpinx += pinx[n];
					newnumpinz += pinz[n];
				}

				for (int o = 0; o < count + 1; o++)//?
				{
					newnumrefx += pinx[o];
					newnumrefz += pinz[o];
				}
				//////////////////////////////////////////////////////////////////////
				//LINES FOR THE VERY START

				comp = 12;
				glUniform1ui(componentID, comp);

				lightpos = view *  glm::vec4(0.2f, 0.5f, 7.0f, 1.0);
				glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

				set_linevertex(newnumplatex, 5.3f, newnumplatez, newnumplatex, 2.1f, newnumplatez);

				drawBuffers();
				illuminate();

				//////////////////////////////////////////////////////////////////////
				//LINES FROM PIN TO END OF RATCHET

				lightpos = view *  glm::vec4(0.2f, 0.5f, -2.0f, 1.0);
				glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

				set_linevertex(newnumpinx, 0.0f, newnumpinz, newnumpinx, -5.3f, newnumpinz);

				drawBuffers();
				illuminate();

				//////////////////////////////////////////////////////////////////////
				//LINES FROM RATCHET TO REFLECTOR

				set_linevertex(newnumpinx, -5.3f, newnumpinz, newnumrefx, -6.22f, newnumrefz);

				drawBuffers();
				illuminate();

				//////////////////////////////////////////////////////////////////////
				//NEWNUMREF LINE GOING LONGER
				
				set_linevertex(newnumrefx, -6.22f, newnumrefz, newnumrefx, -7.0f, newnumrefz);

				drawBuffers();
				illuminate();

				//////////////////////////////////////////////////////////////////////
				//LINE IN REFLECTOR
				glUniform1f(color1ID, 1.0f);
				glUniform1f(color2ID, 1.0f);
				glUniform1f(color3ID, 0.0f);

				set_linevertex(newnumrefx, -7.0f, newnumrefz, convrefx, -7.0f, convrefz);

				drawBuffers();
				illuminate();

				//////////////////////////////////////////////////////////////////////
				//CONVREF LINE LONGER
				comp = 13;
				glUniform1ui(componentID, comp);

				set_linevertex(convrefx, -6.22f, convrefz, convrefx, -7.0f, convrefz);
				
				drawBuffers();
				illuminate();

			
				//////////////////////////////////////////////////////////////////////
				//LINES TO COME BACK FROM REFLECTOR

				set_linevertex(convpinx, -5.3f, convpinz, convrefx, -6.22f, convrefz);

				drawBuffers();
				illuminate();

				//////////////////////////////////////////////////////////////////////
				//LINES THROUGH RATCHET BACK TO PINS

				set_linevertex(convpinx, 0.0f, convpinz, convpinx, -5.3f, convpinz);

				drawBuffers();
				illuminate();

				lightpos = view *  glm::vec4(0.2f, 0.5f, 7.0f, 1.0);
				glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

				//////////////////////////////////////////////////////////////////////
				//LINES BACK TO KEY

				set_linevertex(convplatex, 5.3f, convplatez, convplatex, 2.1f, convplatez);

				drawBuffers();
				illuminate();


				count = 0.0f;
				countpin = 0.0f;

				refcount = 0.0f;
				pincountv = 0.0f;
				platecountv = 0.0f;

			}
		}
}

//Draw array of plate contacts in a circle
void drawPlates()
{
	for (int i = 0; i < 26; i++)
	{
		model = glm::translate(model, glm::vec3(platex[i], y, platez[i]));
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

		plate_contacts.drawObject();
	}
}

//Draw array of pins contacts in a circle
void drawPins()
{
	
	for (int i = 0; i < 26; i++)
	{
		model = glm::translate(model, glm::vec3(pinx[i], y, pinz[i]));
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

		pin_contact.drawObject();
	}
}

//Initialize pin arrays
void calculateXZ()
{
	for (int i = 0; i < 26; i++)
	{
		//1.51 is the radius of the circle at which the whole are cut out at
		float angle = slice * i;
		platex[i] = (1.51 / 4) * cos(angle);
		platez[i] = (1.51 / 4) * sin(angle);
		pinx[i] = (1.51 / 4) * cos(angle);
		pinz[i] = (1.51 / 4) * sin(angle);

	}
}

//Set line vertex positions
void set_linevertex(GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2)
{
	line_vertex[0] = x1; 
	line_vertex[1] = y1;//Start length of line
	line_vertex[2] = z1;

	line_vertex[3] = x2;
	line_vertex[4] = y2;//End length of line
	line_vertex[5] = z2;
}

//Create buffesr
void createBuffers()
{
	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	float vertexColours[] = { 0.1f, 0.5f, 1.0f, 1.0f };
	calculateXZ();
	line_vertex = new GLfloat[numpoints * 3];

	for (int i = 0; i < numpoints; i++)
	{
		line_vertex[i * 3] = 0.0f;
		line_vertex[i * 3 + 1] = 0.0f;
		line_vertex[i * 3 + 2] = 0.0f;

		/*std::cout << "x: " << line_vertex[i * 3];
		std::cout << ", y: " << line_vertex[i * 3 + 1];
		std::cout << ", z: " << line_vertex[i * 3 + 2];
		std::cout << " t: " << t << std::endl;*/
	}
	

	/* Create a vertex buffer object to store vertices */
	glGenBuffers(1, &positionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* numpoints * 3, &(line_vertex[0]), GL_DYNAMIC_DRAW);

	//glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), line_vertex, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	/* Create a vertex buffer object to store vertex colours */
	glGenBuffers(1, &colourObject);
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColours), vertexColours, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	done = true;
}

//Get number on which pin to map to
GLuint mapAlphabet(int number, int type)
{
	char letter = ' ';
	char reflect = ' ';
	GLint character = 0;
	std::size_t newchar;
	
	switch (type)
	{
	case 0: //MAPS WIRES PIN TO PIN
			
			letter = glw->getStaticrOne().at(number);
			newchar = glw->getAlphabet().find(letter, 0);
			character = newchar;
			break;

	case 1: //MAPS PIN TO REFLECTOR
		if (glw->mode == "En")
		{
			
			newchar = glw->getReflector().find(glw->machine.char_rOne, 0);
			//std::cout << letter << " | " << newchar << std::endl;

			character = newchar;
			break;
		}
		else if (glw->mode == "De")
		{
			newchar = glw->getReflector().find(glw->machine.char_rOne, 0);
			//std::cout << letter << " | " << newchar << std::endl;

			character = newchar;
			break;
		}

	case 2: //POINT ON REFLECTOR
		if (glw->mode == "En")
		{
			newchar = glw->getReflector().find(glw->machine.char_reflect, 0);
			//std::cout << reflect << " | " << newchar << std::endl;

			character = newchar;
			break;
		}
		else if (glw->mode == "De")
		{
			newchar = glw->getReflector().find(glw->machine.char_inrOne, 0);
			//std::cout << letter << " | " << newchar << std::endl;

			character = newchar;
			break;
		}

	case 3: //REFLECTOR MAPPED BACK TO PINS
		if (glw->mode == "En")
		{
			
			//newchar = glw->getAlphabet().find(glw->char_letter, 0);
			newchar = glw->getAlphabet().find(glw->machine.char_letter, 0);
			letter = glw->getStaticrOne().at(newchar);
			newchar = glw->getAlphabet().find(letter, 0);

			//letter = glw->char_reflect;
			//newchar = glw->st_newchar;
			//std::cout << letter << " | " << newchar << std::endl;

			character = newchar;
			break;
		}
		else if (glw->mode == "De")
		{

			//newchar = glw->getRotorOne().find(glw->char_letter, 0);
			newchar = glw->getAlphabet().find(glw->machine.char_letter, 0);
			letter = glw->getStaticrOne().at(newchar);
			newchar = glw->getAlphabet().find(letter, 0);

			//std::cout << letter << " | " << newchar << std::endl;

			character = newchar;
			break;
		}

	case 4: //FINAL CIPHERED KEY
		if (glw->mode == "En")
		{
			newchar = glw->getAlphabet().find(glw->machine.char_letter, 0);
			//std::cout << letter << " | " << newchar << std::endl;

			glw->platechange[newchar] = true;

			character = newchar;
			break;
		}
		else if (glw->mode == "De")
		{
			newchar = glw->getAlphabet().find(glw->machine.char_letter, 0);
			glw->platechange[newchar] = true;

			//std::cout << letter << " | " << newchar << std::endl;

			character = newchar;
			break;
		}


	default:
		break;
	}

	return character;
}

//Make line thicker
void illuminate()
{
	for (int i = 0; i < 35; i++)
	{
		line_vertex[0] += 0.001f;
		drawBuffers();
	}
	for (int i = 0; i < 35; i++)
	{
		line_vertex[2] += 0.001f;
		drawBuffers();
	}
	for (int i = 0; i < 35; i++)
	{
		line_vertex[3] += 0.001f;
		drawBuffers();
	}
	for (int i = 0; i < 35; i++)
	{
		line_vertex[5] += 0.001f;
		drawBuffers();
	}

}

//Initialize
void init(GLWrapper *glw)
{
	angle_x = 15.0f;
	angle_x_inc = 0;

	angle_z = 0;
	angle_z_inc = 0;

	angle_y = 60.0f;
	angle_y_inc = 0;

	x = 0;
	
	y = 0;
	
	z = 0;
	
	scale = 0.5;
	scale_inc = 0;
	aspect_ratio = 1.3333f;

	vx = 0.0f;
	vy = 4.0f;
	vz = 8.0f;

	cx, cy, cz = 0.0f;
	lightx = -1.5f;

	lighty = 3.75f;

	lightz = 3.0f;
	
	emitmode = 0;
	color4 = 1.0f;
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
	proj_mtx_location = glGetUniformLocation(program[0], "ortho");
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

	/* Define vertices for a cube in 12 triangles */
	GLfloat vertexPositions[] =
	{
		-0.25f, 0.25f, -0.25f,
		-0.25f, -0.25f, -0.25f,
		0.25f, -0.25f, -0.25f,

		0.25f, -0.25f, -0.25f,
		0.25f, 0.25f, -0.25f,
		-0.25f, 0.25f, -0.25f,

		0.25f, -0.25f, -0.25f,
		0.25f, -0.25f, 0.25f,
		0.25f, 0.25f, -0.25f,

		0.25f, -0.25f, 0.25f,
		0.25f, 0.25f, 0.25f,
		0.25f, 0.25f, -0.25f,

		0.25f, -0.25f, 0.25f,
		-0.25f, -0.25f, 0.25f,
		0.25f, 0.25f, 0.25f,

		-0.25f, -0.25f, 0.25f,
		-0.25f, 0.25f, 0.25f,
		0.25f, 0.25f, 0.25f,

		-0.25f, -0.25f, 0.25f,
		-0.25f, -0.25f, -0.25f,
		-0.25f, 0.25f, 0.25f,

		-0.25f, -0.25f, -0.25f,
		-0.25f, 0.25f, -0.25f,
		-0.25f, 0.25f, 0.25f,

		-0.25f, -0.25f, 0.25f,
		0.25f, -0.25f, 0.25f,
		0.25f, -0.25f, -0.25f,

		0.25f, -0.25f, -0.25f,
		-0.25f, -0.25f, -0.25f,
		-0.25f, -0.25f, 0.25f,

		-0.25f, 0.25f, -0.25f,
		0.25f, 0.25f, -0.25f,
		0.25f, 0.25f, 0.25f,

		0.25f, 0.25f, 0.25f,
		-0.25f, 0.25f, 0.25f,
		-0.25f, 0.25f, -0.25f,
	};

	/* Manually specified colours for our cube */
	float vertexColours[] = {
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,

		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,

		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
	};

	/* Manually specified normals for our cube */
	GLfloat normals[] =
	{
		0, 0, -1.f, 0, 0, -1.f, 0, 0, -1.f,
		0, 0, -1.f, 0, 0, -1.f, 0, 0, -1.f,
		1.f, 0, 0, 1.f, 0, 0, 1.f, 0, 0,
		1.f, 0, 0, 1.f, 0, 0, 1.f, 0, 0,
		0, 0, 1.f, 0, 0, 1.f, 0, 0, 1.f,
		0, 0, 1.f, 0, 0, 1.f, 0, 0, 1.f,
		-1.f, 0, 0, -1.f, 0, 0, -1.f, 0, 0,
		-1.f, 0, 0, -1.f, 0, 0, -1.f, 0, 0,
		0, -1.f, 0, 0, -1.f, 0, 0, -1.f, 0,
		0, -1.f, 0, 0, -1.f, 0, 0, -1.f, 0,
		0, 1.f, 0, 0, 1.f, 0, 0, 1.f, 0,
		0, 1.f, 0, 0, 1.f, 0, 0, 1.f, 0,
	};

	GLfloat texcoords[] =
	{
		0, 1.f, 0, 0, 1.f, 0,
		1.f, 0, 1.f, 1.f, 0, 1.f
	};//ONLY NEEDED THESE BECAUE CUBE BEING SCALED TO SQUARE

	/* Create the vertex buffer for the cube */
	glGenBuffers(1, &textpositionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, textpositionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create the colours buffer for the cube */
	glGenBuffers(1, &textcolourObject);
	glBindBuffer(GL_ARRAY_BUFFER, textcolourObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColours), vertexColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create the normals  buffer for the cube */
	glGenBuffers(1, &textnormalsBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, textnormalsBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), normals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create the texture coordinate  buffer for the cube */
	glGenBuffers(1, &texCoordsObject);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program[1], "model");
	viewID = glGetUniformLocation(program[1], "view");
	projectionID = glGetUniformLocation(program[1], "projection");
	lightposID = glGetUniformLocation(program[1], "lightpos");

	normalmatrixID = glGetUniformLocation(program[1], "normalmatrix");
	componentID = glGetUniformLocation(program[1], "comp");
	color1ID = glGetUniformLocation(program[1], "color1");
	color2ID = glGetUniformLocation(program[1], "color2");
	color3ID = glGetUniformLocation(program[1], "color3");
	color4ID = glGetUniformLocation(program[1], "color4");
	emitmodeID = glGetUniformLocation(program[1], "emitmode");
	textmodeID = glGetUniformLocation(program[1], "textmode");

	/* Load a texture */
	try
	{
		/* Not actually needed if using one texture at a time */
		glActiveTexture(GL_TEXTURE1);

		/* load an image file directly as a new OpenGL texture */
		texID = SOIL_load_OGL_texture("rotordetails.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		/* check for an error during the load process */
		if (texID == 0)
		{
			printf("Texture ID1 SOIL loading error: '%s'\n", SOIL_last_result());
		}


		int loc = glGetUniformLocation(program[1], "tex1");
		if (loc >= 0) glUniform1i(loc, 0);
	}
	catch (std::exception &e)
	{
		printf("\nImage file loading failed.");
	}

	//These make the texture look sharper!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

//Loads texture for fonts
void LoadFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();

	//ImFont* my_font1 = io.Fonts->AddFontDefault();
	//ImFont* my_font2 = io.Fonts->AddFontFromFileTTF("extra_fonts/Karla-Regular.ttf", 15.0f);
	//ImFont* my_font3 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyClean.ttf", 13.0f); my_font3->DisplayOffset.y += 1;
	//ImFont* my_font4 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyTiny.ttf", 10.0f); my_font4->DisplayOffset.y += 1;
	//ImFont* my_font5 = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 20.0f, io.Fonts->GetGlyphRangesJapanese());

	//io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 15.0f);
	
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

	GLuint tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *)(intptr_t)tex_id;
}

//Initialize ImGUI
void InitImGui()
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = 1.0f / 60.0f;                                  // Time elapsed since last frame, in seconds (in this sample app we'll override this every frame because our timestep is variable)
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
	//io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE; //THIS DELETED THE TEXT IN THE INPUT BOX, SO IT IS COMMENTED OUT
	io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
	io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
	io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
	io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
	io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
	io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

	io.RenderDrawListsFn = ImImpl_RenderDrawLists;
	io.SetClipboardTextFn = ImImpl_SetClipboardTextFn;
	io.GetClipboardTextFn = ImImpl_GetClipboardTextFn;

	LoadFontsTexture();
}

//Draw the lines
void drawBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* numpoints * 3, &(line_vertex[0]), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);

	/* glVertexAttribPointer(index, size, type, normalised, stride, pointer)
	index relates to the layout qualifier in the vertex shader and in
	glEnableVertexAttribArray() and glDisableVertexAttribArray() */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glEnableVertexAttribArray(1);

	/* glVertexAttribPointer(index, size, type, normalised, stride, pointer)
	index relates to the layout qualifier in the vertex shader and in
	glEnableVertexAttribArray() and glDisableVertexAttribArray() */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_LINES, 0, numpoints);

}

//Main Application Code
//int main(int argc, char ** argv) //SHOWS CONSOLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int cmdShow) //NO CONSOLE
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
	glDeleteProgram(program[0]);
	glDeleteProgram(program[1]);

	delete(glw);
	return 0;
}

