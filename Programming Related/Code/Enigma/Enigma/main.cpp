/*
Majed Monem
*/

#ifdef _MSC_VER
#pragma warning (disable: 4996)         // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"   // warning: unused function
#endif

#include "imgui.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
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
#include "glm/gtc/random.hpp"

#include "object_ldr.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


//GLuint positionBufferObject, colourObject;
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

GLfloat* line_vertex;
GLboolean done = false;
/* Uniforms*/
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID, componentID, color1ID, color2ID, color3ID, emitmodeID;
glm::mat4 model;
GLuint comp, emitmode;
GLfloat color1[26], color2[26], color3[26];

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

static GLWrapper *glw = new GLWrapper(800, 500, "Graphical Enigma Simulator - Main Menu");

static GLFWwindow* window;
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

void display();
void drawObjects();
void drawPlates();
void drawPins();
void createObjects();
void drawBuffers();
void createBuffers();
void calculateXZ();
GLfloat bezier(float A, float B, float C, float D, float t);
GLuint mapAlphabet(int number,int type);

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
		if (done == false)
		{
			createBuffers();
		}
		display();
	}
	else if (glw->mode == "De")
	{
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
	model = glm::mat4(1.0f);

	//model = glm::rotate(model, -angle_x, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	//model = glm::rotate(model, -angle_z, glm::vec3(0, 0, 1)); //rotate z axis
	//model = glm::rotate(model, -angle_y, glm::vec3(0, 1, 0)); //rotate y axis
	model = glm::translate(model, glm::vec3(x, y, z));
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	
	// Send our transformations to the currently bound shader,

	glm::mat4 projection = glm::perspective(80.0f, aspect_ratio, 0.1f, 100.0f);

	glm::mat4 view = glm::lookAt(
		glm::vec3(vx, vy, vz),
		glm::vec3(cx, cy, cz),
		glm::vec3(0, 1, 0)
		);

	//// Apply rotations to the view position
	view = glm::rotate(view, -angle_x, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = glm::rotate(view, -angle_y, glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = glm::rotate(view, -angle_z, glm::vec3(0, 0, 1));


	glm::mat3 normalmatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));
	glm::vec4 lightpos = view *  glm::vec4(lightx, lighty, lightz, 1.0);

	

	// Send our transformations to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos));

	
	drawObjects();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);

	/* Modify our animation variables */
	
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

	//64 and 90 are all alphabet values
	if (k > 64 && k < 91 && action == GLFW_PRESS)
	{
		//std::cout << "letter code :" << k << std::endl;
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

	}

	//Backspace
	else if (k == 259 && action == GLFW_PRESS)
	{
		if (glw->encrypted != "" || glw->decrypted != "")
		{
			if (glw->mode == "En")
			{
				glw->encrypted.erase(glw->encrypted.end() - 1);
			}
			else if (glw->mode == "De")
			{
				glw->decrypted.erase(glw->decrypted.end() - 1);
			}
		}
	}
	
	if (k == GLFW_KEY_LEFT) angle_y += 0.5f;

	if (k == GLFW_KEY_RIGHT) angle_y -= 0.5f;

	if (k == GLFW_KEY_UP) angle_x += 0.5f;

	if (k == GLFW_KEY_DOWN) angle_x -= 0.5f;

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

	if (k == GLFW_KEY_KP_ADD) cy += 0.1f;

	if (k == GLFW_KEY_KP_SUBTRACT) cy -= 0.1f;

	if (k == GLFW_KEY_LEFT_BRACKET) cz += 0.1f;

	if (k == GLFW_KEY_RIGHT_BRACKET) cz -= 0.1f;

	/*if (k == 'Q') angle_x_inc += 0.1f;
	if (k == 'W') angle_x_inc -= 0.1f;
	if (k == 'A') angle_z_inc += 0.1f;
	if (k == 'S') angle_z_inc -= 0.1f;
	if (k == 'Z') angle_y_inc += 0.1f;
	if (k == 'X') angle_y_inc -= 0.1f;

	

	if (k == GLFW_KEY_M) z_inc += 0.0001f;

	if (k == GLFW_KEY_N) z_inc -= 0.0001f;

	if (k == 'I') scale_inc += 0.0001f;
	if (k == 'K') scale_inc -= 0.0001f;
*/
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
	//Notched Ring
	notched_ring.load_obj("objects/notched_ring.obj");
	//notched_ring.smoothNormals(); //Might not need this
	notched_ring.createObject();

	//Contact
	contact.load_obj("objects/contact.obj");
	//contact.smoothNormals(); //Might not need this
	contact.createObject();

	//Alphabet Tyre
	alphabet_tyre.load_obj("objects/alphabet_tyre.obj");
	//alphabet_tyre.smoothNormals(); //Might not need this
	alphabet_tyre.createObject();

	//Plate contacts
	plate_contacts.load_obj("objects/plate_contact.obj");
	//plate_contacts.smoothNormals(); //Might not need this
	plate_contacts.createObject();

	//Pin contacts
	pin_contact.load_obj("objects/pin_contact.obj");
	//pin_contacts.smoothNormals(); //Might not need this
	pin_contact.createObject();
	
	//Spring-loaded lever
	spring_loaded_lever.load_obj("objects/spring-loaded_lever.obj");
	//spring_loaded_lever.smoothNormals(); //Might not need this
	spring_loaded_lever.createObject();

	//Hub
	hub.load_obj("objects/hub.obj");
	//hub.smoothNormals(); //Might not need this
	hub.createObject();

	//Finger Wheel
	finger_wheel.load_obj("objects/finger_wheel.obj");
	//finger_wheel.smoothNormals(); //Might not need this
	finger_wheel.createObject();

	//Ratchet Wheel
	ratchet_wheel.load_obj("objects/ratchet_wheel.obj");
	//ratchet_wheel.smoothNormals(); //Might not need this
	ratchet_wheel.createObject();

	//Back contact
	back_contact.load_obj("objects/back_contact.obj");
	//back_contact.smoothNormals(); //Might not need this
	back_contact.createObject();

	//Reflector
	reflector.load_obj("objects/reflector.obj");
	//reflector.smoothNormals(); //Might not need this
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

	model = glm::translate(model, glm::vec3(x, y + 5.5f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 0;
	glUniform1ui(componentID, comp);


	notched_ring.drawObject();

	//
	model = glm::translate(model, glm::vec3(x, y - 0.35f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 1;
	glUniform1ui(componentID, comp);

	notched_ring.drawObject();

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

	model = glm::translate(model, glm::vec3(x, y + 0.45f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	///* WIRES HERE */

	glBindVertexArray(vao);

	//model = glm::scale(model, glm::vec3(scale, scale, scale));
	//model = glm::rotate(model, 90.0f, glm::vec3(0, 0, 1)); //rotating in clockwise direction around x-axis
	////
	
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
		
		line_vertex[0] = newnumpinx;// bezier(0.01f, 1.0f, 1.5f, 1.0f, 0);
		line_vertex[1] = 0.0f;// bezier(-0.01f, 0.0f, 1.0f, 2.5f, 0); //DOES NOTHING
		line_vertex[2] = newnumpinz;// bezier(0.01f, 0.0f, 0.0f, 0.02f, 0);
		
		line_vertex[3] = newnumplatex;// bezier(newnumplatex, newnumplatex, newnumplatex, newnumplatex, 0.5); //Controls horizontal position of line nearest to plate contact
		line_vertex[4] = 1.35f;// bezier(5.f, 0.0f, 0.0f, 0.0f, 0.5); //Controls length of line
		line_vertex[5] = newnumplatez;// bezier(newnumplatez, newnumplatez, newnumplatez, newnumplatez, 0.5); //Controls height of line (up and down) nearest to plate contact
		
		//line_vertex[6] = bezier(0.01f, 1.0f, 1.5f, 1.0f, 0);  //DOES NOTHING
		//line_vertex[7] = bezier(-0.01f, 0.0f, 1.0f, 2.5f, 0); //DOES NOTHING
		//line_vertex[8] =  bezier(0.01f, 0.0f, 0.0f, 0.02f, 0); //DOES NOTHING
		comp = 12;

		

		if (glw->changed[i] == true)
		{
			glUniform1f(color1ID, 0.9f);
			glUniform1f(color2ID, 0.9f);
			glUniform1f(color3ID, 0.7f);
			
			emitmode = 1;
			glUniform1ui(emitmodeID, emitmode);

		}
		else
		{
			glUniform1f(color1ID, 0.9f);
			glUniform1f(color2ID, 0.1f);
			glUniform1f(color3ID, 0.1f);
		}

		if (glw->platechange[i] == true)
		{
			comp = 13;
			glUniform1ui(componentID, comp);
		}

		emitmode = 0;
		glUniform1ui(emitmodeID, emitmode);

		glUniform1ui(componentID, comp);

		model = glm::translate(model, glm::vec3(x, y, z));
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);


		drawBuffers();

		if (glw->changed[i] == true && glw->platechange[i] == true)
		{
			line_vertex[0] += 0.05f;
			line_vertex[3] += 0.05f;
			comp = 12;
			glUniform1ui(componentID, comp);
			drawBuffers();
		}
	}

	////

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

	model = glm::translate(model, glm::vec3(x, y - 1.3f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 6;
	glUniform1ui(componentID, comp);

	spring_loaded_lever.drawObject();

	//
	model = glm::translate(model, glm::vec3(x, y - 0.5f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 7;
	glUniform1ui(componentID, comp);

	hub.drawObject();

	//
	model = glm::translate(model, glm::vec3(x, y - 0.7f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 8;
	glUniform1ui(componentID, comp);

	finger_wheel.drawObject();

	//
	model = glm::mat4(1.0f);
	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = glm::rotate(model, 3.5f, glm::vec3(0, 1, 0));
	model = glm::rotate(model, -glw->introtation, glm::vec3(0, 1, 0)); //rotate y axis

	if (glw->introtation < glw->rotation)
	{
		glw->introtation += rotationinc;
	}
	model = glm::translate(model, glm::vec3(x, y - 3.5f, z + 0.05f));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 9;
	glUniform1ui(componentID, comp);

	ratchet_wheel.drawObject();

	//
	float rot = float(360.0f / 26.0f) / 2.0f;

	
	
	model = glm::translate(model, glm::vec3(x, y - 0.7f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 10;
	glUniform1ui(componentID, comp);

	back_contact.drawObject();

	//
	model = glm::mat4(1.0f);

	glw->rotation = (360.0f / 26.0f) * (float)glw->count;

	
	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis

	model = glm::rotate(model, -glw->introtation, glm::vec3(0, 1, 0)); //rotate y axis

	if (glw->introtation < glw->rotation)
	{
		glw->introtation += rotationinc;
	}
	model = glm::rotate(model, -rot, glm::vec3(0, 1, 0));
	
	model = glm::translate(model, glm::vec3(x, y - 7.3f, z + 0.1f));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	comp = 2;
	glUniform1ui(componentID, comp);

	reflector.drawObject();

	//
	model = glm::translate(model, glm::vec3(x - 0.02f, y + 1.8f, z));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	contact.drawObject();

	model = glm::mat4(1.0f);

	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis

	glw->rotation = (360.0f / 26.0f) * (float)glw->count;

	model = glm::rotate(model, -glw->introtation, glm::vec3(0, 1, 0)); //rotate y axis

	if (glw->introtation < glw->rotation)
	{
		glw->introtation += rotationinc;
	}

	model = glm::rotate(model, 180.0f, glm::vec3(0, 0, 1));
	//model = glm::rotate(model, rot, glm::vec3(0, 1, 0));
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

	//model = glm::rotate(model, rot, glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(x - 0.2f, y + 1.f, z - 1.45f));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	for (int i = 0; i < 26; i++)
	{	
		if (glw->changed[i] == true)
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

			count = mapAlphabet(i, 1);
			countpin = mapAlphabet(i, 0);

			refcount = mapAlphabet(i, 2);
			pincountv = mapAlphabet(i, 3);
			platecountv = mapAlphabet(i, 4);

			for (int j = 0; j < platecountv + 1; j++)
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

			for (int m = 0; m < i + 1; m++)
			{
				newnumplatex += platex[m];
				newnumplatez += platez[m];
			}

			for (int n = 0; n < countpin + 1; n++)
			{
				newnumpinx += pinx[n];
				newnumpinz += pinz[n];
			}

			for (int o = 0; o < count + 1; o++)
			{
				newnumrefx += pinx[o];
				newnumrefz += pinz[o];
			}
			//////////////////////////////////////////////////////////////////////
			//LINES FOR THE VERY START

			line_vertex[0] = newnumplatex;// bezier(0.01f, 1.0f, 1.5f, 1.0f, 0);
			line_vertex[1] = 5.3f;// bezier(0.0, 100.0f, 8.0f, newnumplatex * newnumplatez + 5.5f, 0);
			line_vertex[2] = newnumplatez;// bezier(0.01f, 0.0f, 0.0f, 0.02f, 0);

			line_vertex[3] = newnumplatex;// bezier(newnumplatex, newnumplatex, newnumplatex, newnumplatex, 0.5); //Controls horizontal position of line nearest to plate contact
			//MUST BE GREATER THAN line_vertex[1]
			line_vertex[4] = 2.1f;// bezier(5.f, 0.0f, 0.0f, 0.0f, 0.5); //Controls length of line
			line_vertex[5] = newnumplatez;// bezier(newnumplatez, newnumplatez, newnumplatez, newnumplatez, 0.5); //Controls height of line (up and down) nearest to plate contact

			emitmode = 1;
			glUniform1ui(emitmodeID, emitmode);

			drawBuffers();

			//THROUGH TO REFLECTOR
			line_vertex[0] = newnumpinx;// bezier(0.01f, 1.0f, 1.5f, 1.0f, 0);
			line_vertex[1] = 0.0f;// bezier(0.0, 100.0f, 8.0f, newnumplatex * newnumplatez + 5.5f, 0);
			line_vertex[2] = newnumpinz;// bezier(0.01f, 0.0f, 0.0f, 0.02f, 0);
			
			line_vertex[3] = newnumpinx;// bezier(newnumplatex, newnumplatex, newnumplatex, newnumplatex, 0.5); //Controls horizontal position of line nearest to plate contact
			line_vertex[4] = -5.3f;// bezier(5.f, 0.0f, 0.0f, 0.0f, 0.5); //Controls length of line
			line_vertex[5] = newnumpinz;// bezier(newnumplatez, newnumplatez, newnumplatez, newnumplatez, 0.5); //Controls height of line (up and down) nearest to plate contact

			//line_vertex[6] = newnumrefx;
			//line_vertex[7] = 1.3f;
			//line_vertex[8] = newnumrefz;
			
			emitmode = 1;
			glUniform1ui(emitmodeID, emitmode);

			drawBuffers();
			//////////////////////////////////////////////////////////////////////
			//LINES FROM RATCHET TO REFLECTOR

			line_vertex[0] = newnumpinx;// bezier(0.01f, 1.0f, 1.5f, 1.0f, 0);
			line_vertex[1] = -5.3f;// bezier(0.0, 100.0f, 8.0f, newnumplatex * newnumplatez + 5.5f, 0);
			line_vertex[2] = newnumpinz;// bezier(0.01f, 0.0f, 0.0f, 0.02f, 0);
			
			line_vertex[3] = newnumrefx;// bezier(newnumplatex, newnumplatex, newnumplatex, newnumplatex, 0.5); //Controls horizontal position of line nearest to plate contact
			//MUST BE GREATER THAN line_vertex[1]
			line_vertex[4] = -6.18f;// bezier(5.f, 0.0f, 0.0f, 0.0f, 0.5); //Controls length of line
			line_vertex[5] = newnumrefz;// bezier(newnumplatez, newnumplatez, newnumplatez, newnumplatez, 0.5); //Controls height of line (up and down) nearest to plate contact
			
			emitmode = 1;
			glUniform1ui(emitmodeID, emitmode);

			drawBuffers();

			//
			comp = 13;
			glUniform1ui(componentID, comp);

			//////////////////////////////////////////////////////////////////////
			//LINES TO COME BACK FROM REFLECTOR

			line_vertex[0] = convpinx;// bezier(0.01f, 1.0f, 1.5f, 1.0f, 0);
			line_vertex[1] = -5.3f;// bezier(0.0, 100.0f, 8.0f, newnumplatex * newnumplatez + 5.5f, 0);
			line_vertex[2] = convpinz;// bezier(0.01f, 0.0f, 0.0f, 0.02f, 0);

			line_vertex[3] = convrefx;// bezier(newnumplatex, newnumplatex, newnumplatex, newnumplatex, 0.5); //Controls horizontal position of line nearest to plate contact
			//MUST BE GREATER THAN line_vertex[1]
			line_vertex[4] = -6.18f;// bezier(5.f, 0.0f, 0.0f, 0.0f, 0.5); //Controls length of line
			line_vertex[5] = convrefz;// bezier(newnumplatez, newnumplatez, newnumplatez, newnumplatez, 0.5); //Controls height of line (up and down) nearest to plate contact

			
			drawBuffers();
			

			//////////////////////////////////////////////////////////////////////
			//LINES THROUGH RATCHET BACK TO PINS

			line_vertex[0] = convpinx;// bezier(0.01f, 1.0f, 1.5f, 1.0f, 0);
			line_vertex[1] = 0.0f;// bezier(0.0, 100.0f, 8.0f, newnumplatex * newnumplatez + 5.5f, 0);
			line_vertex[2] = convpinz;// bezier(0.01f, 0.0f, 0.0f, 0.02f, 0);

			line_vertex[3] = convpinx;// bezier(newnumplatex, newnumplatex, newnumplatex, newnumplatex, 0.5); //Controls horizontal position of line nearest to plate contact
			//MUST BE GREATER THAN line_vertex[1]
			line_vertex[4] = -5.3f;// bezier(5.f, 0.0f, 0.0f, 0.0f, 0.5); //Controls length of line
			line_vertex[5] = convpinz;// bezier(newnumplatez, newnumplatez, newnumplatez, newnumplatez, 0.5); //Controls height of line (up and down) nearest to plate contact


			drawBuffers();

			//CHANGE ALL THESE TO MAYBE COMP=LIGHTCOLOUR?
			emitmode = 0;
			glUniform1ui(emitmodeID, emitmode);


			//////////////////////////////////////////////////////////////////////
			//LINES BACK TO KEY

			line_vertex[0] = convplatex;// bezier(0.01f, 1.0f, 1.5f, 1.0f, 0);
			line_vertex[1] = 5.3f;// bezier(0.0, 100.0f, 8.0f, newnumplatex * newnumplatez + 5.5f, 0);
			line_vertex[2] = convplatez;// bezier(0.01f, 0.0f, 0.0f, 0.02f, 0);

			line_vertex[3] = convplatex;// bezier(newnumplatex, newnumplatex, newnumplatex, newnumplatex, 0.5); //Controls horizontal position of line nearest to plate contact
			//MUST BE GREATER THAN line_vertex[1]
			line_vertex[4] = 2.1f;// bezier(5.f, 0.0f, 0.0f, 0.0f, 0.5); //Controls length of line
			line_vertex[5] = convplatez;// bezier(newnumplatez, newnumplatez, newnumplatez, newnumplatez, 0.5); //Controls height of line (up and down) nearest to plate contact


			drawBuffers();

			//CHANGE ALL THESE TO MAYBE COMP=LIGHTCOLOUR?
			emitmode = 0;
			glUniform1ui(emitmodeID, emitmode);

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

void calculateXZ()
{
	for (int i = 0; i < 26; i++)
	{
		float angle = slice * i;
		platex[i] = (1.51 / 4) * cos(angle);
		platez[i] = (1.51 / 4) * sin(angle);
		pinx[i] = (1.51 / 4) * cos(angle);
		pinz[i] = (1.51 / 4) * sin(angle);

		/*std::cout << i << ": ";
		std::cout << "platex: " << platex[i];
		std::cout << ", platez: " << platez[i];
		std::cout << "||";
		std::cout << "pinx: " << pinx[i];
		std::cout << ", pinz: " << pinz[i] << std::endl;
		*/
		color1[i] = glm::linearRand(0.0f, (float)i / 26);
		color2[i] = glm::linearRand(0.0f, (float)i / 26);
		color3[i] = glm::linearRand(0.0f, (float)i / 26);
	}
}

GLfloat bezier(float origin, float control1, float control2, float destination, float t)
{
	float point = pow(1 - t, 3) * origin + 3.0 * pow(1 - t, 2) * t * control1 + 3.0 * (1 - t) * t * t * control2 + t * t * t * destination;
			
	return point;
}

void createBuffers()
{
	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	float vertexColours[] = { 0.1f, 0.5f, 1.0f, 1.0f };
	calculateXZ();
	line_vertex = new GLfloat[numpoints * 3];
	float t = 0.0f;
	for (int i = 0; i < numpoints; i++)
	{
		line_vertex[i * 3] = bezier(0.01f, 1.0f, 1.5f, 1.0f, t); 
		line_vertex[i * 3 + 1] = bezier(-0.01f, 0.0f, 1.0f, 2.5f, t);
		line_vertex[i * 3 + 2] =  bezier(0.01f, 0.0f, 0.0f, 0.02f, t);
		
		//line_vertex[0] = bezier(0.01f, 1.0f, 1.5f, 1.0f, 0);
		//line_vertex[1] = bezier(-0.01f, 0.0f, 1.0f, 2.5f, 0);
		//line_vertex[2] = bezier(0.01f, 0.0f, 0.0f, 0.02f, 0);
		//line_vertex[3] = bezier(0.01f, 1.0f, 1.5f, 1.0f, 0.5); 
		//line_vertex[4] = bezier(-0.01f, 0.0f, 1.0f, 2.5f, 0.5);
		//line_vertex[5] = bezier(10.01f, 0.0f, 0.0f, 0.02f, 0.5);
		//line_vertex[6] = bezier(0.01f, 1.0f, 1.5f, 1.0f, 1);
		//line_vertex[7] = bezier(-0.01f, 0.0f, 1.0f, 2.5f, 1);
		//line_vertex[8] = bezier(1.01f, 1.0f, 1.0f, 1.02f, 1);


		t += 1.0 / (float)numpoints;

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
		letter = glw->getStaticrOne().at(number);
		newchar = glw->getAlphabet().find(letter, 0);

		reflect = glw->getAlphabet().at(newchar);
		newchar = glw->getReflector().find(reflect, 0);
		
		character = newchar;
		break;

	case 2: //POINT ON REFLECTOR
		letter = glw->getStaticrOne().at(number);
		newchar = glw->getAlphabet().find(letter, 0);//4 AND E

		//GETS LETTER OF THAT INDEX
		reflect = glw->getAlphabet().at(newchar);//E
		newchar = glw->getReflector().find(reflect, 0); //16

		reflect = glw->getAlphabet().at(newchar);//Q
		newchar = glw->getReflector().find(reflect, 0); //4

		
		character = newchar;
		break;

	case 3: //REFLECTOR MAPPED BACK TO PINS
		letter = glw->getStaticrOne().at(number);
		newchar = glw->getAlphabet().find(letter, 0);

		reflect = glw->getAlphabet().at(newchar);
		newchar = glw->getReflector().find(reflect, 0);

		reflect = glw->getAlphabet().at(newchar);
		newchar = glw->getReflector().find(reflect, 0);

		newchar = glw->getAlphabet().find(reflect, 0); //16
		letter = glw->getStaticrOne().at(newchar); //X
		newchar = glw->getAlphabet().find(letter, 0);//23
		//std::cout << letter << " | " << newchar << std::endl;

		character = newchar;//MUST BE 24
		break;

	case 4: //FINAL CIPHERED KEY
		letter = glw->getStaticrOne().at(number);
		newchar = glw->getAlphabet().find(letter, 0);

		reflect = glw->getAlphabet().at(newchar);
		newchar = glw->getReflector().find(reflect, 0);

		reflect = glw->getAlphabet().at(newchar);
		newchar = glw->getReflector().find(reflect, 0);

		newchar = glw->getAlphabet().find(reflect, 0); //16
		letter = glw->getStaticrOne().at(newchar); //X
		newchar = glw->getAlphabet().find(letter, 0);//23

		letter = glw->getAlphabet().at(newchar);
		newchar = glw->getStaticrOne().find(letter, 0);
		glw->platechange[newchar] = true;

		//std::cout << letter << " | " << newchar << std::endl;

		character = newchar;
		break;

	default:
		break;
	}

	return character;

	/*switch (letter)
	{
	case 'A':
		character = 0;
		break;

	case 'B':
		character = 1;
		break;

	case 'C':
		character = 2;
		break;

	case 'D':
		character = 3;
		break;

	case 'E':
		character = 4;
		break;

	case 'F':
		character = 5;
		break;

	case 'G':
		character = 6;
		break;

	case 'H':
		character = 7;
		break;

	case 'I':
		character = 8;
		break;

	case 'J':
		character = 9;
		break;

	case 'K':
		character = 10;
		break;

	case 'L':
		character = 11;
		break;

	case 'M':
		character = 12;
		break;

	case 'N':
		character = 13;
		break;

	case 'O':
		character = 14;
		break;

	case 'P':
		character = 15;
		break;

	case 'Q':
		character = 16;
		break;

	case 'R':
		character = 17;
		break;

	case 'S':
		character = 18;
		break;

	case 'T':
		character = 19;
		break;

	case 'U':
		character = 20;
		break;

	case 'V':
		character = 21;
		break;

	case 'W':
		character = 22;
		break;

	case 'X':
		character = 23;
		break;

	case 'Y':
		character = 24;
		break;

	case 'Z':
		character = 25;
		break;

	default:
		break;

	}
	return character;*/
}

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
	lightx = 3.5f;

	lighty = 1.75f;

	lightz = 0.0f;
	
	emitmode = 0;
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
	emitmodeID = glGetUniformLocation(program[1], "emitmode");

}

void LoadFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();
	//ImFont* my_font1 = io.Fonts->AddFontDefault();
	//ImFont* my_font2 = io.Fonts->AddFontFromFileTTF("extra_fonts/Karla-Regular.ttf", 15.0f);
	//ImFont* my_font3 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyClean.ttf", 13.0f); my_font3->DisplayOffset.y += 1;
	//ImFont* my_font4 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyTiny.ttf", 10.0f); my_font4->DisplayOffset.y += 1;
	//ImFont* my_font5 = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 20.0f, io.Fonts->GetGlyphRangesJapanese());

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

	LoadFontsTexture();
}

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
int main(int argc, char ** argv)
{

	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);
	glw->setMouseButtonCallback(glfw_mouse_button_callback);
	glw->setScrollCallback(glfw_scroll_callback);
	glw->setCharCallback(glfw_char_callback);
	glw->setRotorOne("EKMFLGDQVZNTOWYHXUSPAIBRCJ");
	glw->setStaticrOne("EKMFLGDQVZNTOWYHXUSPAIBRCJ");
	glw->setRelfector("YRUHQSLDPXNGOKMIEBFZCWVJAT");
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
