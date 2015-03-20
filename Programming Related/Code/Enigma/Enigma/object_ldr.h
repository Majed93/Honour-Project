#pragma once

/* object_ldr.h
   Example class to show the start of an .obj mesh obkect file
   loader
   Iain Martin November 2014
   Modified by Majed Monem
*/

#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class object_ldr
{
public:
	object_ldr();
	~object_ldr();

	void load_obj(const char* filename);
	void drawObject();
	void createObject();
	void smoothNormals();

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	GLfloat* colours;
	GLfloat* textures;
	std::vector<GLushort> elements;

	GLuint vbo_mesh_vertices;
	GLuint vbo_mesh_normals;
	GLuint vbo_mesh_colours;
	GLuint vbo_mesh_tex;

	GLuint ibo_mesh_elements;
	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;
	GLuint attribute_v_tex;

	GLuint vao;
};

